/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "http_parser.h"
#include "ota_api.h"
#include "ota_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * adapter
 */
#include "plat_tcp.h"
#include "ota_sys.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro)                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

uint8_t ota_loop(void *ota_ctx, uint32_t timeout_ms)
{
    ota_context *ctx = (ota_context *)ota_ctx;

    switch(ctx->state)
    {
    case CTX_STATE_SUBMIT_VERSION: {
        return OTA_Submit_Version(ctx, timeout_ms);
    }
    case CTX_STATE_CHECK_TASK: {
        if(ctx->check_timer.check_task_period == 0 || ctx->check_timer.last_check_time == 0 ||
           ota_get_tick_ms() - ctx->check_timer.last_check_time >= ctx->check_timer.check_task_period * 1000)
        {
            ctx->check_timer.last_check_time = ota_get_tick_ms();
            return OTA_Check_Task(ctx, timeout_ms);
        }
        break;
    }
    case CTX_STATE_VALIDATE_TID: {
        if(OTA_Validate_TID(ctx, timeout_ms) == OTA_OK)
        {
            if(ctx->task_info.status < 3)
            {
                ctx->state = CTX_STATE_DOWNLOAD_PACKAGE;
                return OTA_OK;
            }
            else
            {
                ota_log_printf("task status error\r\n");
                ctx->state = CTX_STATE_FINISH_UPDATE;
            }
        }
        return OTA_ERROR;
    }
    case CTX_STATE_DOWNLOAD_PACKAGE: {
        uint32_t _start_ = 0;
        uint32_t _end_ = 0;
        ota_calculate_segment_range(ctx->download_info.range_now, &_start_, &_end_);
        if(_end_ - _start_ + 350 > ctx->net.RecvBufferMaxLength)
        {
            ota_log_printf("RecvBufferMaxLength too small\r\n");
            return OTA_ERROR;
        }
        if(OTA_Download_Package(ctx, _start_, _end_, timeout_ms) == OTA_OK)
        {
            if(ctx->state != CTX_STATE_REPORT_DOWNLOAD_PROGRESS)
            {
                ctx->state = CTX_STATE_VALIDATE_TID;
            }
            return OTA_OK;
        }
        return OTA_ERROR;
    }
    case CTX_STATE_REPORT_DOWNLOAD_PROGRESS: {
        ota_log_printf("report download progress: %d%%\r\n", ctx->download_info.download_progress);
        return OTA_Update_Progress(ctx, timeout_ms);
    }
    case CTX_STATE_REPORT: {
        ota_log_printf("report code: %d \r\n", ctx->report_code);
        return OTA_Update_Progress(ctx, timeout_ms);
    }
    case CTX_STATE_UPDATE: {
        OTA_Event_Handle(ctx, OTA_EVENT_custom_ready_update);
        break;
    }
    case CTX_STATE_FINISH_UPDATE: {
        return CTX_STATE_FINISH_UPDATE;
    }
    case CTX_STATE_NULL: {
        break;
    }
    default:
        ota_log_printf("error in ctx state\r\n");
        return OTA_ERROR;
        break;
    }

    return OTA_OK;
}

