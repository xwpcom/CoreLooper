/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "cJSON.h"
#include "ota_api.h"
#include "ota_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro)                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/
static uint8_t ota_copy_json_str_item(cJSON *item, char **d);
/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

static uint8_t ota_copy_json_str_item(cJSON *item, char **d)
{
    if(item != NULL)
    {
        size_t cplen = ota_strlen(item->valuestring) + 1;
        *d = (char *)ota_malloc(cplen);
        if(*d == NULL)
        {
            return OTA_ERROR;
        }
        ota_memset(*d, 0, cplen);
        ota_memcpy(*d, item->valuestring, cplen);
        return OTA_OK;
    }
    return OTA_ERROR;
}

uint8_t otaParse_Submit_Version(ota_context *ctx)
{
    const char *buf = ctx->net.HTTP_body_start_ptr;

    cJSON *root = NULL;
    cJSON *item = NULL;

    root = cJSON_Parse(buf);
    // errno
    item = cJSON_GetObjectItem(root, "errno");
    if(item != NULL)
    {
        switch(item->valueint)
        {
        case 0: {
            cJSON_Delete(root);
            return OTA_OK;
        }
        default:
            break;
        }
    }
    cJSON_Delete(root);
    return OTA_ERROR;
}

uint8_t otaParse_Check_Task(ota_context *ctx)
{
    const char *buf = ctx->net.HTTP_body_start_ptr;

    cJSON *root = NULL;
    cJSON *item = NULL;

    root = cJSON_Parse(buf);

    // errno
    item = cJSON_GetObjectItem(root, "errno");
    if(item != NULL)
    {
        if(item->valueint != TASK_CHECK_SUCCESS)
        {
            switch(item->valueint)
            {
            case TASK_CHECK_NO_TASK: {
                OTA_Event_Handle(ctx, OTA_EVENT_CHECK_TASK_NO_TASK);
                cJSON_Delete(root);
                return OTA_ERROR;
            }
            default: {
                goto _parse_check_task_error_;
            }
            }
        }
    }
    else
    {
        goto _parse_check_task_error_;
    }

    // data
    cJSON *root_data = NULL;
    root_data = cJSON_GetObjectItem(root, "data");
    // target
    item = cJSON_GetObjectItem(root_data, "target");
    if(ota_copy_json_str_item(item, &ctx->task_info.target) == OTA_ERROR)
    {
        goto _parse_check_task_error_;
    }
    // tid
    item = cJSON_GetObjectItem(root_data, "tid");
    if(item == NULL)
    {
        goto _parse_check_task_error_;
    }
    ctx->task_info.tid = item->valueint;
    // size
    item = cJSON_GetObjectItem(root_data, "size");
    if(item == NULL)
    {
        goto _parse_check_task_error_;
    }
    ctx->task_info.size = item->valueint - 1;

    // md5
    item = cJSON_GetObjectItem(root_data, "md5");
    if(ota_copy_json_str_item(item, &ctx->task_info.md5) == OTA_ERROR)
    {
        goto _parse_check_task_error_;
    }
    // type
    item = cJSON_GetObjectItem(root_data, "type");
    if(item == NULL)
    {
        goto _parse_check_task_error_;
    }
    ctx->task_info.type = item->valueint;
    // status
    item = cJSON_GetObjectItem(root_data, "status");
    if(item == NULL)
    {
        goto _parse_check_task_error_;
    }
    ctx->task_info.status = item->valueint;

    cJSON_Delete(root);
    OTA_Event_Handle(ctx, OTA_EVENT_CHECK_TASK_SUCCESS);
    return OTA_OK;

_parse_check_task_error_:
    cJSON_Delete(root);
    OTA_Event_Handle(ctx, OTA_EVENT_CHECK_TASK_OTHER_ERROR);
    return OTA_ERROR;
}

uint8_t otaParse_Validate_TID(ota_context *ctx)
{
    const char *buf = ctx->net.HTTP_body_start_ptr;

    cJSON *root = NULL;
    cJSON *item = NULL;

    root = cJSON_Parse(buf);
    // errno
    item = cJSON_GetObjectItem(root, "errno");

    if(item != NULL)
    {
        switch(item->valueint)
        {
        case TID_VALIDATE_SUCCESS: {
            break;
        }
        default: {
            OTA_Event_Handle(ctx, OTA_EVENT_VALIDATE_TID_OTHER_ERROR);
            break;
        }
        }
    }

    // data
    cJSON *root_data = NULL;
    root_data = cJSON_GetObjectItem(root, "data");
    // status
    item = cJSON_GetObjectItem(root_data, "status");
    if(item == NULL)
    {
        goto _parse_validate_tid_error_;
    }
    ctx->task_info.status = item->valueint;

    cJSON_Delete(root);
    return OTA_OK;

_parse_validate_tid_error_:
    cJSON_Delete(root);
    OTA_Event_Handle(ctx, OTA_EVENT_VALIDATE_TID_OTHER_ERROR);
    return OTA_ERROR;
}

uint8_t otaParse_Download_Package(ota_context *ctx)
{
    ctx_download_info_t *dl_info = &(ctx->download_info);
    if(dl_info->content_range_start == dl_info->range_next_start &&
       dl_info->content_range_end == dl_info->range_next_end)
    {
        OTA_Event_Handle(ctx, OTA_EVENT_DOWNLOAD_OK);
        dl_info->content_range_start = 0;
        dl_info->content_range_end = 0;

        dl_info->range_now = dl_info->range_next_end;
        return OTA_OK;
    }
    else
    {
        ota_log_printf("range doesn't match.%d-%d | %d-%d\n", dl_info->range_next_start, dl_info->range_next_end,
                       dl_info->content_range_start, dl_info->content_range_end);
        OTA_Event_Handle(ctx, OTA_EVENT_DOWNLOAD_ERROR);
        return OTA_ERROR;
    }
}

uint8_t otaParse_Report_Progress(ota_context *ctx)
{
    const char *buf = ctx->net.HTTP_body_start_ptr;

    cJSON *root = NULL;
    cJSON *item = NULL;

    root = cJSON_Parse(buf);
    // errno
    item = cJSON_GetObjectItem(root, "errno");
    if(ctx->state == CTX_STATE_REPORT_DOWNLOAD_PROGRESS)
    {
        if(item != NULL)
        {
            switch(item->valueint)
            {
            case REPORT_SUCCESS: {
                cJSON_Delete(root);
                return OTA_OK;
            }
            default:
                break;
            }
        }
        cJSON_Delete(root);
        return OTA_ERROR;
    }
    else // CTX_STATE_REPORT
    {
        if(item != NULL)
        {
            switch(item->valueint)
            {
            case REPORT_SUCCESS: {
                cJSON_Delete(root);
                return OTA_OK;
            }
            default: {
                OTA_Event_Handle(ctx, OTA_EVENT_custom_report_error_clear);
                break;
            }
            }
        }
        cJSON_Delete(root);
        return OTA_ERROR;
    }
}
