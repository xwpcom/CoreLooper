/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file http_api.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "plat_osl.h"
#include "plat_tcp.h"
#include "plat_time.h"
#include "err_def.h"

#include "http_api.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define CONFIG_HTTP_HEADER_MAX_CNT 10

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
enum request_pack_status_e
{
    PACK_METHOD = 0,
    PACK_URI_PATH,
    PACK_URI_QUERY,
    PACK_HEADER,
    PACK_BODY,
    PARSE_STATUS,
    PARSE_HEADER,
    PARSE_BODY
};

struct http_lenstr_t
{
    uint32_t len;
    int8_t *str;
};

struct http_kv_t
{
    struct http_lenstr_t name;
    struct http_lenstr_t value;
};

struct http_server_addr_t
{
    int8_t *addr;
    uint16_t port;
};

struct http_ctx_t
{
    struct http_server_addr_t addr;
    uint32_t status;
    uint8_t *packet;
    uint32_t buf_size;
    uint32_t packet_len;
    uint8_t *content_length_ptr;
    int32_t content_length;
    uint32_t status_code;
    struct http_kv_t headers[CONFIG_HTTP_HEADER_MAX_CNT];
    uint8_t *body;
    uint32_t body_len;
};

struct http_method_str_t
{
    uint32_t method;
    int8_t *method_str;
};
/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
static struct http_method_str_t method_tbl[] = {{HTTP_METHOD_GET, "GET"},
                                                {HTTP_METHOD_POST, "POST"},
                                                {HTTP_METHOD_PUT, "PUT"},
                                                {HTTP_METHOD_DELETE, "DELETE"}};

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
static int32_t parse_host(const int8_t *host, struct http_server_addr_t *addr)
{
    int8_t *colon_ptr = osl_strstr(host, (const int8_t *)":");

    if(colon_ptr)
    {
        addr->addr = osl_strndup(host, colon_ptr - host);
        addr->port = osl_atoi((const int8_t *)(colon_ptr + 1));
    }
    else
    {
        addr->addr = osl_strdup(host);
        addr->port = 80;
    }

    return ERR_OK;
}

void *http_new(enum http_method_e method, const int8_t *host, const int8_t *abs_path, uint32_t size)
{
    struct http_ctx_t *ctx = NULL;

    if(NULL == (ctx = osl_malloc(sizeof(*ctx))))
    {
        return ctx;
    }

    osl_memset(ctx, 0, sizeof(*ctx));
    ctx->packet = osl_malloc(size);
    if(NULL == ctx->packet)
    {
        goto exit;
    }
    osl_memset(ctx->packet, 0, size);
    ctx->buf_size = size;

    parse_host(host, &ctx->addr);
    if(NULL == abs_path)
    {
        abs_path = (const int8_t *)"*";
    }
    osl_sprintf((int8_t *)ctx->packet, "%s %s", method_tbl[method].method_str, abs_path);
    ctx->status = PACK_URI_PATH;

    return ctx;
exit:
    osl_free(ctx);
    return NULL;
}

void http_delete(void *ctx)
{
    struct http_ctx_t *http_ctx = (struct http_ctx_t *)ctx;

    if(!ctx)
    {
        return;
    }
    if(http_ctx->addr.addr)
    {
        osl_free(http_ctx->addr.addr);
    }
    if(http_ctx->packet)
    {
        osl_free(http_ctx->packet);
    }
    osl_free(ctx);
}

int32_t http_add_param(void *ctx, const int8_t *name, const int8_t *value)
{
    struct http_ctx_t *http_ctx = (struct http_ctx_t *)ctx;

    if(!ctx || !name || !value)
    {
        return ERR_INVALID_PARAM;
    }
    if((PACK_URI_PATH != http_ctx->status) && (PACK_URI_QUERY != http_ctx->status))
    {
        return ERR_OTHERS;
    }

    if(PACK_URI_PATH == http_ctx->status)
    {
        osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"?");
    }
    else
    {
        osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"&");
    }

    osl_strcat((int8_t *)http_ctx->packet, name);
    osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"=");
    osl_strcat((int8_t *)http_ctx->packet, value);
    http_ctx->status = PACK_URI_QUERY;

    return ERR_OK;
}

