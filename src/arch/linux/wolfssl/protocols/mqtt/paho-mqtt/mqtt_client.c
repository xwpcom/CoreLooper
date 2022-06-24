/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 *
 * @file mqtt_client.c
 * @brief 本文件来源于PahoMQTT MQTTClient-C\src\MQTTClient.c。
 */


/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "plat_osl.h"
#include "plat_time.h"
#include "plat_tcp.h"
#include "mqtt_api.h"
#include "mqtt_client.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define MAX_PACKET_ID 65535 /* according to the MQTT specification - do not change! */

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
typedef struct mqtt_client
{
    unsigned int next_packetid;
    size_t buf_size,
           readbuf_size;
    unsigned char *buf,
             *readbuf;
    unsigned int keepAliveInterval;
    char ping_outstanding;
    int isconnected;
    int cleansession;

    struct MessageHandlers
    {
        const char *topicFilter;
        #if 0 // New message handler
        void (*fp)(mqtt_message_data *);
        #else
        void (*fp)(void *, const int8_t *, struct mqtt_message_t *);
        void *arg;
        #endif
    } messageHandlers[MAX_MESSAGE_HANDLERS];      /* Message handlers are indexed by subscription topic */

    #if 0 // New message handler
    void (*defaultMessageHandler)(mqtt_message_data *);
    #endif

    mqtt_network *ipstack;
    handle_t last_sent, last_received;
} mqtt_client;

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
#if 0 // New message handler
static void NewMessageData(mqtt_message_data *md, MQTTString *aTopicName,
                           struct mqtt_message_t *aMessage)
{
    md->topicName = aTopicName;
    md->message = aMessage;
}
#endif

static int getNextPacketId(mqtt_client *c)
{
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 :
                              c->next_packetid + 1;
}

static int sendPacket(mqtt_client *c, int length, handle_t cd_handle)
{
    int rc = FAILURE,
        sent = 0;

    // printf("mqtt send data: ");
    // for (int i = 0; i < length; i++)
    // {
    //     printf("%02x ", c->buf[i]);
    // }
    // printf("\n");

    while(sent < length && !countdown_is_expired(cd_handle))
    {  
        rc = c->ipstack->mqttwrite(c->ipstack->handle, &c->buf[sent], length,
                                   countdown_left(cd_handle));

        if(rc < 0)   // there was an error writing the data
        {
            break;
        }

        sent += rc;
    }

    if(sent == length)
    {
        countdown_set(c->last_sent,
                      c->keepAliveInterval *
                      1000); // record the fact that we have successfully sent the packet
        rc = SUCCESS;
    }
    else
    {
        log_error("Mqtt client send packet failed!");
        rc = FAILURE;
    }

    return rc;
}

void *mqtt_client_init(mqtt_network *network,
                       unsigned char *sendbuf, size_t sendbuf_size, unsigned char *readbuf,
                       size_t readbuf_size)
{
    int i;
    mqtt_client *c = NULL;

    c = (mqtt_client *)osl_malloc(sizeof(*c));

    if(NULL == c)
    {
        return NULL;
    }

    osl_memset(c, 0, sizeof(*c));

    c->ipstack = network;

    for(i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        c->messageHandlers[i].topicFilter = 0;
    }

    c->buf = sendbuf;
    c->buf_size = sendbuf_size;
    c->readbuf = readbuf;
    c->readbuf_size = readbuf_size;
    c->isconnected = 0;
    c->cleansession = 0;
    c->ping_outstanding = 0;
    #if 0 // New message handler
    c->defaultMessageHandler = NULL;
    #endif
    c->next_packetid = 1;
    c->last_sent = countdown_start(0);
    c->last_received = countdown_start(0);

    return c;
}

void mqtt_client_deinit(void *client)
{
    if(client)
    {
        countdown_stop(((mqtt_client *)client)->last_sent);
        countdown_stop(((mqtt_client *)client)->last_received);
        osl_free(client);
    }
}

