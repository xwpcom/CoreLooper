/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file plat_udp.h
 * @brief
 */

#ifndef __PLAT_UDP_H__
#define __PLAT_UDP_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "slist.h"
#include "plat_osl.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition (Constant and Macro)                                  */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
typedef struct udp_packet_list_node
{
    struct slist_node node;
    uint8_t *buffer;
    uint32_t length;
} udp_packet_list_node_t;

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 创建UDP连接。指定host和port后，可直接调用udp_send和udp_recv进行收发。
 *
 * @param host 网络数据目标地址
 * @param port 网络数据目标端口
 * @retval -1 - 操作错误
 * @retval 其它 - 网络操作句柄
 */
handle_t udp_connect(const char *host, uint16_t port);

/**
 * @brief 发送UDP数据包。
 *
 * @param handle 网络操作句柄
 * @param buf 需要发送的数据缓冲区地址
 * @param len 需要发送的数据长度
 * @param timeout_ms 数据发送超时时间
 * @retval -1 - 错误
 * @retval  0 - 超时
 * @retval 其它 - 成功发送的数据长度
 */
int32_t udp_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief 接收UDP网络数据
 *
 * @param handle 网络操作句柄
 * @param buf 用于接收数据的缓冲区地址
 * @param len 需要接收的数据长度
 * @param timeout_ms 操作超时时间
 * @retval -1 - 错误
 * @retval  0 - 超时
 * @retval 其它 - 实际接收的数据长度
 */
int32_t udp_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief 创建UDPsocket
 *
 * @param local_host 指定本地使用的主机地址，可为空
 * @param local_port 指定本地使用的端口
 * @retval -1 - 错误
 * @retval 其它 - 网络操作句柄
 */
handle_t udp_bind(const char *local_host, uint16_t local_port);

/**
 * @brief 向指定地址发送UDP数据
 *
 * @param handle 网络操作句柄
 * @param buf 需要发送的数据缓冲区地址
 * @param len 需要发送的数据长度
 * @param host 指定目标主机地址
 * @param port 指定目标端口
 * @param timeout_ms 数据发送超时时间
 * @retval -1 - 错误
 * @retval  0 - 超时
 * @retval 其它 - 实际发送的数据长度
 */
int32_t udp_sendto(handle_t handle, void *buf, uint32_t len, const char *host, uint16_t port, uint32_t timeout_ms);

/**
 * @brief 从指定地址接收UDP数据
 *
 * @param handle 网络操作句柄
 * @param buf 用于接收数据的缓冲区地址
 * @param len 需要接收的数据长度
 * @param host 指定接收数据的源地址
 * @param port 指定接收数据的源端口
 * @param timeout_ms 操作超时时间
 * @retval -1 - 错误
 * @retval  0 - 超时
 * @retval 其它 -  实际接收的数据长度
 */
int32_t udp_recvfrom(handle_t handle, void *buf, uint32_t len, const char *host, uint16_t port, uint32_t timeout_ms);

/**
 * @brief 关闭指定连接
 *
 * @param handle 网络操作句柄
 * @return int32_t
 */
int32_t udp_disconnect(handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