int32_t http_add_header(void *ctx, const int8_t *name, const int8_t *value)
{
    struct http_ctx_t *http_ctx = (struct http_ctx_t *)ctx;

    if(!ctx || !name || !value)
    {
        return ERR_INVALID_PARAM;
    }
    if((PACK_URI_PATH != http_ctx->status) && (PACK_URI_QUERY != http_ctx->status) && (PACK_HEADER != http_ctx->status))
    {
        return ERR_OTHERS;
    }
    if(PACK_HEADER != http_ctx->status)
    {
        osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)" HTTP/1.1\r\n");
        osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"Host:");
        osl_strcat((int8_t *)http_ctx->packet, http_ctx->addr.addr);
        if(80 != http_ctx->addr.port)
        {
            osl_sprintf((int8_t *)(http_ctx->packet + osl_strlen((const int8_t *)http_ctx->packet)), ":%d", http_ctx->addr.port);
        }
        osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"\r\n");
    }
    osl_strcat((int8_t *)http_ctx->packet, name);
    osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)":");
    if((0 == osl_strcmp(name, (const int8_t *)"Transfer-Encoding")) && (0 == osl_strcmp(value, (const int8_t *)"chunked")))
    {
        http_ctx->content_length = -1;
    }
    osl_strcat((int8_t *)http_ctx->packet, value);
    osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"\r\n");
    http_ctx->status = PACK_HEADER;
    return ERR_OK;
}

void refresh_content_length(struct http_ctx_t *http_ctx, uint32_t content_length)
{
    int8_t str[8] = {0};

    osl_sprintf(str, "%7d", content_length);
    osl_memcpy(http_ctx->content_length_ptr, str, 7);
}

int32_t http_add_body(void *ctx, uint8_t *body, uint32_t body_len)
{
    struct http_ctx_t *http_ctx = (struct http_ctx_t *)ctx;

    if(!ctx || !body || !body_len)
    {
        return ERR_INVALID_PARAM;
    }

    if((PACK_HEADER != http_ctx->status) && (PACK_BODY != http_ctx->status))
    {
        return ERR_OTHERS;
    }
    if(PACK_HEADER == http_ctx->status)
    {
        if(-1 != http_ctx->content_length)
        {
            osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"Content-Length:");
            http_ctx->content_length_ptr = http_ctx->packet + osl_strlen((const int8_t *)http_ctx->packet);
            osl_sprintf((int8_t *)http_ctx->content_length_ptr, "%7d\r\n", http_ctx->content_length);
        }
        osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"\r\n");
        http_ctx->packet_len = osl_strlen((const int8_t *)http_ctx->packet);
    }

    if(body)
    {
        osl_memcpy((int8_t *)(http_ctx->packet + http_ctx->packet_len), body, body_len);
        http_ctx->packet_len += body_len;
        if(-1 != http_ctx->content_length)
        {
            http_ctx->content_length += body_len;
            refresh_content_length(http_ctx, http_ctx->content_length);
        }
    }
    http_ctx->status = PACK_BODY;

    return ERR_OK;
}

static int32_t parse_header(int8_t *data, struct http_kv_t *kv)
{
    int8_t *p = data;

    if(('\r' == *p) && ('\n' == *(p + 1)))
    {
        return 2;
    }

    kv->name.str = data;
    while(':' != *p)
    {
        kv->name.len++;
        p++;
    }
    while((':' == *p) || (' ' == *p))
    {
        p++;
    }
    kv->value.str = p;
    while('\r' != *p)
    {
        kv->value.len++;
        p++;
    }
    return (p - data) + 2;
}

static int32_t parse_chunk_info(uint8_t *data, uint32_t *chunk_size)
{
    uint8_t *p = data;

    *chunk_size = 0;
    do
    {
        *chunk_size <<= 4;
        if(('0' <= *p) && ('9' >= *p))
        {
            *chunk_size += *p - '0';
        }
        else
        {
            *chunk_size += *p - 'a' + 10;
        }
        p++;
    } while('\r' != *p);

    return (p + 2 - data);
}