static int decodePacket(mqtt_client *c, int *value, int timeout)
{
    unsigned char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;

    do
    {
        int rc = MQTTPACKET_READ_ERROR;

        if(++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
        {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }

        rc = c->ipstack->mqttread(c->ipstack->handle, &i, 1, timeout);

        if(rc != 1)
        {
            goto exit;
        }

        *value += (i & 127) * multiplier;
        multiplier *= 128;
    }
    while((i & 128) != 0);

exit:
    return len;
}

static int readPacket(mqtt_client *c, handle_t cd_handle)
{
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    int rc = c->ipstack->mqttread(c->ipstack->handle, c->readbuf, 1,
                                  countdown_left(cd_handle));

    if(rc != 1)
    {
        goto exit;
    }

    // printf("mqtt read header: ");
    // for (int i = 0; i < rc; i++)
    // {
    //     printf("%02x ", c->readbuf[i]);
    // }
    // printf("\n");

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, countdown_left(cd_handle));
    len += MQTTPacket_encode(c->readbuf + 1,
                             rem_len); /* put the original remaining length back into the buffer */

    if(rem_len > (c->readbuf_size - len))
    {
        rc = BUFFER_OVERFLOW;
        goto exit;
    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if(rem_len > 0 &&
            (rc = c->ipstack->mqttread(c->ipstack->handle, c->readbuf + len, rem_len,
                                       countdown_left(cd_handle)) != rem_len))
    {
        rc = 0;
        goto exit;
    }

    // printf("mqtt read data: ");
    // for (int i = 0; i < rem_len; i++)
    // {
    //     printf("%02x ", c->readbuf[len + i]);
    // }
    // printf("\n");

    header.byte = c->readbuf[0];
    rc = header.bits.type;

    if(c->keepAliveInterval > 0)
    {
        countdown_set(c->last_received, c->keepAliveInterval *
                      1000);    // record the fact that we have successfully received a packet
    }

exit:
    return rc;
}

// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char *topicFilter, MQTTString *topicName)
{
    char *curf = topicFilter;
    char *curn = topicName->lenstring.data;
    char *curn_end = curn + topicName->lenstring.len;

    while(*curf && curn < curn_end)
    {
        if(*curn == '/' && *curf != '/')
        {
            break;
        }

        if(*curf != '+' && *curf != '#' && *curf != *curn)
        {
            break;
        }

        if(*curf == '+')
        {
            // skip until we meet the next separator, or end of string
            char *nextpos = curn + 1;

            while(nextpos < curn_end && *nextpos != '/')
            {
                nextpos = ++curn + 1;
            }
        }
        else if(*curf == '#')
        {
            curn = curn_end - 1;    // skip until end of string
        }

        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}

static int deliverMessage(mqtt_client *c, MQTTString *topicName,
                          struct mqtt_message_t *message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for(i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if(c->messageHandlers[i].topicFilter != 0 &&
                (MQTTPacket_equals(topicName, (char *)c->messageHandlers[i].topicFilter) ||
                 isTopicMatched((char *)c->messageHandlers[i].topicFilter, topicName)))
        {
            if(c->messageHandlers[i].fp != NULL)
            {
                #if 0 // New message handler
                mqtt_message_data md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].fp(&md);
                #else
                int data_len = topicName->lenstring.len;
                char *data = NULL;
                if (NULL != (data = malloc(data_len + 1)))
                {
                    memset(data, 0, data_len + 1);
                    memcpy(data, topicName->lenstring.data, data_len);
                    c->messageHandlers[i].fp(c->messageHandlers[i].arg, data, message);
                    free(data);
                }
                #endif
                rc = SUCCESS;
            }
        }
    }
#if 0 // New message handler
    if(rc == FAILURE && c->defaultMessageHandler != NULL)
    {
        mqtt_message_data md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler(&md);
        rc = SUCCESS;
    }
#endif
    return rc;
}

static int keepalive(mqtt_client *c)
{
    int rc = SUCCESS;

    if(c->keepAliveInterval == 0)
    {
        goto exit;
    }

    /**
     * 为保障MQTT响应的及时性，目前数据通道的MQTT yield间隔设置为50ms，一般情况下工作正常。
     * 某些情况下，心跳包的req和resp间隔可能达数百ms，超过调度间隔，导致心跳判定失败。为解决
     * 该问题，目前修改为只有距离上一次发包间隔超过keepalive时间时才会进入心跳处理流程，也就是
     * 说一旦服务器没有及时返回resp，设备端会在下一次心跳的时候才会判定上一次心跳是否成功。如果
     * 在这段时间内正常收到了resp，那就继续一次正常的心跳，否则报错，防止resp延时导致心跳失败
    */
    //if (TimerIsExpired(&c->last_sent) || TimerIsExpired(&c->last_received))
    if(countdown_is_expired(c->last_sent))
    {
        if(c->ping_outstanding)
        {
            rc = FAILURE;    /* PINGRESP not received in keepalive interval */
        }
        else
        {
            handle_t countdown_hdl = countdown_start(2000);

            int len = MQTTSerialize_pingreq(c->buf, c->buf_size);

            if(len > 0 &&
                    (rc = sendPacket(c, len, countdown_hdl)) == SUCCESS) // send the ping packet
            {
                c->ping_outstanding = 1;
            }

            countdown_stop(countdown_hdl);
        }
    }

exit:
    return rc;
}

static void MQTTCleanSession(mqtt_client *c)
{
    int i = 0;

    for(i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        c->messageHandlers[i].topicFilter = NULL;
    }
}

static void MQTTCloseSession(mqtt_client *c)
{
    c->ping_outstanding = 0;
    c->isconnected = 0;

    if(c->cleansession)
    {
        MQTTCleanSession(c);
    }
}

static int cycle(mqtt_client *c, handle_t cd_handle)
{
    int len = 0,
        rc = SUCCESS;

    int packet_type = readPacket(c, cd_handle);     /* read the socket, see what work is due */

    switch(packet_type)
    {
    default:
        /* no more data to read, unrecoverable. Or read packet fails due to unexpected network error */
        rc = packet_type;
        goto exit;

    case 0: /* timed out reading packet */
        break;

    case CONNACK:
    case PUBACK:
    case SUBACK:
    case UNSUBACK:
        break;

    case PUBLISH:
    {
        MQTTString topicName;
        struct mqtt_message_t msg;
        int intQoS;
        msg.payload_len =
            0; /* this is a size_t, but deserialize publish sets this as int */

        if(MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id,
                                   &topicName,
                                   (unsigned char **)&msg.payload, (int *)&msg.payload_len, c->readbuf,
                                   c->readbuf_size) != 1)
        {
            goto exit;
        }

        msg.qos = (enum mqtt_qos_e)intQoS;
        deliverMessage(c, &topicName, &msg);

        if(msg.qos != MQTT_QOS0)
        {
            if(msg.qos == MQTT_QOS1)
            {
                len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
            }
            else if(msg.qos == MQTT_QOS2)
            {
                len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
            }

            if(len <= 0)
            {
                rc = FAILURE;
            }
            else
            {
                rc = sendPacket(c, len, cd_handle);
            }

            if(rc == FAILURE)
            {
                goto exit;    // there was a problem
            }
        }

        break;
    }

    case PUBREC:
    case PUBREL:
    {
        unsigned short mypacketid;
        unsigned char dup, type;

        if(MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf,
                               c->readbuf_size) != 1)
        {
            rc = FAILURE;
        }
        else if((len = MQTTSerialize_ack(c->buf, c->buf_size,
                                         (packet_type == PUBREC) ? PUBREL : PUBCOMP, 0, mypacketid)) <= 0)
        {
            rc = FAILURE;
        }
        else if((rc = sendPacket(c, len,
                                 cd_handle)) != SUCCESS)  // send the PUBREL packet
        {
            rc = FAILURE;    // there was a problem
        }

        if(rc == FAILURE)
        {
            goto exit;    // there was a problem
        }

        break;
    }

    case PUBCOMP:
        break;

    case PINGRESP:
        log_info("keep alive ok\n");
        c->ping_outstanding = 0;
        break;
    }

    if(keepalive(c) != SUCCESS)
    {
        //check only keepalive FAILURE status so that previous FAILURE status can be considered as FAULT
        log_error("Mqtt keep alive time out!");
        rc = FAILURE;
    }

