/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
/*
 * adapter
 */
#include "plat_tcp.h"
#include "plat_time.h"
#include "ota_sys.h"

#include "http_parser.h"
#include "ota_api.h"
#include "ota_internal.h"

#include "dev_token.h"

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

// http-parse CB
int ota_status_callback(http_parser *p, const char *buf, size_t len);
int ota_header_field_callback(http_parser *p, const char *buf, size_t len);
int ota_header_value_callback(http_parser *p, const char *buf, size_t len);
int ota_body_callback(http_parser *p, const char *buf, size_t len);
int ota_message_complete_callback(http_parser *p);
int ota_chunk_header_callback(http_parser *p);
int ota_on_chunk_complete_callback(http_parser *p);

static uint8_t OTA_HTTP_Send_Handle(ota_context *ota_ctx, uint32_t timeout_ms);
static uint8_t OTA_HTTP_Recv_Handle(ota_context *ota_ctx, uint32_t timeout_ms);
static uint8_t OTA_Socket_Recreate(ota_context *ota_ctx);

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
static int64_t _http_body_all_size_ = 0;
static uint8_t _is_Content_Length_ = 0; // 0 None; 1 Content_Length; 2 chunked
static uint8_t _is_http_complete_ = 0;
static uint8_t _is_Content_Range_ = 0;
static int8_t _is_Ota_Errno_ = 0;
/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