static int32_t http_parse_response(uint8_t *data, uint32_t data_len, struct http_ctx_t *ctx)
{
    int8_t *p = (int8_t *)data;
    int32_t ret = 0;

    switch(ctx->status)
    {
        case PARSE_STATUS:
        {
            /** Check Header is compeleted*/
            p = osl_strstr((const int8_t *)data, (const int8_t *)"\r\n\r\n");
            if(NULL == p)
            {
                return 0;
            }

            /** Status Line*/
            p = osl_strstr((const int8_t *)data, (const int8_t *)"HTTP/1.1 ");
            if(NULL == p)
            {
                return 0;
            }
            p += 9;

            /** Parse Status Code*/
            while(' ' != *p)
            {
                ctx->status_code = ctx->status_code * 10 + (*p - '0');
                p++;
            }

            p = osl_strstr((const int8_t *)p, (const int8_t *)"\r\n");
            p += 2;
            ctx->status = PARSE_HEADER;
        }
        case PARSE_HEADER:
        {
            uint32_t i = 0;
            do
            {
                ret = parse_header(p, &ctx->headers[i++]);
                p += ret;
            } while(2 != ret);

            ctx->body = (uint8_t *)p;
            for(i = 0; i < CONFIG_HTTP_HEADER_MAX_CNT; i++)
            {
                if(NULL == ctx->headers[i].name.str)
                {
                    break;
                }
                ctx->headers[i].name.str[ctx->headers[i].name.len] = '\0';
                ctx->headers[i].value.str[ctx->headers[i].value.len] = '\0';
            }
            http_get_resp_header(ctx, (const int8_t *)"Transfer-Encoding", (const int8_t **)&p);
            if(0 == osl_strcmp(p, (const int8_t *)"chunked"))
            {
                /** Only one chunk*/
                ctx->body += parse_chunk_info(ctx->body, &ctx->body_len);
            }
            else
            {
                http_get_resp_header(ctx, (const int8_t *)"Content-Length", (const int8_t **)&p);
                ctx->body_len = osl_atoi((const int8_t *)p);
            }
            ctx->status = PARSE_BODY;
        }
        case PARSE_BODY:
        {
            if((ctx->body + ctx->body_len - data) <= data_len)
            {
                return ctx->status_code;
            }
        }
        default:
            break;
    }
    return 0;
}

int32_t http_send_request(void *ctx, const int8_t *server, uint16_t server_port, uint8_t **resp_body,
                          uint32_t *resp_body_len, uint32_t timeout_ms)
{
    struct http_ctx_t *http_ctx = (struct http_ctx_t *)ctx;
    handle_t net_hdl = -1;
    handle_t cd_hdl = countdown_start(timeout_ms);
    struct http_server_addr_t net_addr = {0};
    int32_t ret = ERR_OK;
    int32_t data_len = 0;

    net_addr.addr = (server) ? (int8_t *)server : http_ctx->addr.addr;
    net_addr.port = (server_port) ? server_port : http_ctx->addr.port;
    if(PACK_BODY != http_ctx->status)
    {
        osl_strcat((int8_t *)http_ctx->packet, (const int8_t *)"\r\n");
        http_ctx->packet_len = osl_strlen((const int8_t *)http_ctx->packet);
    }

    net_hdl = tcp_connect((const int8_t *)net_addr.addr, net_addr.port, countdown_left(cd_hdl));
    if(0 > net_hdl)
    {
        countdown_stop(cd_hdl);
        goto exit;
    }
    do
    {
        ret = tcp_send(net_hdl, http_ctx->packet + data_len, http_ctx->packet_len - data_len, countdown_left(cd_hdl));
        if(0 < ret)
        {
            data_len += ret;
            if(data_len == http_ctx->packet_len)
            {
                break;
            }
        }
    } while(0 == countdown_is_expired(cd_hdl));

    http_ctx->status = PARSE_STATUS;
    data_len = 0;
    do
    {
        ret = tcp_recv(net_hdl, http_ctx->packet + data_len, http_ctx->buf_size - data_len, 50);
        if(0 < ret)
        {
            data_len += ret;
            if(0 < (ret = http_parse_response(http_ctx->packet, data_len, http_ctx)))
            {
                *resp_body = http_ctx->body;
                *resp_body_len = http_ctx->body_len;
                goto exit1;
            }
        }
    } while(0 == countdown_is_expired(cd_hdl));
    ret = ERR_TIMEOUT;

exit1:
    tcp_disconnect(net_hdl);
exit:
    countdown_stop(cd_hdl);
    return ret;
}

int32_t http_get_resp_header(void *ctx, const int8_t *name, const int8_t **value)
{
    struct http_ctx_t *http_ctx = (struct http_ctx_t *)ctx;
    uint32_t i = 0;

    if(!ctx || !name || !value)
    {
        return ERR_INVALID_PARAM;
    }

    for(i = 0; i < CONFIG_HTTP_HEADER_MAX_CNT; i++)
    {
        if(NULL == http_ctx->headers[i].name.str)
        {
            break;
        }
        if(0 == osl_strcmp(name, http_ctx->headers[i].name.str))
        {
            *value = http_ctx->headers[i].value.str;
            return ERR_OK;
        }
    }

    return ERR_INVALID_DATA;
}

int32_t http_get_resp_status(void *ctx)
{
    struct http_ctx_t *http_ctx = (struct http_ctx_t *)ctx;

    if(!ctx)
    {
        return ERR_INVALID_PARAM;
}

    return http_ctx->status_code;
}
