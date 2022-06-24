/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_mqtt.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "plat_osl.h"
#include "plat_time.h"
#include "dev_token.h"
#include "mqtt_api.h"
#include "tm_mqtt.h"
#include "tm_api.h"
#include "config.h"
#include "log.h"

#include <stdlib.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_NONE
#define THING_MODEL_SERVER_ADDR "183.230.102.116"
#define THING_MODEL_SERVER_PORT 1883
#else
//#define THING_MODEL_SERVER_ADDR_TLS "183.230.102.116" //onenet自带
#define THING_MODEL_SERVER_ADDR_TLS "210.76.75.21" //gd2022正式
#define THING_MODEL_SERVER_PORT_TLS 8883 
#endif

#define THING_MODEL_SUBED_TOPIC "$sys/%s/%s/thing/#"

#ifdef FEATURE_TM_OTA_ENABLED
#define THING_MODEL_SUBED_OTA_TOPIC "$sys/%s/%s/ota/#"
#endif
/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
struct tm_mqtt_obj_t
{
    void *client;
    struct mqtt_param_t mqtt_param;
    int8_t *subed_topic;
    int32_t (*recv_cb)(const int8_t * /** data_name*/, uint8_t * /** data*/, uint32_t /** data_len*/);
#ifdef FEATURE_TM_OTA_ENABLED
    int8_t *subed_ota_topic;
#endif
};
/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
struct tm_mqtt_obj_t *tm_obj = NULL;

#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_WOLFSSL
static const int8_t tm_cert[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDOzCCAiOgAwIBAgIJAPCCNfxANtVEMA0GCSqGSIb3DQEBCwUAMDQxCzAJBgNV\r\n"
    "BAYTAkNOMQ4wDAYDVQQKDAVDTUlPVDEVMBMGA1UEAwwMT25lTkVUIE1RVFRTMB4X\r\n"
    "DTE5MDUyOTAxMDkyOFoXDTQ5MDUyMTAxMDkyOFowNDELMAkGA1UEBhMCQ04xDjAM\r\n"
    "BgNVBAoMBUNNSU9UMRUwEwYDVQQDDAxPbmVORVQgTVFUVFMwggEiMA0GCSqGSIb3\r\n"
    "DQEBAQUAA4IBDwAwggEKAoIBAQC/VvJ6lGWfy9PKdXKBdzY83OERB35AJhu+9jkx\r\n"
    "5d4SOtZScTe93Xw9TSVRKrFwu5muGgPusyAlbQnFlZoTJBZY/745MG6aeli6plpR\r\n"
    "r93G6qVN5VLoXAkvqKslLZlj6wXy70/e0GC0oMFzqSP0AY74icANk8dUFB2Q8usS\r\n"
    "UseRafNBcYfqACzF/Wa+Fu/upBGwtl7wDLYZdCm3KNjZZZstvVB5DWGnqNX9HkTl\r\n"
    "U9NBMS/7yph3XYU3mJqUZxryb8pHLVHazarNRppx1aoNroi+5/t3Fx/gEa6a5PoP\r\n"
    "ouH35DbykmzvVE67GUGpAfZZtEFE1e0E/6IB84PE00llvy3pAgMBAAGjUDBOMB0G\r\n"
    "A1UdDgQWBBTTi/q1F2iabqlS7yEoX1rbOsz5GDAfBgNVHSMEGDAWgBTTi/q1F2ia\r\n"
    "bqlS7yEoX1rbOsz5GDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQAL\r\n"
    "aqJ2FgcKLBBHJ8VeNSuGV2cxVYH1JIaHnzL6SlE5q7MYVg+Ofbs2PRlTiWGMazC7\r\n"
    "q5RKVj9zj0z/8i3ScWrWXFmyp85ZHfuo/DeK6HcbEXJEOfPDvyMPuhVBTzuBIRJb\r\n"
    "41M27NdIVCdxP6562n6Vp0gbE8kN10q+ksw8YBoLFP0D1da7D5WnSV+nwEIP+F4a\r\n"
    "3ZX80bNt6tRj9XY0gM68mI60WXrF/qYL+NUz+D3Lw9bgDSXxpSN8JGYBR85BxBvR\r\n"
    "NNAhsJJ3yoAvbPUQ4m8J/CoVKKgcWymS1pvEHmF47pgzbbjm5bdthlIx+swdiGFa\r\n"
    "WzdhzTYwVkxBaU+xf/2w\r\n"
    "-----END CERTIFICATE-----"
 };
#endif

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
static void tm_mqtt_message_arrived(void *arg, const int8_t *topic, struct mqtt_message_t *message)
{
    tm_obj->recv_cb(topic, message->payload, message->payload_len);
}

static int8_t *tm_topic_construct(const int8_t *format, const int8_t *pid, const int8_t *dev_name)
{
    uint32_t topic_len = 0;
    int8_t *topic = NULL;

    topic_len = osl_strlen(format) + osl_strlen(pid) + osl_strlen(dev_name) - 3;

    if (NULL != (topic = osl_malloc(topic_len)))
    {
        osl_memset(topic, 0, topic_len);
        osl_sprintf(topic, format, pid, dev_name);
    }

    return topic;
}

