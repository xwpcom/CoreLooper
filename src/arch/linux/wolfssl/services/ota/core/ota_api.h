#ifndef _OTA_API_H_
#define _OTA_API_H_

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "http_parser.h"
#include "plat_tcp.h"

#include <stdint.h>
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition (Constant and Macro)                                 */
/*****************************************************************************/

#define OTA_SERVER_ADDR "183.230.40.50"
#define OTA_SERVER_PORT 80
#define OTA_HOST "studio-ota.heclouds.com"

#define SUBMIT_VERSION_BUF_MAX_LEN (600)
#define CHECK_TASK_BUF_MAX_LEN (420)
#define VALIDATE_TID_BUF_MAX_LEN (420)
#define DOWNLOAD_PACKAGE_BUF_MAX_LEN (420)
#define UPDATE_PROGRESS_BUF_MAX_LEN (480)

//m Bytes
#define RECV_BUFFER_MAX (900 * 1024)

// TASK ERROR CODE
#define TASK_CHECK_SUCCESS (0)
#define TASK_CHECK_NO_TASK (10)

// TID ERROR CODE
#define TID_VALIDATE_SUCCESS (0)

// REPORT ERROR CODE
#define REPORT_SUCCESS (0)

// EVENT CODE

typedef enum ota_event_code
{
    // custom ~ - -1
    OTA_EVENT_custom_delete_package = -20,
    OTA_EVENT_custom_report_error_clear,
    OTA_EVENT_custom_save_packet,
    OTA_EVENT_custom_retry_download,
    OTA_EVENT_custom_ready_update,

    // OTA_EVENT_SUBMIT_VERSION 1 - 19
    OTA_EVENT_SUBMIT_VERSION_OK = 1,
    OTA_EVENT_SUBMIT_VERSION_ERROR,

    // OTA_EVENT_CHECK_TASK 20 - 29
    OTA_EVENT_CHECK_TASK_SUCCESS = 20,
    OTA_EVENT_CHECK_TASK_NO_TASK,
    OTA_EVENT_CHECK_TASK_OTHER_ERROR,

    // OTA_EVENT_VALIDATE_TID 30 - 49

    OTA_EVENT_VALIDATE_TID_OK = 30,
    OTA_EVENT_VALIDATE_TID_OTHER_ERROR,

    // OTA_EVENT_DOWNLOAD 50 - 59

    OTA_EVENT_DOWNLOAD_OK = 50,
    OTA_EVENT_DOWNLOAD_ERROR,

    // OTA_EVENT_REPORT 60 - ~

    OTA_EVENT_REPORT_DOWNLOAD_PROGRESS = 60,
    OTA_EVENT_REPORT_DOWNLOAD_PROGRESS_OK,
    OTA_EVENT_REPORT_DOWNLOAD_PROGRESS_ERROR,

    OTA_EVENT_REPORT_SUCCESS,
    OTA_EVENT_REPORT_FAILED,

    OTA_EVENT_REPORT_DOWNLOAD_SUCCESS = 101,
    OTA_EVENT_REPORT_DOWNLOAD_LOW_POWER_ERROR = 105,
    OTA_EVENT_REPORT_DOWNLOAD_BAD_SIGNAL_ERROR = 106,
    OTA_EVENT_REPORT_DOWNLOAD_UNKNOWN_ERROR = 107,
    OTA_EVENT_REPORT_UPDATE_SUCCESS = 201,
    OTA_EVENT_REPORT_UPDATE_UNKNOWN_ERROR = 206
} ota_event_code_t;

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
typedef enum
{
    CTX_STATE_SUBMIT_VERSION = 2,
    CTX_STATE_CHECK_TASK,
    CTX_STATE_VALIDATE_TID,
    CTX_STATE_DOWNLOAD_PACKAGE,
    CTX_STATE_REPORT_DOWNLOAD_PROGRESS,
    CTX_STATE_REPORT,
    CTX_STATE_UPDATE,
    CTX_STATE_FINISH_UPDATE,
    CTX_STATE_NULL
} ctx_state_t;