uint8_t OTA_Event_Handle(void *context, ota_event_code_t EVENT)
{
    ota_context *ctx = (ota_context *)context;
    switch(EVENT)
    {
    case OTA_EVENT_SUBMIT_VERSION_OK: {
        ota_log_printf("\r\nSubmit Version Success\r\n\r\n");
        ctx->state = CTX_STATE_CHECK_TASK;
        break;
    }
    case OTA_EVENT_SUBMIT_VERSION_ERROR: {
        ota_log_printf("\r\nSubmit Version Error\r\n\r\n");
        break;
    }
    case OTA_EVENT_CHECK_TASK_SUCCESS: {
        ctx->state = CTX_STATE_VALIDATE_TID;
        ota_log_printf("\r\nCheck OTATask Success\r\n\r\n");

        ota_scene_store(ctx);

        OTA_Event_Handle(ctx, OTA_EVENT_custom_delete_package);
        break;
    }
    case OTA_EVENT_CHECK_TASK_NO_TASK: {
        ota_log_printf("ERROR CODE: %d. OTA Task isn't exist.\r\n", TASK_CHECK_NO_TASK);
        if(ctx->check_timer.check_task_period == 0)
        {
            ctx->state = CTX_STATE_FINISH_UPDATE;
        }
        break;
    }
    case OTA_EVENT_CHECK_TASK_OTHER_ERROR: {
        ota_log_printf("Check Task Other Error. Body: %.*s\n", ctx->net.HTTP_body_length,
                       (char *)(ctx->net.HTTP_body_start_ptr));
        ctx->state = CTX_STATE_FINISH_UPDATE;
        break;
    }
    case OTA_EVENT_VALIDATE_TID_OK: {
        ota_log_printf("\r\nValidate OTA TID Success\r\n\r\n");
        break;
    }
    case OTA_EVENT_VALIDATE_TID_OTHER_ERROR: {
        ota_log_printf("Validate TID Other Error. Body: %.*s\n", ctx->net.HTTP_body_length,
                       (char *)(ctx->net.HTTP_body_start_ptr));
        ota_log_printf("Destroy OTA Context\r\n");
        ctx->state = CTX_STATE_FINISH_UPDATE;
        break;
    }
    case OTA_EVENT_DOWNLOAD_OK: {
        ota_log_printf("\r\nDOWNLOAD Success\r\n\r\n");
        OTA_Event_Handle(ctx, OTA_EVENT_custom_save_packet);
        OTA_Event_Handle(ctx, OTA_EVENT_REPORT_DOWNLOAD_PROGRESS);
        break;
    }
    case OTA_EVENT_REPORT_DOWNLOAD_PROGRESS: {
        /*Modify*/
        uint64_t progress = ctx->download_info.range_next_end;
        progress = progress * 100 / ctx->task_info.size;
        if(progress - ctx->download_info.download_progress >= 10 ||
           ctx->download_info.range_next_end == ctx->task_info.size)
        {
            ctx->download_info.download_progress = progress;
            ctx->state = CTX_STATE_REPORT_DOWNLOAD_PROGRESS;
        }
        /*Modify*/
        break;
    }
    case OTA_EVENT_REPORT_DOWNLOAD_PROGRESS_OK: {
        ota_log_printf("Report download progress success\r\n");
        if(ctx->download_info.range_next_end == ctx->task_info.size)
        {
            ctx->state = CTX_STATE_UPDATE;
            break;
        }
        ctx->state = CTX_STATE_DOWNLOAD_PACKAGE;
        break;
    }
    case OTA_EVENT_REPORT_DOWNLOAD_PROGRESS_ERROR: {
        ota_log_printf("Report download progress failed\r\n");
        break;
    }
    case OTA_EVENT_REPORT_SUCCESS: {
        ota_log_printf("Report OK\r\n");
        if(ctx->state == CTX_STATE_REPORT && ctx->report_code == OTA_EVENT_REPORT_DOWNLOAD_SUCCESS)
        {
            ctx->state = CTX_STATE_UPDATE;
        }
        else
        {
            ota_log_printf("Destroy OTA Context\r\n");
            ctx->state = CTX_STATE_FINISH_UPDATE;
        }
        break;
    }
    case OTA_EVENT_REPORT_FAILED: {
        ota_log_printf("Report Failed. Body: %s\r\n", (char *)(ctx->net.HTTP_body_start_ptr));
        break;
    }
    case OTA_EVENT_REPORT_DOWNLOAD_SUCCESS: {
        ctx->state = CTX_STATE_REPORT;
        ctx->report_code = OTA_EVENT_REPORT_DOWNLOAD_SUCCESS;
        ota_log_printf("report: Download success\r\n");
        break;
    }
    case OTA_EVENT_REPORT_DOWNLOAD_UNKNOWN_ERROR: {
        ota_log_printf("Download unknown error\r\n");
        ctx->state = CTX_STATE_REPORT;
        ctx->report_code = OTA_EVENT_REPORT_DOWNLOAD_UNKNOWN_ERROR;
        break;
    }
    case OTA_EVENT_REPORT_UPDATE_SUCCESS: {
        ctx->state = CTX_STATE_REPORT;
        ctx->report_code = OTA_EVENT_REPORT_UPDATE_SUCCESS;
        break;
    }
    case OTA_EVENT_REPORT_UPDATE_UNKNOWN_ERROR: {
        ctx->state = CTX_STATE_REPORT;
        ctx->report_code = OTA_EVENT_REPORT_UPDATE_UNKNOWN_ERROR;
        break;
    }
    case OTA_EVENT_custom_ready_update: {
        FILE *fsread;
        fsread = fopen("package", "rb");
        if(fsread == NULL)
        {
            printf("open file failed!\n");
            exit(1);
        }
        else
        {
            ota_log_printf("\r\nMD5 check success\r\nUpdate...OK!\r\n\r\n");
            ctx->state = CTX_STATE_REPORT;
            ctx->report_code = OTA_EVENT_REPORT_UPDATE_SUCCESS;
            if(ctx->task_info.type == 1)
            {
                ota_free(ctx->device_info.f_version);
                ctx->device_info.f_version_len = ota_strlen(ctx->task_info.target);
                ctx->device_info.f_version = ota_malloc(ctx->device_info.f_version_len + 1);
                ota_memset((void *)ctx->device_info.f_version, 0, ctx->device_info.f_version_len);
                ota_strcpy((char *)ctx->device_info.f_version, ctx->task_info.target);
            }
            else if(ctx->task_info.type == 2)
            {
                ota_free(ctx->device_info.s_version);
                ctx->device_info.s_version_len = ota_strlen(ctx->task_info.target);
                ctx->device_info.s_version = ota_malloc(ctx->device_info.s_version_len + 1);
                ota_memset((void *)ctx->device_info.s_version, 0, ctx->device_info.s_version_len);
                ota_strcpy((char *)ctx->device_info.s_version, ctx->task_info.target);
            }
        }
        fclose(fsread);
        break;
    }
    case OTA_EVENT_custom_retry_download: {
        ota_log_printf("will retry\r\n");
        break;
    }
    case OTA_EVENT_custom_save_packet: {
        FILE *fswrite;
        fswrite = fopen("package", "ab+");
        if(fswrite == NULL)
        {
            printf("open file failed!\n");
            exit(1);
        }
        else
        {
            fwrite(ctx->net.HTTP_body_start_ptr, sizeof(char), ctx->net.HTTP_body_length, fswrite);
        }
        fclose(fswrite);
        break;
    }
    case OTA_EVENT_custom_report_error_clear: {
        ota_log_printf("Destroy OTA Context, Because: \r\n%s\r\n", (char *)(ctx->net.HTTP_body_start_ptr));
        ctx->state = CTX_STATE_FINISH_UPDATE;
        break;
    }
    case OTA_EVENT_custom_delete_package: {
        ota_log_printf("delete package\r\n");
        remove("package");
        break;
    }
    default:
        return OTA_ERROR;
    }

    return OTA_OK;
}
