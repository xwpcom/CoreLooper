/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_coap.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_coap.h"
#include "dev_token.h"
#include "plat_osl.h"
#include "plat_time.h"
#include "plat_udp.h"
#include "err_def.h"
#include "config.h"
#include "log.h"
#include "er-coap-13.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define COAP_LIFETIME_MIN_S (16)     // 6s, but use 16s
#define COAP_LIFETIME_MAX_S (604800) // 7 days

#define COAP_LIFETIME_LIMIT_S (600)

#define THING_MODEL_COAP_SERVER_ADDR "183.230.102.116"
#define THING_MODEL_COAP_SERVER_PORT 5683
/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
enum CTX_STATE
{
    UNINIT = 0,
    INIT,
    ONLINE
};

struct tm_coap_ctx_t
{
    enum CTX_STATE ctx_state;

    uint32_t client_mid;
    int8_t *saastoken;
    int8_t *product_id;
    int8_t *dev_name;

    handle_t net_socket;
    int32_t (*recv_cb)(const int8_t * /** data_name*/, uint8_t * /** data*/, uint32_t /** data_len*/);
    uint8_t *recv_buf;
    uint16_t recv_mid;
    uint16_t recv_response_code; //保存ack报文的code
    uint8_t recv_token_len;
    uint8_t recv_token[COAP_TOKEN_LEN];

    int8_t *post_token;
    uint16_t post_token_len;

    uint32_t lifetime;
    uint32_t lifetime_ms;
    uint64_t lifetime_threshold_ms;
    handle_t lifetime_timer;
};
/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/
static int32_t tm_coap_update(uint32_t lifetime, uint32_t timeout_ms);
static int32_t tm_coap_parse(const int8_t *packet, uint32_t packet_len, int8_t **get_uri_path, int8_t **get_payload,
                             uint16_t *get_payload_len, uint8_t *get_code);
static int32_t tm_coap_send_post_packet(const int8_t *uri, uint8_t *payload, uint32_t payload_len,
                                        int8_t **get_uri_path, int8_t **get_payload, uint16_t *get_payload_len,
                                        coap_status_t expectation, uint32_t timeout_ms);

static int32_t send_with_expectation(const coap_packet_t *pkt, handle_t socket, coap_status_t expectation,
                                     int8_t **get_uri_path, int8_t **get_payload, uint16_t *get_payload_len,
                                     uint32_t timeout_ms);

static int32_t uri_add_path_tm_prefix(coap_packet_t *pkt, const char *suffix, const int8_t *product_id,
                                      const int8_t *dev_name);
static int32_t payload_add_lifetime_and_saastoken(coap_packet_t *pkt, uint32_t lifetime, const int8_t *saastoken);
static void add_random_token(coap_packet_t *pkt, uint16_t mid);

static int32_t tm_udp_packet_handle(int8_t **get_uri_path, int8_t **get_payload, uint16_t *get_payload_len,
                                    handle_t timeout_cd);
/*****************************************************************************/
/*****************************************************************************/
/* Local Variables                                                           */
static struct tm_coap_ctx_t tm_coap_ctx = {0};
/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
int32_t tm_coap_init(int32_t (*recv_cb)(const int8_t * /** data_name*/, uint8_t * /** data*/, uint32_t /** data_len*/))
{
    if(tm_coap_ctx.ctx_state != UNINIT)
    {
        return ERR_REPETITIVE;
    }

    osl_memset(&tm_coap_ctx, 0, sizeof(struct tm_coap_ctx_t));
    tm_coap_ctx.net_socket = udp_connect(THING_MODEL_COAP_SERVER_ADDR, THING_MODEL_COAP_SERVER_PORT);
    if(tm_coap_ctx.net_socket <= 0)
    {
        osl_memset(&tm_coap_ctx, 0, sizeof(struct tm_coap_ctx_t));
        return ERR_UNINITIALIZED;
    }

    tm_coap_ctx.recv_cb = recv_cb;
    tm_coap_ctx.recv_buf = osl_malloc(FEATURE_TM_RECV_BUF_LEN);
    osl_assert(tm_coap_ctx.recv_buf != NULL);
    osl_memset(tm_coap_ctx.recv_buf, 0, FEATURE_TM_RECV_BUF_LEN);
    tm_coap_ctx.ctx_state = INIT;
    tm_coap_ctx.client_mid = 1;

    return ERR_OK;
}