exit:

    if(rc == SUCCESS)
    {
        rc = packet_type;
    }
    else if(c->isconnected)
    {
        MQTTCloseSession(c);
    }

    return rc;
}

int mqtt_client_yield(void *client, int timeout_ms)
{
    mqtt_client *c = (mqtt_client *)client;
    int rc = SUCCESS;
    handle_t yield_cd_hdl = 0;

    yield_cd_hdl = countdown_start(timeout_ms);

    if (0 > cycle(c, yield_cd_hdl))
    {
        rc = FAILURE;
    }

    countdown_stop(yield_cd_hdl);

    return rc;
}

static int waitfor(mqtt_client *c, int packet_type, handle_t cd_handle)
{
    int rc = FAILURE;

    do
    {
        if(countdown_is_expired(cd_handle))
        {
            break;    // we timed out
        }

        rc = cycle(c, cd_handle);
    }
    while(rc != packet_type && rc >= 0);

    return rc;
}

int mqtt_client_connect_with_results(void *client, MQTTPacket_connectData *options,
                                     mqtt_conn_ack_data *data, uint32_t timeout_ms)
{
    handle_t connect_cd_hdl = 0;
    int rc = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    mqtt_client *c = (mqtt_client *)client;
    int len = 0;

    if(c->isconnected)  /* don't send connect packet again if we are already connected */
    {
        goto exit;
    }

    connect_cd_hdl = countdown_start(timeout_ms);

    if(options == 0)
    {
        options = &default_options;    /* set default options if none were supplied */
    }

    c->keepAliveInterval = options->keepAliveInterval;
    c->cleansession = options->cleansession;
    countdown_set(c->last_received, c->keepAliveInterval * 1000);

    if((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
    {
        goto exit;
    }

    if((rc = sendPacket(c, len, connect_cd_hdl)) != SUCCESS)   // send the connect packet
    {
        goto exit;    // there was a problem
    }

    // this will be a blocking call, wait for the connack
    if(waitfor(c, CONNACK, connect_cd_hdl) == CONNACK)
    {
        data->rc = 0;
        data->sessionPresent = 0;

        if(MQTTDeserialize_connack(&data->sessionPresent, &data->rc, c->readbuf,
                                   c->readbuf_size) == 1)
        {
            rc = data->rc;
        }
        else
        {
            log_error("Mqtt connect respond deserialize error!");
            rc = FAILURE;
        }
    }
    else
    {
        log_error("Mqtt connect respond time out!");
        rc = FAILURE;
    }

exit:
    countdown_stop(connect_cd_hdl);

    if(rc == SUCCESS)
    {
        c->isconnected = 1;
        c->ping_outstanding = 0;
    }

    return rc;
}


int mqtt_client_connect(void *client, MQTTPacket_connectData *options, uint32_t timeout_ms)
{
    mqtt_conn_ack_data data;
    return mqtt_client_connect_with_results(client, options, &data, timeout_ms);
}


int mqtt_client_set_message_handler(void *client, const char *topicFilter,
                                    messageHandler messageHandler, void *arg)
{
    mqtt_client *c = (mqtt_client *)client;
    int rc = FAILURE;
    int i = -1;

    /* first check for an existing matching slot */
    for(i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if(c->messageHandlers[i].topicFilter != NULL &&
                strcmp(c->messageHandlers[i].topicFilter, topicFilter) == 0)
        {
            if(messageHandler == NULL)  /* remove existing */
            {
                c->messageHandlers[i].topicFilter = NULL;
                c->messageHandlers[i].fp = NULL;
                c->messageHandlers[i].arg = arg;
            }

            rc = SUCCESS; /* return i when adding new subscription */
            break;
        }
    }

    /* if no existing, look for empty slot (unless we are removing) */
    if(messageHandler != NULL)
    {
        if(rc == FAILURE)
        {
            for(i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
            {
                if(c->messageHandlers[i].topicFilter == NULL)
                {
                    rc = SUCCESS;
                    break;
                }
            }
        }

        if(i < MAX_MESSAGE_HANDLERS)
        {
            c->messageHandlers[i].topicFilter = topicFilter;
            c->messageHandlers[i].fp = messageHandler;
        }
    }

    return rc;
}


int mqtt_client_subscribe_with_results(void *client, const char *topicFilter, enum mqtt_qos_e qos,
                                       messageHandler messageHandler, void *arg, mqtt_sub_ack_data *data,
                                       uint32_t timeout_ms)
{
    mqtt_client *c = (mqtt_client *)client;
    int rc = FAILURE;
    handle_t sub_cd_hdl = 0;
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;

    if(!c->isconnected)
    {
        goto exit;
    }

    sub_cd_hdl = countdown_start(timeout_ms);

    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1,
                                  &topic, (int *)&qos);

    if(len <= 0)
    {
        goto exit;
    }

    if((rc = sendPacket(c, len, sub_cd_hdl)) != SUCCESS)  // send the subscribe packet
    {
        goto exit;    // there was a problem
    }

    if(waitfor(c, SUBACK, sub_cd_hdl) == SUBACK)       // wait for suback
    {
        int count = 0;
        unsigned short mypacketid;
        data->grantedQoS = MQTT_QOS0;

        if(MQTTDeserialize_suback(&mypacketid, 1, &count, (int *)&data->grantedQoS,
                                  c->readbuf, c->readbuf_size) == 1)
        {
            if(data->grantedQoS != 0x80)
            {
                rc = mqtt_client_set_message_handler(c, topicFilter, messageHandler, arg);
            }
        }
    }
    else
    {
        log_error("Mqtt subscribe respond time out!");
        rc = FAILURE;
    }

exit:
    countdown_stop(sub_cd_hdl);

    if(rc == FAILURE)
    {
        MQTTCloseSession(c);
    }

    return rc;
}


int mqtt_client_subscribe(void *c, const char *topicFilter, enum mqtt_qos_e qos,
                          messageHandler messageHandler, void *arg, uint32_t timeout_ms)
{
    mqtt_sub_ack_data data;
    return mqtt_client_subscribe_with_results(c, topicFilter, qos, messageHandler, arg, &data, timeout_ms);
}


int mqtt_client_unsubscribe(void *client, const char *topicFilter, uint32_t timeout_ms)
{
    mqtt_client *c = (mqtt_client *)client;
    int rc = FAILURE;
    handle_t unsub_cd_hdl = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;

    if(!c->isconnected)
    {
        goto exit;
    }

    unsub_cd_hdl = countdown_start(timeout_ms);

    if((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c),
                                        1, &topic)) <= 0)
    {
        goto exit;
    }

    if((rc = sendPacket(c, len,
                        unsub_cd_hdl)) != SUCCESS)  // send the subscribe packet
    {
        goto exit;    // there was a problem
    }

    if(waitfor(c, UNSUBACK, unsub_cd_hdl) == UNSUBACK)
    {
        unsigned short mypacketid;  // should be the same as the packetid above

        if(MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1)
        {
            /* remove the subscription message handler associated with this topic, if there is one */
            mqtt_client_set_message_handler(c, topicFilter, NULL, NULL);
        }
    }
    else
    {
        log_error("Mqtt unsubscribe respond time out!");
        rc = FAILURE;
    }

exit:
    countdown_stop(unsub_cd_hdl);

    if(rc == FAILURE)
    {
        MQTTCloseSession(c);
    }

    return rc;
}