typedef struct
{
    int socket_fd;

    char *SendBuffer;

    char *RecvBuffer;
    uint32_t RecvBufferMaxLength;
    uint32_t RecvBufferCurrentLength;

    const char *HTTP_body_start_ptr;
    uint32_t HTTP_body_length;

} ctx_net_t;

typedef struct
{
    char *product_id;
    uint16_t product_id_len;

    char *dev_name;
    uint16_t dev_name_len;

    char *f_version;
    uint16_t f_version_len;

    char *s_version;
    uint16_t s_version_len;

    char *access_key;
    uint16_t access_key_len;

    uint8_t ota_type;

    char *authorization;
    uint16_t authorization_len;
} ctx_device_info_t;

typedef struct
{
    char *target;
    uint32_t tid;
    uint32_t size;
    char *md5;
    uint8_t type; //1: 完整包，2：差分包
    uint8_t status; //1：待升级，2：下载中，3：升级中，4: 升级成功，5: 升级失败，6: 升级取消
} ctx_task_info_t;

typedef struct
{
    time_t check_task_period;
    uint64_t last_check_time;
} ctx_check_task_timer_t;

typedef struct
{
    uint32_t range_next_end;
    uint32_t range_next_start;
    uint32_t range_now;

    uint32_t content_range_start;
    uint32_t content_range_end;

    uint8_t download_errno;
    uint8_t download_progress;
} ctx_download_info_t;

typedef struct
{
    ctx_state_t state;
    uint16_t report_code;
    ctx_check_task_timer_t check_timer;

    ctx_device_info_t device_info;
    ctx_task_info_t task_info;
    ctx_download_info_t download_info;

    ctx_net_t net;

    http_parser *ota_http_parser;
    http_parser_settings *ota_http_parser_settings;

} ota_context;
/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 初始化ota上下文
 *
 * @param ota_ctx 待初始化的上下文指针地址
 * @param product_id 产品ID
 * @param device_name 设备名称
 * @param access_key 产品密钥
 * @param task_type 设备任务类型，1：fota；2：sota
 * @param firmware_version 模组固件版本号，可为NULL，但不可与software_version同时为NULL
 * @param software_version 应用固件版本号，可为NULL，但不可与firmware_version同时为NULL
 * @param recv_max_len 用于接收tcp报文的buffer总长度
 * @param check_period_s 设置为0时，调用ota_loop不会进行周期性检查任务；设置为大于0的数值，将以该数值做周期性检测
 * @param timeout_ms 初始化超时时间，单位：毫秒
 *
 * @return uint8_t 初始化结果
 */
uint8_t ota_init(void **ota_ctx, const char *product_id, const char *device_name, const char *access_key,
                 uint8_t task_type, const char *firmware_version, const char *software_version, uint32_t recv_max_len,
                 uint16_t check_period_s, uint32_t timeout_ms);
uint8_t ota_deinit(ota_context *ota_ctx);

uint8_t OTA_Submit_Version(ota_context *ota_ctx, uint32_t timeout_ms);
uint8_t OTA_Check_Task(ota_context *ota_ctx, uint32_t timeout_ms);
uint8_t OTA_Validate_TID(ota_context *ota_ctx, uint32_t timeout_ms);
/**
 * @brief 下载文件
 *
 * @param ota_ctx ota上下文
 * @param segment_start 本次下载分片包起始位置
 * @param segment_end 本次下载分片包终止位置
 * @param timeout_ms 下载文件超时时间
 *
 * @return uint8_t 初始化结果
 */
uint8_t OTA_Download_Package(ota_context *ota_ctx, uint32_t segment_start, uint32_t segment_end, uint32_t timeout_ms);
uint8_t OTA_Update_Progress(ota_context *ota_ctx, uint32_t timeout_ms);

uint8_t ota_loop(void *ota_ctx, uint32_t timeout_ms);
uint8_t OTA_Event_Handle(void *context, ota_event_code_t EVENT);
#ifdef __cplusplus
}
#endif

#endif //_OTA_API_H_