uint8_t ota_init(void **ota_ctx, const char *product_id, const char *device_name, const char *access_key,
                 uint8_t task_type, const char *firmware_version, const char *software_version, uint32_t recv_max_len,
                 uint16_t check_period_s, uint32_t timeout_ms)
{
    ota_context *ctx;

    ctx = (ota_context *)ota_malloc(sizeof(ota_context));
    if(ctx == NULL)
    {
        ota_log_printf("malloc ota_ctx error\r\n");
        goto _ota_init_exit_;
    }
    ota_memset(ctx, 0, sizeof(ota_context));

    ctx->net.RecvBufferMaxLength = recv_max_len;
    ctx->net.RecvBuffer = ota_malloc(recv_max_len);
    if(ctx->net.RecvBuffer == NULL)
    {
        goto _ota_init_exit_;
    }

    if(task_type != 1 && task_type != 2)
    {
        goto _ota_init_exit_;
    }
    ctx->device_info.ota_type = task_type;

    ctx->device_info.product_id_len = ota_strlen(product_id);
    ctx->device_info.product_id = ota_malloc(ctx->device_info.product_id_len + 1);
    if(ctx->device_info.product_id == NULL)
    {
        goto _ota_init_exit_;
    }
    ota_memset(ctx->device_info.product_id, 0, ctx->device_info.product_id_len + 1);
    ota_memcpy(ctx->device_info.product_id, product_id, ctx->device_info.product_id_len);

    ctx->device_info.dev_name_len = ota_strlen(device_name);
    ctx->device_info.dev_name = ota_malloc(ctx->device_info.dev_name_len + 1);
    if(ctx->device_info.dev_name == NULL)
    {
        goto _ota_init_exit_;
    }
    ota_memset(ctx->device_info.dev_name, 0, ctx->device_info.dev_name_len + 1);
    ota_memcpy(ctx->device_info.dev_name, device_name, ctx->device_info.dev_name_len);

    if(firmware_version != NULL)
    {
        ctx->device_info.f_version_len = ota_strlen(firmware_version);
        ctx->device_info.f_version = ota_malloc(ctx->device_info.f_version_len + 1);
        if(ctx->device_info.f_version == NULL)
        {
            goto _ota_init_exit_;
        }
        ota_memset(ctx->device_info.f_version, 0, ctx->device_info.f_version_len + 1);
        ota_memcpy(ctx->device_info.f_version, firmware_version, ctx->device_info.f_version_len);
    }

    if(software_version != NULL)
    {
        ctx->device_info.s_version_len = ota_strlen(software_version);
        ctx->device_info.s_version = ota_malloc(ctx->device_info.s_version_len + 1);
        if(ctx->device_info.s_version == NULL)
        {
            goto _ota_init_exit_;
        }
        ota_memset(ctx->device_info.s_version, 0, ctx->device_info.s_version_len + 1);
        ota_memcpy(ctx->device_info.s_version, software_version, ctx->device_info.s_version_len);
    }

    ctx->device_info.access_key_len = ota_strlen(access_key);
    ctx->device_info.access_key = ota_malloc(ctx->device_info.access_key_len + 1);
    if(ctx->device_info.access_key == NULL)
    {
        goto _ota_init_exit_;
    }
    ota_memset(ctx->device_info.access_key, 0, ctx->device_info.access_key_len + 1);
    ota_memcpy(ctx->device_info.access_key, access_key, ctx->device_info.access_key_len);

    ctx->net.socket_fd = tcp_connect(OTA_SERVER_ADDR, OTA_SERVER_PORT, timeout_ms);
    if(ctx->net.socket_fd < 0)
    {
        goto _ota_init_exit_;
    }

    ctx->ota_http_parser = (http_parser *)ota_malloc(sizeof(http_parser));
    ota_memset(ctx->ota_http_parser, 0, sizeof(http_parser));
    if(!ctx->ota_http_parser)
    {
        goto _ota_init_exit_;
    }
    ctx->ota_http_parser->data = ctx;

    ctx->ota_http_parser_settings = (http_parser_settings *)ota_malloc(sizeof(http_parser_settings));
    ota_memset(ctx->ota_http_parser_settings, 0, sizeof(http_parser_settings));
    if(!ctx->ota_http_parser)
    {
        goto _ota_init_exit_;
    }
    ctx->ota_http_parser_settings->on_status = ota_status_callback;
    ctx->ota_http_parser_settings->on_header_field = ota_header_field_callback;
    ctx->ota_http_parser_settings->on_header_value = ota_header_value_callback;
    ctx->ota_http_parser_settings->on_body = ota_body_callback;
    ctx->ota_http_parser_settings->on_message_complete = ota_message_complete_callback;
    ctx->ota_http_parser_settings->on_chunk_header = ota_chunk_header_callback;
    ctx->ota_http_parser_settings->on_chunk_complete = ota_on_chunk_complete_callback;

    ctx->check_timer.check_task_period = check_period_s;

    *ota_ctx = ctx;
    ctx->state = CTX_STATE_SUBMIT_VERSION;
    ota_scene_recall(ctx);

    if(ctx->device_info.authorization == NULL)
    {
        ctx->device_info.authorization = ota_malloc(256);
        if(ctx->device_info.authorization == NULL)
        {
            goto _ota_init_exit_;
        }
        ota_memset(ctx->device_info.authorization, 0, 256);

        dev_token_generate((int8_t *)ctx->device_info.authorization, SIG_METHOD_SHA1, 2524579200,
                           (const int8_t *)product_id, (const int8_t *)device_name, (const int8_t *)access_key);
        ota_log_printf("authorization: %s\n", ctx->device_info.authorization);
    }

    ota_log_printf("ota init ok\n");
    return OTA_OK;

_ota_init_exit_:
    ota_deinit(ctx);
    ota_log_printf("ota init error\n");
    return OTA_ERROR;
}

uint8_t ota_deinit(ota_context *ota_ctx)
{
    ota_log_printf("Deinit\r\n");
    ota_context *ctx = (ota_context *)ota_ctx;

    ota_deinit_free(ctx->device_info.product_id);
    ota_deinit_free(ctx->device_info.dev_name);

    ota_deinit_free(ctx->device_info.f_version);
    ota_deinit_free(ctx->device_info.s_version);

    ota_deinit_free(ctx->device_info.access_key);
    ota_deinit_free(ctx->device_info.authorization);

    ota_deinit_free(ctx->net.SendBuffer);
    ota_deinit_free(ctx->net.RecvBuffer);

    ota_deinit_free(ctx->task_info.md5);
    ota_deinit_free(ctx->task_info.target);
    ota_deinit_free(ctx->ota_http_parser);
    ota_deinit_free(ctx->ota_http_parser_settings);

    ota_memset(ctx, 0, sizeof(ota_context));
    ota_deinit_free(ctx);

    return OTA_OK;
}

