/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file udp_linux.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "plat_time.h"
#include "plat_udp.h"
#include "plat_task.h"
#include "log.h"

#include <string.h>
#include <pthread.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
static struct slist_head *udp_packet_list = NULL;
/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/
static void *udp_recv_thread(void *socket_handle);
/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
static int8_t udp_close_flag = 0;
/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
handle_t udp_connect(const char *host, uint16_t port)
{
    int32_t fd = -1;
    int32_t flags = -1;
    struct sockaddr_in addr_in;
    struct hostent *ip_addr = NULL;

    if(NULL == (ip_addr = gethostbyname(host)))
    {
        goto exit;
    }

    if(0 > (fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)))
    {
        goto exit;
    }

    if(0 > (flags = fcntl(fd, F_GETFL, 0)) || 0 > fcntl(fd, F_SETFL, flags | O_NONBLOCK))
    {
        goto exit1;
    }

    osl_memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr = *((struct in_addr *)ip_addr->h_addr_list[0]);
    addr_in.sin_port = htons(port);

    if(0 > connect(fd, (struct sockaddr *)&addr_in, sizeof(addr_in)))
    {
        goto exit1;
    }

    if(udp_packet_list == NULL)
    {
        udp_packet_list = (struct slist_head *)osl_malloc(sizeof(struct slist_head));
        osl_assert(udp_packet_list != NULL);
        slist_init(udp_packet_list);
    }
    osl_task_handle_t udp_recv_handle = 0;
    task_create(&udp_recv_handle, NULL, udp_recv_thread, (void *)((long)fd));

    return (handle_t)fd;

exit1:
    close(fd);
exit:
    return -1;
}

int32_t udp_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms)
{
    handle_t countdown_tmr = 0;
    struct timeval tv;
    fd_set fs;
    uint32_t left_time = 0;
    int32_t sent_len = 0;
    int32_t ret = 0;

    if((0 > handle) || (0 == (countdown_tmr = countdown_start(timeout_ms))))
    {
        return -1;
    }

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
    } while((sent_len < len) && (0 == countdown_is_expired(countdown_tmr)));
    countdown_stop(countdown_tmr);

    log_debug("%d bytes sent\n", sent_len);
    return sent_len;
}

int32_t udp_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout_ms)
{
    handle_t countdown_tmr = 0;

    if((0 > handle) || (0 == (countdown_tmr = countdown_start(timeout_ms))))
    {
        return -1;
    }

    while(0 == countdown_is_expired(countdown_tmr))
    {
        struct slist_node *node = slist_get_head(udp_packet_list);
        if(node == NULL)
        {
            osl_sleep_ms(100);
            continue;
        }
        else
        {
            uint32_t len_temp = 0;
            udp_packet_list_node_t *packet = (udp_packet_list_node_t *)node;

            osl_memset(buf, 0, len);
            len_temp = packet->length > len ? len : packet->length;
            osl_memcpy(buf, packet->buffer, len_temp);

            slist_remove_head(udp_packet_list);
            osl_free(packet->buffer);
            osl_free(packet);

            countdown_stop(countdown_tmr);
            return len_temp;
        }
    }
    countdown_stop(countdown_tmr);
    return 0;
}

int32_t udp_disconnect(handle_t handle)
{
    handle_t countdown_tmr = 0;
    countdown_tmr = countdown_start(5000);

    udp_close_flag = 2;
    while(udp_close_flag != 0)
    {
        osl_sleep_ms(50);
        if(countdown_is_expired(countdown_tmr) == 1)
        {
            break;
        }
    }

    countdown_stop(countdown_tmr);

    while(udp_packet_list->cnt > 0)
    {
        struct slist_node *slist_node_deinit = NULL;
        struct udp_packet_list_node *udp_packet_list_node_deinit = NULL;

        slist_node_deinit = slist_get_head(udp_packet_list);
        slist_remove_head(udp_packet_list);

        udp_packet_list_node_deinit = (struct udp_packet_list_node *)slist_node_deinit;

        if(udp_packet_list_node_deinit->buffer != NULL)
        {
            osl_free(udp_packet_list_node_deinit->buffer);
        }
        osl_free(slist_node_deinit);
    }
    osl_free(udp_packet_list);
    udp_packet_list = NULL;

    if(0 < handle)
    {
        close(handle);
        return 0;
    }

    return 1;
}

static void *udp_recv_thread(void *socket_handle)
{
    fd_set fs;
    uint32_t left_time = 0;
    int recv_len = 0;
    int ret = 0;

    const handle_t sock = (handle_t)socket_handle;

    udp_close_flag = 1;

    pthread_detach(pthread_self());

    while(1)
    {
        if(udp_close_flag == 2)
        {
            udp_close_flag = 0;
            log_debug("close socket recv thread\n");
            return NULL;
        }
        struct timeval tv = { 2, 0 };
        FD_ZERO(&fs);
        FD_SET(sock, &fs);
        ret = select(sock + 1, &fs, NULL, NULL, &tv);
        if(0 < ret)
        {
            uint8_t buffer[1024] = { 0 };
            int numBytes = 0;

            if(FD_ISSET(sock, &fs))
            {
                struct sockaddr_storage addr;
                socklen_t addrLen;

                addrLen = sizeof(addr);

                recv_len = recv(sock, buffer, 1024, MSG_DONTWAIT);
                if(0 < recv_len)
                {
                    log_debug("%d bytes received\n", recv_len);

                    udp_packet_list_node_t *udp_packet =
                        (udp_packet_list_node_t *)osl_malloc(sizeof(udp_packet_list_node_t));

                    if(udp_packet == NULL)
                    {
                        log_error("udp rcv no mem\n");
                        continue;
                    }

                    osl_memset(udp_packet, 0, sizeof(udp_packet_list_node_t));

                    udp_packet->buffer = (uint8_t *)osl_malloc(recv_len + 1);
                    if(udp_packet->buffer == NULL)
                    {
                        log_error("udp rcv malloc error\n");
                    }
                    osl_memset(udp_packet->buffer, 0, recv_len + 1);
                    osl_memcpy(udp_packet->buffer, buffer, recv_len);

                    udp_packet->length = recv_len;

                    slist_insert_tail(udp_packet_list, &(udp_packet->node));
                    log_debug("udp_packet_list cnt %d\n", udp_packet_list->cnt);
                }
                else if(0 > recv_len)
                {
                    log_error("Error in recvfrom(): %d , %s\r\n", errno, strerror(errno));
                }
            }
        }
        else if(0 > ret)
        {
            goto TAG_END;
        }
    }
TAG_END:
    log_error("Error in socket recv thread exit..\n");
    return NULL;
}