int32_t tm_coap_login(const int8_t *product_id, const int8_t *dev_name, const int8_t *dev_token, uint32_t life_time,
                      uint32_t timeout_ms)
{
    int32_t ret = ERR_OTHERS;
    uint16_t mid = 0;

    if(tm_coap_ctx.ctx_state != INIT || life_time < COAP_LIFETIME_MIN_S || life_time > COAP_LIFETIME_MAX_S)
    {
        return ret;
    }
    mid = tm_coap_ctx.client_mid++;

    if(tm_coap_ctx.saastoken == NULL)
    {
        tm_coap_ctx.saastoken = osl_malloc_with_zero(osl_strlen(dev_token) + 1);
        if(tm_coap_ctx.saastoken == NULL)
        {
            return ERR_NO_MEM;
        }
        osl_memcpy(tm_coap_ctx.saastoken, dev_token, osl_strlen(dev_token));
    }

    tm_coap_ctx.lifetime = life_time;
    tm_coap_ctx.lifetime_ms = life_time * 1000;
    tm_coap_ctx.product_id = osl_strdup(product_id);
    tm_coap_ctx.dev_name = osl_strdup(dev_name);

    coap_packet_t message[1] = {0};
    coap_init_message(message, COAP_TYPE_CON, COAP_POST, mid);

    //$sys/{pid}/{device-name}/auth
    ret = uri_add_path_tm_prefix(message, "login", tm_coap_ctx.product_id, tm_coap_ctx.dev_name);
    if(ret != ERR_OK)
    {
        return ret;
    }

    ret = payload_add_lifetime_and_saastoken(message, tm_coap_ctx.lifetime, tm_coap_ctx.saastoken);
    if(ret != ERR_OK)
    {
        return ret;
    }
    add_random_token(message, mid);
    ret = send_with_expectation(message, tm_coap_ctx.net_socket, COAP_STATUS_CREATED_2_01, NULL,
                                &tm_coap_ctx.post_token, &tm_coap_ctx.post_token_len, timeout_ms);

    if(ret == ERR_OK)
    {
        log_info("login ok.\n");

        tm_coap_ctx.ctx_state = ONLINE;

        //登录成功后建立心跳的定时任务
        if(tm_coap_ctx.lifetime < COAP_LIFETIME_LIMIT_S)
        {
            tm_coap_ctx.lifetime_threshold_ms = (uint64_t)(tm_coap_ctx.lifetime_ms / 2);
        }
        else
        {
            tm_coap_ctx.lifetime_threshold_ms = 500 * COAP_LIFETIME_LIMIT_S; // 1000 * (x / 2)
        }
        tm_coap_ctx.lifetime_timer = countdown_start(tm_coap_ctx.lifetime_ms);
    }
    else
    {
        log_error("login failed.\n");
        udp_disconnect(tm_coap_ctx.net_socket);
        osl_free(tm_coap_ctx.recv_buf);
        osl_free(tm_coap_ctx.product_id);
        osl_free(tm_coap_ctx.dev_name);
        osl_free(tm_coap_ctx.saastoken);
        osl_memset(&tm_coap_ctx, 0, sizeof(struct tm_coap_ctx_t));
    }
    osl_free(message->payload);
    return ret;
}

int32_t tm_coap_logout(uint32_t timeout_ms)
{
    coap_packet_t message[1] = {0};
    uint16_t mid = 0;
    int32_t ret = ERR_OTHERS;

    if(tm_coap_ctx.ctx_state != ONLINE)
    {
        return ERR_OTHERS;
    }
    mid = tm_coap_ctx.client_mid++;

    coap_init_message(message, COAP_TYPE_CON, COAP_DELETE, mid);

    //$sys/{pid}/{device-name}/offline
    ret = uri_add_path_tm_prefix(message, "logout", tm_coap_ctx.product_id, tm_coap_ctx.dev_name);
    if(ret != ERR_OK)
    {
        return ret;
    }

    ret = payload_add_lifetime_and_saastoken(message, 0, tm_coap_ctx.saastoken);
    if(ret != ERR_OK)
    {
        return ret;
    }

    add_random_token(message, mid);

    ret =
        send_with_expectation(message, tm_coap_ctx.net_socket, COAP_STATUS_DELETED_2_02, NULL, NULL, NULL, timeout_ms);
    if(ret == ERR_OK)
    {
        log_info("logout ok.\n");
    }
    udp_disconnect(tm_coap_ctx.net_socket);
    osl_free(tm_coap_ctx.recv_buf);
    osl_free(tm_coap_ctx.product_id);
    osl_free(tm_coap_ctx.dev_name);
    osl_free(tm_coap_ctx.saastoken);
    osl_free(tm_coap_ctx.post_token);
    countdown_stop(tm_coap_ctx.lifetime_timer);
    osl_memset(&tm_coap_ctx, 0, sizeof(struct tm_coap_ctx_t));

    osl_free(message->payload);
    return ret;
}