uint8_t OTA_Submit_Version(ota_context *ota_ctx, uint32_t timeout_ms)
{
    ota_handle_t countdown_tmr = 0;
    uint8_t ret = OTA_ERROR;

    if(0 == (countdown_tmr = countdown_start(timeout_ms)))
    {
        return OTA_ERROR;
    }

    if(otaPack_Submit_Version(ota_ctx, SUBMIT_VERSION_BUF_MAX_LEN) == OTA_ERROR)
    {
        goto _exit_;
    }
    ota_log_printf("[ota] Submit Version\r\n\r\n");

    if(OTA_HTTP_Send_Handle(ota_ctx, timeout_ms) != OTA_OK)
    {
        goto _exit_;
    }

    if(OTA_HTTP_Recv_Handle(ota_ctx, countdown_left(countdown_tmr)) == OTA_OK)
    {
        if(otaParse_Submit_Version(ota_ctx) == OTA_OK)
        {
            OTA_Event_Handle(ota_ctx, OTA_EVENT_SUBMIT_VERSION_OK);
            ret = OTA_OK;
            goto _exit_;
        }
    }
    OTA_Event_Handle(ota_ctx, OTA_EVENT_SUBMIT_VERSION_ERROR);

_exit_:
    countdown_stop(countdown_tmr);
    return ret;
}

uint8_t OTA_Check_Task(ota_context *ota_ctx, uint32_t timeout_ms)
{
    ota_handle_t countdown_tmr = 0;
    uint8_t ret = OTA_ERROR;

    if(0 == (countdown_tmr = countdown_start(timeout_ms)))
    {
        return OTA_ERROR;
    }

    if(otaPack_Check_Task(ota_ctx, CHECK_TASK_BUF_MAX_LEN) == OTA_ERROR)
    {
        goto _exit_;
    }
    ota_log_printf("[ota] Check Task\r\n\r\n");

    if(OTA_HTTP_Send_Handle(ota_ctx, timeout_ms) != OTA_OK)
    {
        goto _exit_;
    }

    if(OTA_HTTP_Recv_Handle(ota_ctx, countdown_left(countdown_tmr)) == OTA_OK)
    {
        if(otaParse_Check_Task(ota_ctx) == OTA_OK)
        {
            ret = OTA_OK;
            goto _exit_;
        }
    }
_exit_:
    countdown_stop(countdown_tmr);
    return ret;
}

uint8_t OTA_Validate_TID(ota_context *ota_ctx, uint32_t timeout_ms)
{
    ota_handle_t countdown_tmr = 0;
    uint8_t ret = OTA_ERROR;

    if(0 == (countdown_tmr = countdown_start(timeout_ms)))
    {
        return OTA_ERROR;
    }

    if(otaPack_Validate_TID(ota_ctx, VALIDATE_TID_BUF_MAX_LEN) == OTA_ERROR)
    {
        goto _exit_;
    }
    ota_log_printf("[ota] Validate TID\r\n\r\n");

    if(OTA_HTTP_Send_Handle(ota_ctx, timeout_ms) != OTA_OK)
    {
        goto _exit_;
    }

    if(OTA_HTTP_Recv_Handle(ota_ctx, countdown_left(countdown_tmr)) == OTA_OK)
    {
        if(otaParse_Validate_TID(ota_ctx) == OTA_OK)
        {
            OTA_Event_Handle(ota_ctx, OTA_EVENT_VALIDATE_TID_OK);
            ret = OTA_OK;
        }
        goto _exit_;
    }
    OTA_Event_Handle(ota_ctx, OTA_EVENT_custom_retry_download);
_exit_:
    countdown_stop(countdown_tmr);
    return ret;
}

