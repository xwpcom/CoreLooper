#ifndef _OTA_SYS_H_
#define _OTA_SYS_H_

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include "data_types.h"
#include "plat_osl.h"
#include "plat_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition (Constant and Macro)                                 */
/*****************************************************************************/

#define OTA_OK (1)
#define OTA_ERROR (0)

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
typedef handle_t ota_handle_t;

#define ota_malloc osl_malloc
#define ota_free osl_free
#define ota_memset osl_memset
#define ota_memcpy osl_memcpy
#define ota_strcat osl_strcat
#define ota_strcpy osl_strcpy
#define ota_strstr osl_strstr
#define ota_strlen osl_strlen
#define ota_sprintf osl_sprintf
#define ota_get_tick_ms time_count_ms
/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/

int ota_log_printf(const char *format, ...);
int ota_atoi(const char *nptr);
void ota_hex2str(const char *src_hex, char *Dst_str, int hex_len);

void ota_deinit_free(void *p);

uint8_t ota_scene_recall(void *user_context);
uint8_t ota_scene_store(void *user_context);

/**
 * @brief 计算下次获取升级包的范围
 *
 * @param package_now 目前已经下载的升级包的范围
 * @param start 用于保存下次获取的范围的起始位置
 * @param end 用于保存下次获取的范围的终止位置
 */
void ota_calculate_segment_range(uint32_t package_now, uint32_t *start, uint32_t *end);

#ifdef __cplusplus
}
#endif

#endif //_OTA_SYS_H_