int mqtt_client_publish(void *client, const char *topicName,
                        struct mqtt_message_t *message, uint32_t timeout_ms)
{
    mqtt_client *c = (mqtt_client *)client;
    int rc = FAILURE;
    handle_t pub_cd_hdl = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;

    if(!c->isconnected)
    {
        goto exit;
    }

    pub_cd_hdl = countdown_start(timeout_ms);

    if(message->qos == MQTT_QOS1 || message->qos == MQTT_QOS2)
    {
        message->id = getNextPacketId(c);
    }

    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos,
                                message->retained, message->id,
                                topic, (unsigned char *)message->payload, message->payload_len);

    if(len <= 0)
    {
        goto exit;
    }

    if((rc = sendPacket(c, len,
                        pub_cd_hdl)) != SUCCESS)  // send the subscribe packet
    {
        goto exit;    // there was a problem
    }

    if(message->qos == MQTT_QOS1)
    {
        if(waitfor(c, PUBACK, pub_cd_hdl) == PUBACK)
        {
            unsigned short mypacketid;
            unsigned char dup, type;

            if(MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf,
                                   c->readbuf_size) != 1)
            {
                log_error("Mqtt publish respond deserialize error!");
                rc = FAILURE;
            }
        }
        else
        {
            log_error("Mqtt publish respond time out!");
            rc = FAILURE;
        }
    }
    else if(message->qos == MQTT_QOS2)
    {
        if(waitfor(c, PUBCOMP, pub_cd_hdl) == PUBCOMP)
        {
            unsigned short mypacketid;
            unsigned char dup, type;

            if(MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf,
                                   c->readbuf_size) != 1)
            {
                log_error("Mqtt publish respond deserialize error!");
                rc = FAILURE;
            }
        }
        else
        {
            log_error("Mqtt publish respond time out!");
            rc = FAILURE;
        }
    }