int32_t tm_mqtt_init(int32_t (*recv_cb)(const int8_t * /** data_name*/, uint8_t * /** data*/,
                                        uint32_t /** data_len*/))
{
    if(NULL == (tm_obj = osl_malloc(sizeof(struct tm_mqtt_obj_t))))
    {
        goto exit1;
    }
    osl_memset(tm_obj, 0, sizeof(struct tm_mqtt_obj_t));

    tm_obj->mqtt_param.send_buf_len = FEATURE_TM_SEND_BUF_LEN;
    if(NULL == (tm_obj->mqtt_param.send_buf = osl_malloc(tm_obj->mqtt_param.send_buf_len)))
    {
        goto exit2;
    }
    osl_memset(tm_obj->mqtt_param.send_buf, 0, tm_obj->mqtt_param.send_buf_len);

    tm_obj->mqtt_param.recv_buf_len = FEATURE_TM_RECV_BUF_LEN;
    if(NULL == (tm_obj->mqtt_param.recv_buf = osl_malloc(tm_obj->mqtt_param.recv_buf_len)))
    {
        goto exit3;
    }
    osl_memset(tm_obj->mqtt_param.recv_buf, 0, tm_obj->mqtt_param.recv_buf_len);

    tm_obj->recv_cb = recv_cb;

    return 0;

exit3:
    osl_free(tm_obj->mqtt_param.send_buf);
exit2:
    osl_free(tm_obj);
    tm_obj = NULL;
exit1:
    return -1;
}

int32_t tm_mqtt_login(const int8_t *product_id, const int8_t *dev_name, const int8_t *dev_token, uint32_t life_time,
                      uint32_t timeout_ms)
{
    handle_t cd_hdl = 0;

    if (0 == (cd_hdl = countdown_start(timeout_ms)))
    {
        return -1;
    }

    tm_obj->mqtt_param.client_id = dev_name;
    tm_obj->mqtt_param.username = product_id;
    tm_obj->mqtt_param.password = dev_token;

    tm_obj->mqtt_param.life_time = life_time;
    tm_obj->mqtt_param.connect_flag =
        MQTT_CONNECT_FLAG_CLEAN_SESSION | MQTT_CONNECT_FLAG_USERNAME | MQTT_CONNECT_FLAG_PASSWORD;

    tm_obj->subed_topic = tm_topic_construct(THING_MODEL_SUBED_TOPIC, product_id, dev_name);
#ifdef FEATURE_TM_OTA_ENABLED
    tm_obj->subed_ota_topic = tm_topic_construct(THING_MODEL_SUBED_OTA_TOPIC, product_id, dev_name);
#endif
#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_NONE
    if(NULL == (tm_obj->client = mqtt_connect(THING_MODEL_SERVER_ADDR, THING_MODEL_SERVER_PORT, NULL, 0, &(tm_obj->mqtt_param), countdown_left(cd_hdl))))
#else
    if(NULL == (tm_obj->client = mqtt_connect(THING_MODEL_SERVER_ADDR_TLS, THING_MODEL_SERVER_PORT_TLS, tm_cert, osl_strlen(tm_cert), &(tm_obj->mqtt_param), countdown_left(cd_hdl))))
#endif
    {
        goto exit;
    }

    if (0 != (mqtt_subscribe(tm_obj->client, tm_obj->subed_topic, MQTT_QOS0,
                              tm_mqtt_message_arrived, NULL, countdown_left(cd_hdl))))
    {
        goto exit;
    }
#ifdef FEATURE_TM_OTA_ENABLED
    if(0 != (mqtt_subscribe(tm_obj->client, tm_obj->subed_ota_topic, MQTT_QOS0, tm_mqtt_message_arrived, NULL,
                            countdown_left(cd_hdl))))
    {
        goto exit;
    }
#endif
    countdown_stop(cd_hdl);
    return 0;

exit:
    countdown_stop(cd_hdl);
    tm_mqtt_logout(timeout_ms);
    return -1;
}

int32_t tm_mqtt_logout(uint32_t timeout_ms)
{
    if (tm_obj->client)
    {
        mqtt_disconnect(tm_obj->client, timeout_ms);
    }
    if(tm_obj->subed_topic)
    {
        osl_free(tm_obj->subed_topic);
    }
#ifdef FEATURE_TM_OTA_ENABLED
    if(tm_obj->subed_ota_topic)
    {
        osl_free(tm_obj->subed_ota_topic);
    }
#endif
    if(tm_obj->mqtt_param.send_buf)
    {
        osl_free(tm_obj->mqtt_param.send_buf);
    }
    if(tm_obj->mqtt_param.recv_buf)
    {
        osl_free(tm_obj->mqtt_param.recv_buf);
    }
    if(tm_obj)
    {
        osl_free(tm_obj);
        tm_obj = NULL;
    }

    return 0;
}

int32_t tm_mqtt_send_packet(const int8_t *topic, uint8_t *payload, uint32_t payload_len,
                                   uint32_t timeout_ms)
{
    struct mqtt_message_t msg;

    osl_memset(&msg, 0, sizeof(struct mqtt_message_t));
    msg.qos = MQTT_QOS0;
    msg.payload = payload;
    msg.payload_len = payload_len;

    return mqtt_publish(tm_obj->client, topic, &msg, timeout_ms);
}

int32_t tm_mqtt_step(uint32_t timeout_ms)
{
    return mqtt_yield(tm_obj->client, timeout_ms);
}
