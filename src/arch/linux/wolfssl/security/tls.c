/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file network_tls.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tls.h"
#include "plat_osl.h"
#include "plat_tcp.h"
#include "plat_time.h"
#include "config.h"

#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_WOLFSSL
#include "wolfssl/ssl.h"
#endif

#include <stdio.h>
/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
struct tls_t
{
    handle_t handle;
#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_WOLFSSL
    WOLFSSL_CTX *wolf_ctx;
    WOLFSSL *wolf_ssl;
#endif
    uint32_t send_timeout;
    uint32_t recv_timeout;
};
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
#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_WOLFSSL 
static int32_t wolfssl_send(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    struct tls_t *net = (struct tls_t *)ctx;
    int32_t ret = 0;

    if(0 == (ret = tcp_send(net->handle, buf, sz, net->send_timeout)))
    {
        ret = -2;  //WOLFSSL_CBIO_ERR_WANT_WRITE
    }

    return ret;
}

static int32_t wolfssl_recv(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    struct tls_t *net = (struct tls_t *)ctx;
    int32_t ret = 0;

    if (0 == (ret = tcp_recv(net->handle, buf, sz, net->recv_timeout)))
    {
        return -2;  //WOLFSSL_CBIO_ERR_WANT_READ
    }

    return ret;
}

handle_t tls_connect(const char *host, uint16_t port, const int8_t *ca_cert, 
                             uint16_t ca_cert_len, uint32_t timeout)
{
    struct tls_t *net = NULL;
    handle_t tmr = 0;

    if(NULL == (net = osl_malloc(sizeof(*net))))
    {
        return -1;
    }
    osl_memset(net, 0, sizeof(*net));

    if(0 == (tmr = countdown_start(timeout)))
    {
        osl_free(net);
        return -1;
    }

    wolfSSL_Init();

    if (NULL == (net->wolf_ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())))
    {
        goto exit;
    }

    if(SSL_SUCCESS != wolfSSL_CTX_load_verify_buffer(net->wolf_ctx, ca_cert, ca_cert_len, 
                                                     WOLFSSL_FILETYPE_PEM))
    {
        goto exit1;
    }

    wolfSSL_CTX_set_verify(net->wolf_ctx, WOLFSSL_VERIFY_PEER, NULL);

    wolfSSL_SetIOSend(net->wolf_ctx, wolfssl_send);
    wolfSSL_SetIORecv(net->wolf_ctx, wolfssl_recv);
    if (0 > (net->handle = tcp_connect(host, port, countdown_left(tmr))))
    {
        goto exit1;
    }

    if (NULL == (net->wolf_ssl = wolfSSL_new(net->wolf_ctx)))
    {
        goto exit2;
    }

    wolfSSL_set_fd(net->wolf_ssl, net->handle);
    wolfSSL_SetIOWriteCtx(net->wolf_ssl, net);
    wolfSSL_SetIOReadCtx(net->wolf_ssl, net);

    net->send_timeout = net->recv_timeout = countdown_left(tmr);
    while(SSL_SUCCESS != wolfSSL_connect(net->wolf_ssl))
    {
        if(wolfSSL_want_read(net->wolf_ssl))
        {
            // no error, just non-blocking. Carry on. 
            if(0 == countdown_is_expired(tmr))
            {
                net->send_timeout = net->recv_timeout = countdown_left(tmr);
                continue;
            }
            else
                goto exit3;
        }
        goto exit3;
    }
    countdown_stop(tmr);

    return (handle_t)net;

exit3:
    wolfSSL_free(net->wolf_ssl);
exit2:
    tcp_disconnect(net->handle);
exit1:
    wolfSSL_CTX_free(net->wolf_ctx);
exit:
    countdown_stop(tmr);
    osl_free(net);

    return -1;
}

int32_t tls_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout)
{
    struct tls_t *net = (struct tls_t *)handle;
    int32_t ret = 0;

    net->send_timeout = timeout;
    ret = wolfSSL_write(net->wolf_ssl, buf, len);

    if(wolfSSL_want_write(net->wolf_ssl))
    {
        return 0;
    }

    return ret;
}

int32_t tls_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout)
{
    struct tls_t *net = (struct tls_t *)handle;
    int32_t ret = 0;

    net->recv_timeout = timeout;
    ret = wolfSSL_read(net->wolf_ssl, buf, len);

    if(wolfSSL_want_read(net->wolf_ssl))
    {
        return 0;
    }

    return ret;
}

int32_t tls_disconnect(handle_t handle)
{
    struct tls_t *net = (struct tls_t *)handle;

    if(net)
    {
        tcp_disconnect(net->handle);
        wolfSSL_free(net->wolf_ssl);
        wolfSSL_CTX_free(net->wolf_ctx);
        osl_free(net);
    }

    return 0;
}
#else
handle_t tls_connect(const char *host, uint16_t port, const int8_t *ca_cert,
                     uint16_t ca_cert_len, uint32_t timeout)
{
    return -1;
}

int32_t tls_send(handle_t handle, void *buf, uint32_t len, uint32_t timeout)
{
    return -1;
}

int32_t tls_recv(handle_t handle, void *buf, uint32_t len, uint32_t timeout)
{
    return -1;
}

int32_t tls_disconnect(handle_t handle)
{
    return -1;
}
#endif