exit:
    countdown_stop(pub_cd_hdl);

    if(rc == FAILURE)
    {
//        MQTTCloseSession(c);
    }

    return rc;
}


int mqtt_client_disconnect(void *client, uint32_t timeout_ms)
{
    mqtt_client *c = (mqtt_client *)client;
    int rc = FAILURE;
    handle_t discon_cd_hdl = 0; // we might wait for incomplete incoming publishes to complete
    int len = 0;

    discon_cd_hdl = countdown_start(timeout_ms);

    len = MQTTSerialize_disconnect(c->buf, c->buf_size);

    if(len > 0)
    {
        rc = sendPacket(c, len, discon_cd_hdl);    // send the disconnect packet
    }

    countdown_stop(discon_cd_hdl);
    MQTTCloseSession(c);

    return rc;
}

int mqtt_client_is_connected(void *client)
{
    return ((mqtt_client *)client)->isconnected;
}


/*****************************************************************************/
#include "config.h"
#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_WOLFSSL
#include "tls.h"
#else
#include "plat_tcp.h"
#endif

/**
 * @brief 连接服务器。
 *
 * @param client MQTT客户端实例操作句柄
 * @return int32_t 返回连接结果（CONN_ACK或其它错误）
 */
void *mqtt_connect(const int8_t *remote_addr, uint16_t remote_port, const int8_t *ca_cert, uint16_t ca_cert_len,
                   struct mqtt_param_t *mqtt_param, uint32_t timeout_ms)
{
    struct mqtt_client *client = NULL;
    struct mqtt_network *net_cb = NULL;
    MQTTPacket_connectData conn_data;
    mqtt_conn_ack_data ack_data;
    handle_t cd_hdl = 0;

