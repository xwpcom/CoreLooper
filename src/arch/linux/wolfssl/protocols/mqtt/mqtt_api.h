/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file mqtt_api.h
 * @brief MQTT协议接口封装。
 */

#ifndef __MQTT_API_H__
#define __MQTT_API_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition（Constant and Macro )                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
/**
 * @brief MQTT QOS等级。
 *
 */
enum mqtt_qos_e
{
    MQTT_QOS0 = 0,  /**<QOS0，最多发送一次 */
    MQTT_QOS1,      /**<QOS1，最少发送一次 */
    MQTT_QOS2       /**<QOS2，只被接收到一次 */
};

/**
 * @brief MQTT登录标志。
 *
 */
enum mqtt_connect_flag_e
{
    /** 连接断开后清除会话状态*/
    MQTT_CONNECT_FLAG_CLEAN_SESSION = 0x01,
    /** 连接时设定了遗嘱消息*/
    MQTT_CONNECT_FLAG_WILL = 0x02,
    /** 设定遗嘱消息是否为保留消息，当MQTT_CONNECT_FLAG_WILL被设定时有效*/
    MQTT_CONNECT_FLAG_WILL_RETAIN = 0x04,
    /** 连接时提供用户名信息。当未设定该标志时，MQTT_CONNECT_FLAG_PASSWORD无效*/
    MQTT_CONNECT_FLAG_USERNAME = 0x08,
    /** 连接时提供用户密码，等MQTT_CONNECT_FLAG_USERNAME被设定时有效*/
    MQTT_CONNECT_FLAG_PASSWORD = 0x10
};

/**
 * @brief MQTT遗嘱消息。
 *
 */
struct mqtt_will_message_t
{
    /** 遗嘱消息的topic*/
    const int8_t *will_topic;
    /** 遗嘱消息内容*/
    uint8_t *will_message;
    /** 遗嘱消息长度*/
    uint32_t will_message_len;
    /** 遗嘱消息的QOS等级*/
    enum mqtt_qos_e qos;
};

/**
 * @brief MQTT参数定义。
 *
 */
struct mqtt_param_t
{
    /** 登录服务器用的clientID*/
    const int8_t *client_id;
    /** 登录服务器用的username*/
    const int8_t *username;
    /** 登录服务器用的password*/
    const int8_t *password;

    /** 心跳保活时间间隔，单位秒*/
    uint16_t life_time;
    /** 登录标志，可选项参考mqtt_connect_flag_e，支持同时设定多个标志（或运算）*/
    uint32_t connect_flag;
    /** 遗嘱消息设置，当connect_flag包含MQTT_CONNECT_FLAG_WILL时有效*/
    struct mqtt_will_message_t will_msg;

    /** 数据发送缓冲区*/
    uint8_t *send_buf;
    /** 数据发送缓冲区长度*/
    uint32_t send_buf_len;
    /** 数据接收缓冲区*/
    uint8_t *recv_buf;
    /** 数据接收缓冲区长度*/
    uint32_t recv_buf_len;
};

/**
 * @brief MQTT消息定义。
 *
 */
struct mqtt_message_t
{
    /** 消息QOS等级*/
    enum mqtt_qos_e qos;
    /** 消息内容*/
    uint8_t *payload;
    /** 消息内容长度*/
    uint32_t payload_len;
    /** 消息重发标志*/
    uint8_t dup;
    /** 消息是否为服务器保留消息*/
    uint8_t retained;
    /** 消息ID*/
    uint16_t id;
};

/**
 * @brief MQTT下发消息回调
 *
 */
typedef void (*mqtt_message_handler)(void * /*arg*/, const int8_t * /*topic*/,
                                     struct mqtt_message_t * /*message*/);

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 登录MQTT服务器
 * 
 * @param remote_addr 服务器地址，点分十进制形式
 * @param remote_port 服务器端口
 * @param ca_cert CA证书，为空表示使用非加密连接
 * @param ca_cert_len CA证书数据长度
 * @param mqtt_param MQTT服务器登录设置
 * @param timeout_ms 超时时间
 * @return void* 
 */
void *mqtt_connect(const int8_t *remote_addr, uint16_t remote_port, const int8_t *ca_cert, uint16_t ca_cert_len,
                   struct mqtt_param_t *mqtt_param, uint32_t timeout_ms);

/**
 * @brief MQTT消息处理。
 *
 * @param client MQTT客户端实例操作句柄
 * @param timeout 超时时间
 * @return int32_t
 */
int32_t mqtt_yield(void *client, uint32_t timeout_ms);

/**
 * @brief MQTT消息推送。
 *
 * @param client MQTT客户端实例操作句柄
 * @param topic 推送消息的目标topic
 * @param message 需要推送的消息内容
 * @return int32_t 根据QOS级别返回PUBACK或其它错误
 */
int32_t mqtt_publish(void *client, const int8_t *topic,
                     struct mqtt_message_t *message, uint32_t timeout_ms);

/**
 * @brief 订阅指定topic。
 *
 * @param client MQTT客户端实例操作句柄
 * @param topic 需要订阅的目标topic
 * @param qos 指定订阅的目标topic QoS等级
 * @param msg_handler 指定topic的消息处理回调
 * @param arg 设置消息处理回调参数
 * @return int32_t
 */
int32_t mqtt_subscribe(void *client, const int8_t *topic, enum mqtt_qos_e qos,
                       mqtt_message_handler msg_handler, void *arg, uint32_t timeout_ms);

/**
 * @brief 取消topic订阅。
 *
 * @param client MQTT客户端实例操作句柄
 * @param topic 需要取消订阅的topic
 * @return int32_t
 */
int32_t mqtt_unsubscribe(void *client, const int8_t *topic, uint32_t timeout_ms);

/**
 * @brief 断开MQTT连接。
 *
 * @param client MQTT客户端实例操作句柄
 * @return int32_t
 */
int32_t mqtt_disconnect(void *client, uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif
