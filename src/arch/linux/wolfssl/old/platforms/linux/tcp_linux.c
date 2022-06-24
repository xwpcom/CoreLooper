/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tcp_linux.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "plat_time.h"
#include "plat_tcp.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
static int32_t check_connect(int32_t fd, uint32_t timeout_ms)
{
    struct timeval tv;
    fd_set fs;
    socklen_t len = 0;
    int32_t ret = 0;
    int32_t sock_error = 0;

    tv.tv_sec = timeout_ms/1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    FD_ZERO(&fs);
    FD_SET(fd, &fs);
    ret = select(fd + 1, NULL, &fs, NULL, &tv);
    if (0 < ret)
    {
        if (FD_ISSET(fd, &fs))
        {
            len = sizeof(sock_error);
            if ((0 ==  getsockopt(fd, SOL_SOCKET, SO_ERROR, &sock_error, &len)) &&
                    (0 == sock_error))
                return 1;
        }
    }

    return 0;
}

handle_t tcp_connect(const int8_t *host, uint16_t port, uint32_t timeout_ms)
{
    handle_t fd = -1;
    struct sockaddr_in addr_in;
    struct hostent *ip_addr;
    int32_t flags = -1;
    int ret = -1;
    printf("tcp_connect(%s:%d)\r\n", host, port);
    if(0 == (ip_addr = gethostbyname((const char *)host)))
    {
        goto exit;
    }

    addr_in.sin_family = AF_INET;
    addr_in.sin_addr = *((struct in_addr *)ip_addr->h_addr_list[0]);
    addr_in.sin_port = htons(port);

    if(0 > (fd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        goto exit;
    }

    if((0 > (flags = fcntl(fd, F_GETFL, 0))) ||
            (0 > fcntl(fd, F_SETFL, flags | O_NONBLOCK)))
    {
        goto close_socket;
    }

    if (0 > (ret = connect(fd, (struct sockaddr *)&addr_in, sizeof(addr_in))))
    {
        if(EINPROGRESS != errno)
        {
            goto close_socket;
        }
    }
    if(check_connect(fd, timeout_ms))
        return fd;

close_socket:
    close(fd);
exit:
    return -1;
}

int32_t tcp_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms)
{
    handle_t countdown_tmr = 0;
    struct timeval tv;
    fd_set fs;
    int sent_len = 0;
    int ret = 0;
    uint32_t left_time = 0;

    if((0 > handle) || (0 == (countdown_tmr = countdown_start(timeout_ms))))
        return -1;

    do
    {
        left_time = countdown_left(countdown_tmr);
        tv.tv_sec = left_time / 1000;
        tv.tv_usec = (left_time % 1000) * 1000;

        FD_ZERO(&fs);
        FD_SET(handle, &fs);

        ret = select(handle + 1, NULL, &fs, NULL, &tv);
        if(0 < ret)
        {
            if(FD_ISSET(handle, &fs))
            {
                ret = send(handle, buf + sent_len, len - sent_len, MSG_DONTWAIT);
                if(0 < ret)
                {
                    sent_len += ret;
                }
                else if(0 > ret)
                {
                    sent_len = ret;
                    break;
                }
            }
        }
        else if(0 > ret)
        {
            sent_len = ret;
            break;
        }
    }
    while ((sent_len < len) && (0 == countdown_is_expired(countdown_tmr)));
    countdown_stop(countdown_tmr);

    return sent_len;
}

int32_t tcp_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms)
{
    handle_t countdown_tmr = 0;
    struct timeval tv;
    fd_set fs;
    int recv_len = 0;
    int ret = 0;
    uint32_t left_time = 0;

    if((0 > handle) || (0 == (countdown_tmr = countdown_start(timeout_ms))))
        return -1;
    
    do
    {
        left_time = countdown_left(countdown_tmr);
        tv.tv_sec = left_time / 1000;
        tv.tv_usec = (left_time % 1000) * 1000;

        FD_ZERO(&fs);
        FD_SET(handle, &fs);

        ret = select(handle + 1, &fs, NULL, NULL, &tv);
        if(0 < ret)
        {
            if(FD_ISSET(handle, &fs))
            {
                ret = recv(handle, buf + recv_len, len - recv_len, MSG_DONTWAIT);
                if(0 < ret)
                {
                    recv_len += ret;
                }
                else if(0 > ret)
                {
                    recv_len = ret;
                    break;
                }
            }
        }
        else if(0 > ret)
        {
            recv_len = ret;
            break;
        }
    }
    while ((recv_len < len) && (0 == countdown_is_expired(countdown_tmr)));
    countdown_stop(countdown_tmr);

    return recv_len;
}

int32_t tcp_disconnect(handle_t handle)
{
    if(0 < handle)
    {
        close(handle);
    }
        
    return 0;
}