    if (NULL == (net_cb = osl_malloc(sizeof(struct mqtt_network))))
    {
        return NULL;
    }
    osl_memset(net_cb, 0, sizeof(struct mqtt_network));

    if (0 == (cd_hdl = countdown_start(timeout_ms)))
    {
        osl_free(net_cb);
        return NULL;
    }

#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_WOLFSSL
    net_cb->handle = tls_connect(remote_addr, remote_port, ca_cert, ca_cert_len, countdown_left(cd_hdl));
    net_cb->mqttread = tls_recv;
    net_cb->mqttwrite = tls_send;
    net_cb->disconnect = tls_disconnect;
#else
    net_cb->handle = tcp_connect(remote_addr, remote_port, countdown_left(cd_hdl));
    net_cb->mqttread = tcp_recv;
    net_cb->mqttwrite = tcp_send;
    net_cb->disconnect = tcp_disconnect;
#endif

    if(-1 == net_cb->handle)
    {
        goto exit;
    }

    client = mqtt_client_init(net_cb, mqtt_param->send_buf, mqtt_param->send_buf_len,
                              mqtt_param->recv_buf, mqtt_param->recv_buf_len);

    if(NULL == client)
    {
        goto exit1;
    }

    osl_memset(&conn_data, 0, sizeof(conn_data));
    osl_memcpy(conn_data.struct_id, "MQTC", 4);
    conn_data.MQTTVersion = 4;
    conn_data.keepAliveInterval = mqtt_param->life_time;
    conn_data.clientID.cstring = (int8_t *)mqtt_param->client_id;