int32_t tm_coap_step(uint32_t timeout_ms)
{
    int ret = ERR_OK;
    if(tm_coap_ctx.ctx_state != ONLINE)
    {
        return ERR_OTHERS;
    }
    handle_t cd_hdl = countdown_start(timeout_ms);

    if(countdown_left(tm_coap_ctx.lifetime_timer) <= tm_coap_ctx.lifetime_threshold_ms)
    {
        ret = tm_coap_update(tm_coap_ctx.lifetime, timeout_ms);
        if(ret == ERR_OK)
        {
            log_info("keep alive ok.\n");
            countdown_set(tm_coap_ctx.lifetime_timer, tm_coap_ctx.lifetime_ms);
        }
        else
        {
            log_info("keep alive failed.\n");
        }

        if(countdown_is_expired(tm_coap_ctx.lifetime_timer) == 1)
        {
            log_error("try login again.\n");
            countdown_stop(tm_coap_ctx.lifetime_timer);
            tm_coap_ctx.ctx_state = INIT;
            osl_free(tm_coap_ctx.post_token);
            tm_coap_ctx.post_token = NULL;
            tm_coap_ctx.post_token_len = 0;
            tm_coap_login(tm_coap_ctx.product_id, tm_coap_ctx.dev_name, tm_coap_ctx.saastoken, tm_coap_ctx.lifetime,
                          2000);
        }
    }

    if(tm_udp_packet_handle(NULL, NULL, NULL, cd_hdl) > 0)
    {
        log_debug("coap_step handle a udp packet\n");
    }

    countdown_stop(cd_hdl);
    return ret;
}

int32_t tm_coap_reply(const int8_t *payload, uint32_t payload_len, uint32_t timeout_ms)
{
    coap_packet_t response[1] = {0};
    coap_init_message(response, COAP_TYPE_ACK, COAP_STATUS_CONTENT_2_05, tm_coap_ctx.recv_mid);
    coap_set_header_token(response, tm_coap_ctx.recv_token, tm_coap_ctx.recv_token_len);
    coap_set_header_content_type(response, APPLICATION_JSON);
    coap_set_payload(response, payload, payload_len);
    return send_with_expectation(response, tm_coap_ctx.net_socket, COAP_STATUS_NO_ERROR, NULL, NULL, NULL, timeout_ms);
}

static int32_t tm_coap_update(uint32_t lifetime, uint32_t timeout_ms)
{
    int ret = ERR_OTHERS;
    coap_packet_t message[1] = {0};
    uint16_t mid = tm_coap_ctx.client_mid++;
    coap_init_message(message, COAP_TYPE_CON, COAP_POST, mid);

    //$sys/{pid}/{device-name}/heart
    ret = uri_add_path_tm_prefix(message, "keep_alive", tm_coap_ctx.product_id, tm_coap_ctx.dev_name);
    if(ret != ERR_OK)
    {
        return ret;
    }

    ret = payload_add_lifetime_and_saastoken(message, lifetime, tm_coap_ctx.saastoken);
    if(ret != ERR_OK)
    {
        return ret;
    }

    add_random_token(message, mid);

    ret =
        send_with_expectation(message, tm_coap_ctx.net_socket, COAP_STATUS_CHANGED_2_04, NULL, NULL, NULL, timeout_ms);
    osl_free(message->payload);
    return ret;
}