uint8_t OTA_Download_Package(ota_context *ota_ctx, uint32_t segment_start, uint32_t segment_end, uint32_t timeout_ms)
{
    ota_handle_t countdown_tmr = 0;
    uint8_t ret = OTA_ERROR;

    if(segment_end > ota_ctx->task_info.size)
    {
        segment_end = ota_ctx->task_info.size;
    }

    if(segment_start > segment_end || 0 == (countdown_tmr = countdown_start(timeout_ms)))
    {
        return OTA_ERROR;
    }

    ota_ctx->download_info.range_next_start = segment_start;
    ota_ctx->download_info.range_next_end = segment_end;

    ota_log_printf("start get range %d - %d\r\n", ota_ctx->download_info.range_next_start,
                   ota_ctx->download_info.range_next_end);

    if(otaPack_Download_Package(ota_ctx, DOWNLOAD_PACKAGE_BUF_MAX_LEN) == OTA_ERROR)
    {
        goto _exit_;
    }
    ota_log_printf("[ota] Download Package\r\n\r\n");

    if(OTA_HTTP_Send_Handle(ota_ctx, timeout_ms) != OTA_OK)
    {
        goto _exit_;
    }

    if(OTA_HTTP_Recv_Handle(ota_ctx, countdown_left(countdown_tmr)) == OTA_OK)
    {
        if(otaParse_Download_Package(ota_ctx) == OTA_ERROR)
        {
            OTA_Event_Handle(ota_ctx, OTA_EVENT_custom_retry_download);
        }
        else
        {
            ret = OTA_OK;
            goto _exit_;
        }
    }
    else if(ota_ctx->download_info.download_errno != 0)
    {
        ota_log_printf("download errno: %d \r\n", ota_ctx->download_info.download_errno);
        switch(ota_ctx->download_info.download_errno)
        {
        case 5: {
            ota_log_printf("task canceled\r\n");
            ota_ctx->state = CTX_STATE_FINISH_UPDATE;
            break;
        }
        default:
            OTA_Event_Handle(ota_ctx, OTA_EVENT_REPORT_DOWNLOAD_UNKNOWN_ERROR);
            break;
        }
        ota_ctx->download_info.download_errno = 0;
    }
_exit_:
    countdown_stop(countdown_tmr);
    return ret;
}

uint8_t OTA_Update_Progress(ota_context *ota_ctx, uint32_t timeout_ms)
{
    ota_handle_t countdown_tmr = 0;
    uint8_t ret = OTA_ERROR;

    if(0 == (countdown_tmr = countdown_start(timeout_ms)))
    {
        return OTA_ERROR;
    }

    if(otaPack_Update_Progress(ota_ctx, UPDATE_PROGRESS_BUF_MAX_LEN) == OTA_ERROR)
    {
        goto _exit_;
    }
    ota_log_printf("[ota] Update Progress\r\n\r\n");

    if(OTA_HTTP_Send_Handle(ota_ctx, timeout_ms) != OTA_OK)
    {
        goto _exit_;
    }

    if(OTA_HTTP_Recv_Handle(ota_ctx, countdown_left(countdown_tmr)) == OTA_OK)
    {
        if(otaParse_Report_Progress(ota_ctx) == OTA_OK)
        {
            if(ota_ctx->state == CTX_STATE_REPORT)
            {
                OTA_Event_Handle(ota_ctx, OTA_EVENT_REPORT_SUCCESS);
            }
            else
            {
                OTA_Event_Handle(ota_ctx, OTA_EVENT_REPORT_DOWNLOAD_PROGRESS_OK);
            }

            ret = OTA_OK;
            goto _exit_;
        }
    }

    if(ota_ctx->state == CTX_STATE_REPORT)
    {
        OTA_Event_Handle(ota_ctx, OTA_EVENT_REPORT_FAILED);
    }
    else
    {
        OTA_Event_Handle(ota_ctx, OTA_EVENT_REPORT_DOWNLOAD_PROGRESS_ERROR);
    }

_exit_:
    countdown_stop(countdown_tmr);
    return ret;
}
static uint8_t OTA_HTTP_Send_Handle(ota_context *ota_ctx, uint32_t timeout_ms)
{
    int32_t _send_len_ = 0;
    int32_t _send_ret_ = 0;

    _send_len_ = ota_strlen(ota_ctx->net.SendBuffer);
    _send_ret_ = tcp_send(ota_ctx->net.socket_fd, ota_ctx->net.SendBuffer, _send_len_, timeout_ms);

    ota_deinit_free(ota_ctx->net.SendBuffer);
    ota_ctx->net.SendBuffer = NULL;

    if(_send_ret_ != _send_len_)
    {
        if(_send_ret_ == -1)
        {
            OTA_Socket_Recreate(ota_ctx);
        }
        return OTA_ERROR;
    }
    return OTA_OK;
}

