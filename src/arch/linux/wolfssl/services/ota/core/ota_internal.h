#ifndef _OTA_INTERNAL_H_
#define _OTA_INTERNAL_H_

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "cJSON.h"
#include "http_parser.h"
#include "ota_sys.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition (Constant and Macro)                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/

uint8_t otaPack_Submit_Version(ota_context *ctx, uint16_t buffer_len);
uint8_t otaPack_Check_Task(ota_context *ctx, uint16_t buffer_len);
uint8_t otaPack_Validate_TID(ota_context *ctx, uint16_t buffer_len);
uint8_t otaPack_Download_Package(ota_context *ctx, uint16_t buffer_len);
uint8_t otaPack_Update_Progress(ota_context *ctx, uint16_t buffer_len);

uint8_t otaParse_Submit_Version(ota_context *ctx);
uint8_t otaParse_Check_Task(ota_context *ctx);
uint8_t otaParse_Validate_TID(ota_context *ctx);
uint8_t otaParse_Download_Package(ota_context *ctx);
uint8_t otaParse_Report_Progress(ota_context *ctx);

#ifdef __cplusplus
}
#endif

#endif //_OTA_INTERNAL_H_
