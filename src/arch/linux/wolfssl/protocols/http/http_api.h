/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 * 
 * @file http_api.h
 * @brief 
 */

#ifndef __HTTP_API_H__
#define __HTTP_API_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
enum http_method_e
{
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE
};

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 创建一个新的HTTP请求
 * 
 * @param method 指定请求类型
 * @param host 主机地址，可设置为域名或[ip:port]方式
 * @param abs_path 需要访问的资源路径，以"/"开头，可为空
 * @param size HTTP封包大小
 * @return NULL - 创建失败
 *       非NULL - HTTP上下文
 */
void *http_new(enum http_method_e method, const int8_t *host, const int8_t *abs_path, uint32_t size);
void http_delete(void *ctx);

/**
 * @brief 设置请求参数
 * 
 * @param ctx 创建的HTTP上下文
 * @param name 需要添加的请求参数名
 * @param value 需要添加的请求参数值
 * @return  0 - 成功
 */
int32_t http_add_param(void *ctx, const int8_t *name, const int8_t *value);

/**
 * @brief 设置HTTP头域
 * 
 * @param ctx 创建的HTTP上下文
 * @param name 需要添加的头域名
 * @param value 需要添加的头域值
 * @return  0 - 成功
 */
int32_t http_add_header(void *ctx, const int8_t *name, const int8_t *value);

/**
 * @brief 添加需要发送的HTTP请求数据，可分段调用添加（根据调用顺序进行拼接，可选）
 * 
 * @param ctx 创建的HTTP上下文
 * @param body 需要上报的HTTP请求数据
 * @param body_len 需要上报的HTTP请求数据长度
 * @return  0 - 成功
 */
int32_t http_add_body(void *ctx, uint8_t *body, uint32_t body_len);

/**
 * @brief 发送已设置的HTTP请求，并返回响应数据
 * 
 * @param ctx 创建并设置完参数的HTTP上下文
 * @param server 指定HTTP服务器地址，为空时使用host参数
 * @param server_port 指定HTTP服务器端口，为空时使用host参数（host为域名形式时默认80）
 * @param resp_body 返回响应数据的数据缓冲区地址
 * @param resp_body_len 返回响应数据的数据长度
 * @param timeout_ms 请求超时时间
 * @return  <0 - 发送/接收失败
 *          >0 - HTTP响应码
 */
int32_t http_send_request(void *ctx, const int8_t *server, uint16_t server_port, uint8_t **resp_body,
                          uint32_t *resp_body_len, uint32_t timeout_ms);

/**
 * @brief 调用http_send_request成功后，可调用接口获取响应内容的头域
 * 
 * @param ctx HTTP上下文
 * @param name 响应头域名
 * @param value 返回响应头域值
 * @return  
 */
int32_t http_get_resp_header(void *ctx, const int8_t *name, const int8_t **value);

int32_t http_get_resp_status(void *ctx);

#ifdef __cplusplus
}
#endif

#endif