    if(mqtt_param->connect_flag & MQTT_CONNECT_FLAG_USERNAME)
    {
        conn_data.username.cstring = (int8_t *)mqtt_param->username;
    }

    if(mqtt_param->connect_flag & MQTT_CONNECT_FLAG_PASSWORD)
    {
        conn_data.password.cstring = (int8_t *)mqtt_param->password;
    }

    if(mqtt_param->connect_flag & MQTT_CONNECT_FLAG_CLEAN_SESSION)
    {
        conn_data.cleansession = 1;
    }

    if(mqtt_param->connect_flag & MQTT_CONNECT_FLAG_WILL)
    {
        conn_data.willFlag = 1;
        osl_memcpy(conn_data.will.struct_id, "MQTW", 4);
        conn_data.will.topicName.cstring = (int8_t *)mqtt_param->will_msg.will_topic;

        if(mqtt_param->connect_flag & MQTT_CONNECT_FLAG_WILL_RETAIN)
        {
            conn_data.will.retained = 1;
        }

        conn_data.will.message.lenstring.data = mqtt_param->will_msg.will_message;
        conn_data.will.message.lenstring.len = mqtt_param->will_msg.will_message_len;
        conn_data.will.qos = mqtt_param->will_msg.qos;
    }

    if(0 != mqtt_client_connect(client, &conn_data, countdown_left(cd_hdl)))
    {
        log_debug("mqtt connect error\n");
        goto exit2;
    }
    
    log_debug("mqtt connect ok\n");

    countdown_stop(cd_hdl);
    return client;

exit2:
    mqtt_client_deinit(client);
exit1:
    net_cb->disconnect(net_cb->handle);
exit:
    countdown_stop(cd_hdl);
    osl_free(net_cb);
    return NULL;
}

/**
 * @brief MQTT消息处理。
 *
 * @param client MQTT客户端实例操作句柄
 * @param timeout_ms 超时时间
 * @return int32_t
 */
int32_t mqtt_yield(void *client, uint32_t timeout_ms)
{
    if(client)
    {
        return mqtt_client_yield(client, timeout_ms);
    }

    return -1;
}

/**
 * @brief MQTT消息推送。
 *
 * @param client MQTT客户端实例操作句柄
 * @param topic 推送消息的目标topic
 * @param message 需要推送的消息内容
 * @return int32_t 根据QOS级别返回PUBACK或其它错误
 */
int32_t mqtt_publish(void *client, const int8_t *topic, struct mqtt_message_t *message, uint32_t timeout_ms)
{
    struct mqtt_message_t msg;

    if(client)
    {
        return mqtt_client_publish(client, topic, message, timeout_ms);
    }

    return -1;
}

/**
 * @brief 订阅指定topic。
 *
 * @param client MQTT客户端实例操作句柄
 * @param topic 需要订阅的目标topic
 * @param qos 指定订阅的目标topic QoS等级
 * @param msg_handler 指定topic的消息处理回调
 * @return int32_t
 */
int32_t mqtt_subscribe(void *client, const int8_t *topic, enum mqtt_qos_e qos,
                       mqtt_message_handler msg_handler, void *arg, uint32_t timeout_ms)
{
    if(client)
    {
        return mqtt_client_subscribe(client, topic, qos, msg_handler, arg, timeout_ms);
    }

    return -1;
}

/**
 * @brief 取消topic订阅。
 *
 * @param client MQTT客户端实例操作句柄
 * @param topic 需要取消订阅的topic
 * @return int32_t
 */
int32_t mqtt_unsubscribe(void *client, const int8_t *topic, uint32_t timeout_ms)
{
    if(client)
    {
        return mqtt_client_unsubscribe(client, topic, timeout_ms);
    }

    return -1;
}

/**
 * @brief 断开MQTT连接。
 *
 * @param client MQTT客户端实例操作句柄
 * @return int32_t
 */
int32_t mqtt_disconnect(void *client, uint32_t timeout_ms)
{
    mqtt_client *c = (mqtt_client *)client;

    mqtt_client_disconnect(c, timeout_ms);
    if (c->ipstack)
    {
        c->ipstack->disconnect(c->ipstack->handle);
        osl_free(c->ipstack);
    }
    mqtt_client_deinit(c);

    return 0;
}

