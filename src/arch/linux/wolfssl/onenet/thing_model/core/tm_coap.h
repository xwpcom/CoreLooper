/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_coap.h
 * @brief
 */

#ifndef __TM_COAP_H__
#define __TM_COAP_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "er-coap-13.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
int32_t tm_coap_init(int32_t (*recv_cb)(const int8_t * /** data_name*/, uint8_t * /** data*/,
                                        uint32_t /** data_len*/));
int32_t tm_coap_login(const int8_t *product_id, const int8_t *dev_name, const int8_t *dev_token,
                      uint32_t life_time, uint32_t timeout_ms);
int32_t tm_coap_logout(uint32_t timeout_ms);
int32_t tm_coap_step(uint32_t timeout_ms);
int32_t tm_coap_send_packet(const int8_t *uri, uint8_t *payload, uint32_t payload_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
