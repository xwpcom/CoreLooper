/**
 * Copyright (c), 2012~2019 iot.10086.cn All Rights Reserved
 * 
 * @file mqtt_client.h
 * @brief 本文件来源于PahoMQTT MQTTClient-C\src\MQTTClient.h
 */


#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "MQTTPacket.h"
#include "mqtt_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition（Constant and Macro )                                 */
/*****************************************************************************/
#define MAX_MESSAGE_HANDLERS 5

#define DefaultClient {0, 0, 0, 0, NULL, NULL, 0, 0, 0}

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
enum mqtt_ret_code { BUFFER_OVERFLOW = -2, FAILURE = -1, SUCCESS = 0 };
#if 0
typedef struct mqtt_message_data
{
    struct mqtt_message_t* message;
    int8_t* topicName;
} mqtt_message_data;
#endif
typedef struct mqtt_conn_ack_data
{
    uint8_t rc;
    uint8_t sessionPresent;
} mqtt_conn_ack_data;

typedef struct mqtt_sub_ack_data
{
    enum mqtt_qos_e grantedQoS;
} mqtt_sub_ack_data;

typedef void (*messageHandler)(void *, const int8_t *, struct mqtt_message_t *);

typedef int32_t (*net_write_callback)(handle_t, void *, uint32_t, uint32_t);
typedef int32_t (*net_read_callback)(handle_t, void *, uint32_t, uint32_t);
typedef int32_t (*net_disconnect_callback)(handle_t);

typedef struct mqtt_network
{
    handle_t handle;
    net_read_callback mqttread;
    net_write_callback mqttwrite;
    net_disconnect_callback disconnect;
} mqtt_network;

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
#if 1
/**
 * Create an MQTT client object
 * @param client
 * @param network
 * @param command_timeout_ms
 * @param
 */
void* mqtt_client_init(mqtt_network* network, uint8_t* sendbuf, size_t sendbuf_size,
                       uint8_t* readbuf, size_t readbuf_size);

void mqtt_client_deinit(void *client);

/** MQTT Connect - send an MQTT connect packet down the network and wait for a Connack
 *  The nework object must be connected to the network endpoint32_t before calling this
 *  @param options - connect options
 *  @return success code
 */
int32_t mqtt_client_connect_with_results(void* client, MQTTPacket_connectData* options,
                                         mqtt_conn_ack_data* data, uint32_t timeout_ms);

/** MQTT Connect - send an MQTT connect packet down the network and wait for a Connack
 *  The nework object must be connected to the network endpoint32_t before calling this
 *  @param options - connect options
 *  @return success code
 */
int32_t mqtt_client_connect(void* client, MQTTPacket_connectData* options, uint32_t timeout_ms);

/** MQTT Publish - send an MQTT publish packet and wait for all acks to complete for all QoSs
 *  @param client - the client object to use
 *  @param topic - the topic to publish to
 *  @param message - the message to send
 *  @return success code
 */
int32_t mqtt_client_publish(void *client, const char *topicName, struct mqtt_message_t *message, uint32_t timeout_ms);

/** MQTT SetMessageHandler - set or remove a per topic message handler
 *  @param client - the client object to use
 *  @param topicFilter - the topic filter set the message handler for
 *  @param messageHandler - pointer to the message handler function or NULL to remove
 *  @return success code
 */
int32_t mqtt_client_set_message_handler(void *client, const char *topicFilter, messageHandler messageHandler, void *arg);

/** MQTT Subscribe - send an MQTT subscribe packet and wait for suback before returning.
 *  @param client - the client object to use
 *  @param topicFilter - the topic filter to subscribe to
 *  @param message - the message to send
 *  @return success code
 */
int32_t mqtt_client_subscribe(void *c, const char *topicFilter, enum mqtt_qos_e qos, messageHandler messageHandler,
                              void *arg, uint32_t timeout_ms);

/** MQTT Subscribe - send an MQTT subscribe packet and wait for suback before returning.
 *  @param client - the client object to use
 *  @param topicFilter - the topic filter to subscribe to
 *  @param message - the message to send
 *  @param data - suback granted QoS returned
 *  @return success code
 */
int32_t mqtt_client_subscribe_with_results(void *client, const char *topicFilter, enum mqtt_qos_e qos,
                                           messageHandler messageHandler, void *arg, mqtt_sub_ack_data *data,
                                           uint32_t timeout_ms);

/** MQTT Subscribe - send an MQTT unsubscribe packet and wait for unsuback before returning.
 *  @param client - the client object to use
 *  @param topicFilter - the topic filter to unsubscribe from
 *  @return success code
 */
int32_t mqtt_client_unsubscribe(void *client, const char *topicFilter, uint32_t timeout_ms);

/** MQTT Disconnect - send an MQTT disconnect packet and close the connection
 *  @param client - the client object to use
 *  @return success code
 */
int32_t mqtt_client_disconnect(void* client, uint32_t timeout_ms);

/** MQTT Yield - MQTT background
 *  @param client - the client object to use
 *  @param time - the time, in milliseconds, to yield for
 *  @return success code
 */
int32_t mqtt_client_yield(void* client, int32_t timeout_ms);

/** MQTT isConnected
 *  @param client - the client object to use
 *  @return truth value indicating whether the client is connected to the server
 */
int32_t mqtt_client_is_connected(void* client);
#endif

#ifdef __cplusplus
}
#endif

#endif