static uint8_t OTA_HTTP_Recv_Handle(ota_context *ota_ctx, uint32_t timeout_ms)
{
    ctx_net_t *net_ctx = &(ota_ctx->net);

    uint32_t recv_len = 0;
    ota_handle_t countdown_tmr = 0;

    uint32_t all_recv_len = 0;
    uint32_t last_parse_len = 0;

    _is_Content_Range_ = 0;
    _is_Content_Length_ = 0;
    _is_http_complete_ = 0;
    _is_Ota_Errno_ = 0;
    _http_body_all_size_ = 0;
    net_ctx->HTTP_body_length = 0;
    net_ctx->HTTP_body_start_ptr = NULL;

    if(0 == (countdown_tmr = countdown_start(timeout_ms)))
    {
        return OTA_ERROR;
    }

    ota_memset(net_ctx->RecvBuffer, 0, net_ctx->RecvBufferMaxLength);
    http_parser_init(ota_ctx->ota_http_parser, HTTP_RESPONSE);

    do
    {
        recv_len = tcp_recv(net_ctx->socket_fd, net_ctx->RecvBuffer + all_recv_len,
                            net_ctx->RecvBufferMaxLength - all_recv_len, 20);
        if(recv_len > 0)
        {
            all_recv_len += recv_len;
            ota_log_printf("tcp_recv %d bytes, all %d bytes\n", recv_len, all_recv_len);
        }
        if(last_parse_len != all_recv_len)
        {
            http_parser_init(ota_ctx->ota_http_parser, HTTP_RESPONSE);
            http_parser_execute(ota_ctx->ota_http_parser, ota_ctx->ota_http_parser_settings, net_ctx->RecvBuffer,
                                all_recv_len);
            last_parse_len = all_recv_len;
        }
    } while(countdown_is_expired(countdown_tmr) == 0 && _is_http_complete_ == 0);

    if(_is_Ota_Errno_ != 0)
    {
        ota_log_printf("[ota-error] download error\r\n");
        ota_ctx->download_info.download_errno = (uint8_t)_is_Ota_Errno_;
        goto _recv_return_error_without_recreate_socket_;
    }

    if(all_recv_len == 0)
    {
        ota_log_printf("[ota-error] all_recv_len is 0\r\n");
        goto _recv_return_error_;
    }

    if(net_ctx->HTTP_body_length == 0 && net_ctx->HTTP_body_start_ptr == NULL)
    {
        ota_log_printf("[ota-error] body is null\r\n");
        goto _recv_return_error_;
    }

    if(_is_http_complete_ == 1)
    {
        if(ota_ctx->device_info.authorization != NULL && _is_Content_Length_ != 1 &&
           ota_ctx->state == CTX_STATE_DOWNLOAD_PACKAGE)
        {
            ota_log_printf("[ota-error] packet wrong\r\n");
            goto _recv_return_error_without_recreate_socket_;
        }
        goto _recv_return_ok_;
    }
    else
    {
        ota_log_printf("[ota-error] parse is't completed\r\n");
        goto _recv_return_error_;
    }

_recv_return_ok_:
    ota_log_printf("\nSpending %dms\n", timeout_ms - countdown_left(countdown_tmr));

    countdown_stop(countdown_tmr);
    return OTA_OK;
_recv_return_error_:
    OTA_Socket_Recreate(ota_ctx);
_recv_return_error_without_recreate_socket_:
    net_ctx->HTTP_body_length = 0;
    countdown_stop(countdown_tmr);
    return OTA_ERROR;
}

static uint8_t OTA_Socket_Recreate(ota_context *ota_ctx)
{
    ota_log_printf("fall in recreate socket\n");
    tcp_disconnect(ota_ctx->net.socket_fd);
    ota_ctx->net.socket_fd = tcp_connect(OTA_SERVER_ADDR, OTA_SERVER_PORT, 1000);
    if(ota_ctx->net.socket_fd < 0)
    {
        return OTA_ERROR;
    }
    return OTA_OK;
}

