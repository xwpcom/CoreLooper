/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 * 
 * @file config.h
 * @brief 
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                 */
/*****************************************************************************/
#define PLAT_HAS_STDINT

#define FEATURE_TM_PROTOCOL_MQTT 0
#define FEATURE_TM_PROTOCOL_COAP 1

#define FEATURE_TM_PROTOCOL FEATURE_TM_PROTOCOL_MQTT
// #define FEATURE_TM_PROTOCOL FEATURE_TM_PROTOCOL_COAP

#define FEATURE_TM_VERSION "1.0"
#define FEATURE_TM_LIFE_TIME 30 //in second
#define FEATURE_TM_REPLY_TIMEOUT 1000 //ms
#define FEATURE_TM_SEND_BUF_LEN 1024
#define FEATURE_TM_RECV_BUF_LEN 1024
#define FEATURE_TM_PAYLOAD_BUF_LEN 1024

#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT

#define FEATURE_TM_MQTT_TLS_NONE 0
#define FEATURE_TM_MQTT_TLS_WOLFSSL 1

#define FEATURE_TM_MQTT_TLS_TYPE FEATURE_TM_MQTT_TLS_WOLFSSL
// #define FEATURE_TM_MQTT_TLS_TYPE FEATURE_TM_MQTT_TLS_NONE

#define FEATURE_TM_GATEWAY_ENABLED

#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP

#endif

//#define FEATURE_TM_OTA_ENABLED 1

#define FEATURE_TM_FMS  1
/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
