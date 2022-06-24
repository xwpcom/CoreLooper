/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file plat_tcp.h
 * @brief
 */

#ifndef __PLAT_TCP_H__
#define __PLAT_TCP_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

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
/**
 * @brief 建立TCP连接
 *
 * @param host TCP连接目标主机地址
 * @param port TCP连接目标主机端口
 * @param timeout_ms 建立连接超时时间
 * @retval -1 - 失败
 * @retval 其它 - TCP连接操作句柄
 */
handle_t tcp_connect(const int8_t *host, uint16_t port, uint32_t timeout_ms);

/**
 * @brief 通过TCP连接发送数据
 *
 * @param handle TCP连接操作句柄
 * @param buf 需要发送的数据缓冲区地址
 * @param len 需要发送的数据长度
 * @param timeout_ms 数据发送超时时间
 * @retval -1 - 错误
 * @retval  0 - 超时
 * @retval 其它 - 实际成功发送的数据长度
 */
int32_t tcp_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief 通过TCP连接接收数据
 *
 * @param handle TCP连接操作句柄
 * @param buf 用于接收数据的缓冲区地址
 * @param len 希望接收到的数据长度
 * @param timeout_ms 超时时间
 * @retval -1 - 错误
 * @retval  0 - 超时
 * @retval 其它 - 实际成功接收到的数据长度
 */
int32_t tcp_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief 断开指定TCP连接
 *
 * @param handle TCP连接操作句柄
 * @retval 0 - 成功
 */
int32_t tcp_disconnect(handle_t handle);


#ifdef __cplusplus
}
#endif

#endif
