/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file network_tls.h
 * @brief
 */

#ifndef __TLS_H__
#define __TLS_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definitionï¼ˆConstant and Macro )                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 创建TLS安全连接。
 * 
 * @param host TLS连接目标地址
 * @param port TLS连接目标端口
 * @param ca_cert 安全证书
 * @param ca_cert_len 证书长度 
 * @param timeout 创建连接的超时时间
 * @retval -1 - 操作失败
 * @retval 其它 - 网络连接句柄
 */
handle_t tls_connect(const char *host, uint16_t port, const int8_t *ca_cert, uint16_t ca_cert_len,
                     uint32_t timeout);

/**
 * @brief 通过TLS安全连接发送数据。
 * 
 * @param handle TLS连接操作句柄
 * @param buf 需要发送的数据缓冲区地址
 * @param len 需要发送的数据长度
 * @param timeout 数据发送超时时间
 * @retval -1 - 错误
 * @retval  0 - 超时
 * @retval 其它 - 实际发送成功的数据长度
 */
int32_t tls_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout);
/**
 * @brief 通过TLS安全连接接收数据。
 * 
 * @param handle TLS连接操作句柄
 * @param buf 用于接收数据的缓冲区地址
 * @param len 需要接收的数据长度
 * @param timeout 数据接收超时时间
 * @retval -1 - 错误
 * @retval  0 - 超时
 * @retval 其它 - 实际接收的数据长度
 */
int32_t tls_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout);

/**
 * @brief 关闭指定TLS安全连接。
 * 
 * @param handle 需要关闭的TLS连接操作句柄
 * @retval 0 - 成功
 */
int32_t tls_disconnect(handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