static int32_t tm_coap_parse(const int8_t *packet, uint32_t packet_len, int8_t **get_uri_path, int8_t **get_payload,
                             uint16_t *get_payload_len, uint8_t *get_code)
{
    coap_packet_t coap_packet[1] = {0};
    coap_status_t coap_error_code = COAP_STATUS_NO_ERROR;

    int ret = ERR_NO_MEM;

    coap_error_code = coap_parse_message(coap_packet, (uint8_t *)packet, packet_len);

    if(coap_error_code == COAP_STATUS_NO_ERROR)
    {
        log_debug("Parsed: type %u, tkl %u, token %02x%02x%02x%02x%02x%02x%02x%02x ,code %u.%.2u, mid %u",
                  coap_packet->type, coap_packet->token_len, coap_packet->token[0], coap_packet->token[1],
                  coap_packet->token[2], coap_packet->token[3], coap_packet->token[4], coap_packet->token[5],
                  coap_packet->token[6], coap_packet->token[7], coap_packet->code >> 5, coap_packet->code & 0x1F,
                  coap_packet->mid);

        if(get_uri_path != NULL && IS_OPTION(coap_packet, COAP_OPTION_URI_PATH) && *get_uri_path == NULL)
        {
            multi_option_t *uri_path_ptr = coap_packet->uri_path;
            uint16_t uri_path_len = 0;
            uint8_t _flow_flag_ = 0;

            while(1)
            {
                if(_flow_flag_ == 0)
                {
                    if(uri_path_ptr != NULL)
                    {
                        uri_path_len += uri_path_ptr->len;
                        uri_path_len += 1;
                    }
                    else
                    {
                        _flow_flag_ = 1;
                        uri_path_ptr = coap_packet->uri_path;

                        *get_uri_path = osl_malloc(uri_path_len + 1);
                        if(*get_uri_path == NULL)
                        {
                            log_debug("get_uri_path malloc error\n");
                            goto _parse_exit_;
                        }

                        osl_memset(*get_uri_path, 0, uri_path_len + 1);
                        uri_path_len = 0;
                        continue;
                    }
                }
                else // _flow_flag_ == 1
                {
                    if(uri_path_ptr != NULL)
                    {
                        osl_memcpy((*get_uri_path) + uri_path_len, uri_path_ptr->data, uri_path_ptr->len);
                        uri_path_len += uri_path_ptr->len;
                        if(uri_path_ptr->next != NULL)
                        {
                            (*get_uri_path)[uri_path_len] = '/';
                            uri_path_len += 1;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                uri_path_ptr = uri_path_ptr->next;
            }
        }

        tm_coap_ctx.recv_mid = coap_packet->mid;
        tm_coap_ctx.recv_token_len = coap_packet->token_len;
        osl_memset(tm_coap_ctx.recv_token, 0, COAP_TOKEN_LEN);
        osl_memcpy(tm_coap_ctx.recv_token, coap_packet->token, tm_coap_ctx.recv_token_len);

        if(get_payload != NULL && coap_packet->payload_len != 0 && *get_payload == NULL)
        {
            *get_payload = osl_malloc(coap_packet->payload_len + 1);
            if(*get_payload != NULL)
            {
                osl_memset(*get_payload, 0, coap_packet->payload_len + 1);
                *get_payload_len = coap_packet->payload_len;
                osl_memcpy(*get_payload, coap_packet->payload, *get_payload_len);
            }
            else
            {
                log_debug("get_payload malloc error\n");
                if(*get_uri_path != NULL)
                {
                    osl_free(*get_uri_path);
                    *get_uri_path = NULL;
                }
                goto _parse_exit_;
            }
        }

        if(get_code != NULL && coap_packet->type == COAP_TYPE_ACK)
        {
            *get_code = coap_packet->code;
        }
        ret = ERR_OK;
    }
    else
    {
        log_debug("Failed to parse message: %u.%2u", coap_error_code >> 5, coap_error_code & 0x1F);
    }

_parse_exit_:
    coap_free_header(coap_packet);
    return ret;
}

static int32_t tm_coap_send_post_packet(const int8_t *uri, uint8_t *payload, uint32_t payload_len,
                                        int8_t **get_uri_path, int8_t **get_payload, uint16_t *get_payload_len,
                                        coap_status_t expectation, uint32_t timeout_ms)
{
    int32_t ret = ERR_OTHERS;

    coap_packet_t message[1] = {0};
    coap_init_message(message, COAP_TYPE_CON, COAP_POST, tm_coap_ctx.client_mid++);

    coap_set_header_uri_path(message, uri);

    coap_set_header_token(message, tm_coap_ctx.post_token, tm_coap_ctx.post_token_len);

    coap_set_header_content_type(message, APPLICATION_JSON);
    coap_set_header_accept(message, APPLICATION_JSON);

    coap_set_payload(message, payload, payload_len);

    ret = send_with_expectation(message, tm_coap_ctx.net_socket, expectation, get_uri_path, get_payload,
                                get_payload_len, timeout_ms);

    if(expectation != COAP_STATUS_NO_ERROR && ret == ERR_OK) //刷新心跳任务
    {
        countdown_set(tm_coap_ctx.lifetime_timer, tm_coap_ctx.lifetime_ms);
    }
    return ret;
}

static int32_t send_with_expectation(const coap_packet_t *pkt, handle_t socket, coap_status_t expectation,
                                     int8_t **get_uri_path, int8_t **get_payload, uint16_t *get_payload_len,
                                     uint32_t timeout_ms)
{
    size_t packet_buffer_len = 0;
    uint8_t *packet_buffer = NULL;
    int32_t ret = ERR_OTHERS;

    handle_t timer_handle = countdown_start(timeout_ms);

    size_t alloc_len = 0;
    alloc_len = coap_serialize_get_size((void *)pkt);
    if(alloc_len == 0)
    {
        goto _exit_;
    }

    packet_buffer = (uint8_t *)osl_malloc(alloc_len);
    if(packet_buffer == NULL)
    {
        goto _exit_;
    }

    packet_buffer_len = coap_serialize_message((void *)pkt, packet_buffer);

    // udp send
    ret = udp_send(socket, packet_buffer, packet_buffer_len, countdown_left(timer_handle));
    osl_free(packet_buffer);
    if(ret < -1)
    {
        log_error("UDP send buffer error\n");
        goto _exit_;
    }
    if(expectation != COAP_STATUS_NO_ERROR) //如果是COAP_STATUS_NO_ERROR代表不需要处理ack
    {
        // udp recv
        //等待接收并解析，超时则返回

        while(countdown_is_expired(timer_handle) == 0)
        {
            int32_t handle_len = 0;
            handle_len = tm_udp_packet_handle(get_uri_path, get_payload, get_payload_len, timer_handle);
            if(handle_len != 0)
            {
                log_debug("send_with_expectation handle a udp packet\n");
                if(tm_coap_ctx.recv_mid == pkt->mid)
                {
                    int64_t recv_token = (int64_t)(*(int64_t *)(&(tm_coap_ctx.recv_token)));
                    int64_t pkt_token = (int64_t)(*(int64_t *)(&(pkt->token)));
                    if(recv_token == pkt_token)
                    {
                        if(tm_coap_ctx.recv_response_code == expectation)
                        {
                            ret = ERR_OK;
                        }
                        goto _exit_;
                    }
                }
            }
        }
    }

_exit_:
    countdown_stop(timer_handle);
    return ret;
}

static int32_t uri_add_path_tm_prefix(coap_packet_t *pkt, const char *suffix, const int8_t *product_id,
                                      const int8_t *dev_name)
{
    //$sys/{pid}/{device-name}/
    uint16_t header_uri_temp_length = 8 + osl_strlen((const int8_t *)suffix) + osl_strlen((const int8_t *)product_id)
                                      + osl_strlen((const int8_t *)dev_name); // 7+1
    char *header_uri_temp = osl_malloc_with_zero(header_uri_temp_length);
    if(header_uri_temp == NULL)
    {
        return ERR_NO_MEM;
    }

    osl_sprintf(header_uri_temp, "$sys/%s/%s/%s", product_id, dev_name, suffix);
    coap_set_header_uri_path(pkt, header_uri_temp);
    osl_free(header_uri_temp);
    return ERR_OK;
}

static int32_t payload_add_lifetime_and_saastoken(coap_packet_t *pkt, uint32_t lifetime, const int8_t *saastoken)
{
    char *payload_temp = NULL;
    uint16_t payload_temp_length = 4; //{,}

    payload_temp_length += (7 + osl_strlen(saastoken)); //"st":""

    if(lifetime != 0)
    {
        payload_temp_length += 14; // 4+10,10是2的32次方 "lt":
    }

    payload_temp = osl_malloc_with_zero(payload_temp_length);
    if(payload_temp == NULL)
    {
        return ERR_NO_MEM;
    }

    osl_sprintf(payload_temp, "{\"st\":\"%s\"", saastoken);

    if(lifetime != 0)
    {
        osl_sprintf(payload_temp + osl_strlen(payload_temp), ",\"lt\":%d", lifetime);
    }
    osl_strcat(payload_temp, "}");

    coap_set_header_content_type(pkt, APPLICATION_JSON);
    coap_set_payload(pkt, payload_temp, osl_strlen(payload_temp));
    return ERR_OK;
}

static void add_random_token(coap_packet_t *pkt, uint16_t mid)
{
    // generate a token
    uint8_t temp_token[8];
    uint64_t tv_sec = time_count_ms();

    temp_token[0] = (uint8_t)(mid | (tv_sec >> 2));
    temp_token[1] = (uint8_t)(mid | (tv_sec >> 4));
    temp_token[2] = (uint8_t)(tv_sec);
    temp_token[3] = (uint8_t)(tv_sec >> 6);
    temp_token[4] = (uint8_t)(tv_sec >> 8);
    temp_token[5] = (uint8_t)(tv_sec >> 10);
    temp_token[6] = (uint8_t)(tv_sec >> 12);
    temp_token[7] = (uint8_t)(tv_sec >> 14);

    coap_set_header_token(pkt, temp_token, 8);
}

int32_t tm_coap_send_packet(const int8_t *uri, uint8_t *payload, uint32_t payload_len, uint32_t timeout_ms)
{
    if(uri)
    {
        int8_t *get_payload = NULL;
        uint16_t get_payload_len = 0;
        int32_t ret = 0;
        ret = tm_coap_send_post_packet(uri, payload, payload_len, NULL, &get_payload, &get_payload_len,
                                        COAP_STATUS_CONTENT_2_05, timeout_ms);
        if((ERR_OK == ret) && (NULL != get_payload))
        {
            tm_coap_ctx.recv_cb(uri, get_payload, get_payload_len);
        }
        if(NULL != get_payload)
        {
            osl_free(get_payload);
        }
        return ret;
    }
    else
    {
        return tm_coap_reply(payload, payload_len, timeout_ms);
    }
}

static int32_t tm_udp_packet_handle(int8_t **get_uri_path, int8_t **get_payload, uint16_t *get_payload_len,
                                    handle_t timeout_cd)
{
    uint32_t recv_len = 0;
    int32_t parse_ret = 0;
    while(0 == countdown_is_expired(timeout_cd))
    {
        int8_t *_uri = NULL;
        int8_t *_payload = NULL;
        uint16_t _payload_len = 0;

        recv_len =
            udp_recv(tm_coap_ctx.net_socket, tm_coap_ctx.recv_buf, FEATURE_TM_RECV_BUF_LEN, countdown_left(timeout_cd));
        if(recv_len)
        {
            tm_coap_ctx.recv_mid = 0;
            tm_coap_ctx.recv_response_code = 0;

            parse_ret = tm_coap_parse(tm_coap_ctx.recv_buf, recv_len, &_uri, &_payload, &_payload_len,
                                      &(tm_coap_ctx.recv_response_code));
            if(parse_ret == COAP_STATUS_NO_ERROR)
            {
                if(_uri != NULL)
                {
                    if(_payload != NULL)
                    {
                        tm_coap_ctx.recv_cb(_uri, _payload, _payload_len);
                    }
                }

                if(get_uri_path != NULL)
                {
                    *get_uri_path = _uri;
                }
                else if(_uri != NULL)
                {
                    osl_free(_uri);
                }

                if(get_payload != NULL)
                {
                    *get_payload = _payload;
                }
                else if(_payload != NULL)
                {
                    osl_free(_payload);
                }

                if(get_payload_len != NULL)
                {
                    *get_payload_len = _payload_len;
                }
            }

            osl_memset(tm_coap_ctx.recv_buf, 0, FEATURE_TM_RECV_BUF_LEN);
            return recv_len;
        }
    }
    return 0;
}