int ota_status_callback(http_parser *p, const char *buf, size_t len)
{
    // ota_log_printf("Status: %d\n", p->status_code);
    return 0;
}

int ota_header_field_callback(http_parser *p, const char *buf, size_t len)
{
    //ota_log_printf("Header field: %.*s\n", (int)len, buf);
    if((buf[0] == 'C' || buf[0] == 'O') && (len == 9 || len == 14 || len == 17 || len == 19))
    {
        char field_buff[20] = { 0 };
        ota_memcpy(field_buff, buf, len);
        if(_is_Content_Length_ == 0 && strcmp(field_buff, "Content-Length") == 0)
        {
            _is_Content_Length_ = 1;
        }
        else if(_is_Content_Range_ == 0 && strcmp(field_buff, "Content-Range-Start") == 0)
        {
            _is_Content_Range_ = 1;
        }
        else if(_is_Content_Range_ == 0 && strcmp(field_buff, "Content-Range-End") == 0)
        {
            _is_Content_Range_ = 2;
        }
        else if(_is_Content_Length_ == 0 && strcmp(field_buff, "Ota-Errno") == 0)
        {
            _is_Ota_Errno_ = -1;
        }
    }
    return 0;
}

int ota_header_value_callback(http_parser *p, const char *buf, size_t len)
{
    ota_context *ctx = (ota_context *)p->data;

    char atoibuff[32] = { 0 };
    if(_is_Content_Length_ == 1 && _http_body_all_size_ == 0)
    {
        ota_memcpy(atoibuff, buf, len);
        _http_body_all_size_ = ota_atoi(atoibuff);
    }
    else if(_is_Content_Range_ == 1 && ctx->download_info.content_range_start == 0)
    {
        ota_memcpy(atoibuff, buf, len);
        ctx->download_info.content_range_start = ota_atoi(atoibuff);
        _is_Content_Range_ = 0;
    }
    else if(_is_Content_Range_ == 2 && ctx->download_info.content_range_end == 0)
    {
        ota_memcpy(atoibuff, buf, len);
        ctx->download_info.content_range_end = ota_atoi(atoibuff);
        _is_Content_Range_ = 0;
    }
    else if(_is_Ota_Errno_ == -1)
    {
        ota_memcpy(atoibuff, buf, len);
        _is_Ota_Errno_ = (uint8_t)ota_atoi(atoibuff);
    }
    return 0;
}

int ota_body_callback(http_parser *p, const char *buf, size_t len)
{
    ota_context *ctx = (ota_context *)p->data;
    ctx->net.HTTP_body_start_ptr = buf;
    ctx->net.HTTP_body_length = len;
    return 0;
}

int ota_message_complete_callback(http_parser *p)
{
    ota_log_printf("http message complete\n");
    ota_context *ctx = (ota_context *)p->data;
    if(ctx->state != CTX_STATE_DOWNLOAD_PACKAGE)
    {
        ota_log_printf("%s", ctx->net.RecvBuffer);
    }
    else
    {
        ota_log_printf("%.*s", (ctx->net.HTTP_body_start_ptr - ctx->net.RecvBuffer), ctx->net.RecvBuffer);

        uint8_t _max_printf_len = _http_body_all_size_ > 10 ? 10 : _http_body_all_size_;
        for(int i = 0; i < _max_printf_len; i++)
        {
            ota_log_printf("%02x ", (unsigned char)(ctx->net.HTTP_body_start_ptr)[i]);
        }
        ota_log_printf("\r\n");
    }
    ota_log_printf("HTTP Body Len: %d\r\n", ctx->net.HTTP_body_length);

    _is_http_complete_ = 1;
    return 0;
}

int ota_chunk_header_callback(http_parser *p)
{
    _http_body_all_size_ = p->content_length;
    _is_Content_Length_ = 2;
    return 0;
}

int ota_on_chunk_complete_callback(http_parser *p)
{
    return 0;
}