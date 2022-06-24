/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_subdev.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_subdev.h"
#include "tm_onejson.h"
#include "tm_api.h"

#include "dev_token.h"

#include "plat_osl.h"
#include "err_def.h"

#include "cJSON.h"

#ifdef FEATURE_TM_GATEWAY_ENABLED

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define TM_TOPIC_SUBDEV_LOGIN          "/sub/login"
#define TM_TOPIC_SUBDEV_LOGOUT         "/sub/logout"
#define TM_TOPIC_SUBDEV_ADD            "/sub/topo/add"
#define TM_TOPIC_SUBDEV_DELETE         "/sub/topo/delete"
#define TM_TOPIC_SUBDEV_TOPO_GET       "/sub/topo/get"
#define TM_TOPIC_SUBDEV_TOPO_CHANGE    "/sub/topo/change"
#define TM_TOPIC_SUBDEV_SERVICE_INVOKE "/sub/service/invoke"
#define TM_TOPIC_SUBDEV_PROP_GET       "/sub/property/get"
#define TM_TOPIC_SUBDEV_PROP_SET       "/sub/property/set"

#define TM_TOPIC_SUBDEV_LOGIN_REPLY          "/sub/login/reply"
#define TM_TOPIC_SUBDEV_LOGOUT_REPLY         "/sub/logout/reply"
#define TM_TOPIC_SUBDEV_ADD_REPLY            "/sub/topo/add/reply"
#define TM_TOPIC_SUBDEV_DELETE_REPLY         "/sub/topo/delete/reply"
#define TM_TOPIC_SUBDEV_TOPO_GET_REPLY       "/sub/topo/get/reply"
#define TM_TOPIC_SUBDEV_TOPO_GET_REPLY_RESULT "/sub/topo/get/result"
#define TM_TOPIC_SUBDEV_TOPO_CHANGE_REPLY    "/sub/topo/change_reply"
#define TM_TOPIC_SUBDEV_SERVICE_INVOKE_REPLY "/sub/service/invoke_reply"
#define TM_TOPIC_SUBDEV_PROP_GET_REPLY       "/sub/property/get_reply"
#define TM_TOPIC_SUBDEV_PROP_SET_REPLY       "/sub/property/set_reply"

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
struct tm_subdev_cbs subdev_callbacks;

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
static int32_t subdev_data_callback(const int8_t *name, void *data, uint32_t data_len)
{
    int32_t ret = ERR_OTHERS;

    if(NULL != osl_strstr(name, TM_TOPIC_SUBDEV_TOPO_CHANGE))
    {
        int8_t id[16] = {0};
        void *params = tm_onejson_parse_request(data, data_len, id, 0);

        if(params)
        {
            cJSON *sublist = cJSON_GetObjectItem((cJSON *)params, "subList");
            void *topo = cJSON_PrintUnformatted(sublist);
            if(subdev_callbacks.subdev_topo)
            {
                ret = subdev_callbacks.subdev_topo(topo);
            }
            osl_free(topo);
            cJSON_Delete(params);
        }
        if(ERR_OK == ret)
        {
            tm_send_response(TM_TOPIC_SUBDEV_TOPO_CHANGE_REPLY, id, 200, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
        else
        {
            tm_send_response(TM_TOPIC_SUBDEV_TOPO_CHANGE_REPLY, id, 100, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
    }
    else if(NULL != osl_strstr(name, TM_TOPIC_SUBDEV_TOPO_GET_REPLY))
    {
        int8_t id[16] = {0};
        int32_t msg_code = 0;

        void *topo_data = tm_onejson_parse_reply(data, data_len, id, &msg_code, 1);

        if(topo_data)
        {
            if(subdev_callbacks.subdev_topo)
            {
                ret = subdev_callbacks.subdev_topo(topo_data);
            }
            osl_free(topo_data);
        }
        if(ERR_OK == ret)
        {
            tm_send_response(TM_TOPIC_SUBDEV_TOPO_GET_REPLY_RESULT, id, 200, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
        else
        {
            tm_send_response(TM_TOPIC_SUBDEV_TOPO_GET_REPLY_RESULT, id, 100, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
    }
    else if(NULL != osl_strstr(name, TM_TOPIC_SUBDEV_SERVICE_INVOKE))
    {
        int8_t id[16] = {0};
        void *service_data = tm_onejson_parse_request(data, data_len, id, 0);
        int8_t *product_id = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON *)service_data, "productID"));
        int8_t *dev_name = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON *)service_data, "deviceName"));
        int8_t *svc_id = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON *)service_data, "identifier"));
        cJSON *input = cJSON_DetachItemFromObject((cJSON *)service_data, "input");
        int8_t *input_data = cJSON_PrintUnformatted(input);
        int8_t *output_data = NULL;

        ret = subdev_callbacks.subdev_service_invoke(product_id, dev_name, svc_id, input_data, &output_data);
        cJSON_AddRawToObject((cJSON *)service_data, "output", output_data);
        cJSON_Delete(input);
        osl_free(input_data);
        if(ERR_OK == ret)
        {
            tm_send_response(TM_TOPIC_SUBDEV_SERVICE_INVOKE_REPLY, id, 200, 0, service_data, osl_strlen(output_data),
                             FEATURE_TM_REPLY_TIMEOUT);
        }
        else
        {
            cJSON_Delete((cJSON *)service_data);
            tm_send_response(TM_TOPIC_SUBDEV_SERVICE_INVOKE_REPLY, id, 100, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
        if(output_data)
        {
            osl_free(output_data);
        }
    }
    else if(NULL != osl_strstr(name, TM_TOPIC_SUBDEV_PROP_GET))
    {
        int8_t id[16] = {0};
        void *prop_list_data = tm_onejson_parse_request(data, data_len, id, 0);
        int8_t *product_id = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON *)prop_list_data, "productID"));
        int8_t *dev_name = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON *)prop_list_data, "deviceName"));
        cJSON *params = cJSON_GetObjectItem((cJSON *)prop_list_data, "params");
        int8_t *prop_list = cJSON_PrintUnformatted(params);
        int8_t *prop_data = NULL;

        ret = subdev_callbacks.subdev_props_get(product_id, dev_name, prop_list, &prop_data);
        cJSON_Delete(prop_list_data);
        osl_free(prop_list);
        if(ERR_OK == ret)
        {
            tm_send_response(TM_TOPIC_SUBDEV_PROP_GET_REPLY, id, 200, 1, prop_data, osl_strlen(prop_data),
                             FEATURE_TM_REPLY_TIMEOUT);
        }
        else
        {
            tm_send_response(TM_TOPIC_SUBDEV_PROP_GET_REPLY, id, 100, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
        if(prop_data)
        {
            osl_free(prop_data);
        }
    }
    else if(NULL != osl_strstr(name, TM_TOPIC_SUBDEV_PROP_SET))
    {
        int8_t id[16] = {0};
        void *prop_set_data = tm_onejson_parse_request(data, data_len, id, 0);
        int8_t *product_id = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON *)prop_set_data, "productID"));
        int8_t *dev_name = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON *)prop_set_data, "deviceName"));
        cJSON *params = cJSON_GetObjectItem((cJSON *)prop_set_data, "params");
        int8_t *prop_data = cJSON_PrintUnformatted(params);

        ret = subdev_callbacks.subdev_props_set(product_id, dev_name, prop_data);
        cJSON_Delete(prop_set_data);
        osl_free(prop_data);
        if(ERR_OK == ret)
        {
            tm_send_response(TM_TOPIC_SUBDEV_PROP_SET_REPLY, id, 200, 1, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
        else
        {
            tm_send_response(TM_TOPIC_SUBDEV_PROP_GET_REPLY, id, 100, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
    }
}

int32_t tm_subdev_init(struct tm_subdev_cbs callbacks)
{
    osl_memcpy(&subdev_callbacks, &callbacks, sizeof(callbacks));
    tm_set_subdev_callback(subdev_data_callback);

    return ERR_OK;
}

static int32_t subdev_topo_add_delete(const int8_t *action, const int8_t *product_id, const int8_t *dev_name,
                                      const int8_t *access_key, uint32_t timeout_ms)
{
    int8_t dev_token[256] = {0};
    cJSON *data = cJSON_CreateObject();
    int8_t *raw_data = NULL;
    int32_t ret = ERR_OK;

    if(NULL == data)
    {
        return ERR_NO_MEM;
    }

    dev_token_generate(dev_token, SIG_METHOD_SHA1, 2524579200, product_id, dev_name, access_key);
    cJSON_AddStringToObject(data, "productID", product_id);
    cJSON_AddStringToObject(data, "deviceName", dev_name);
    cJSON_AddStringToObject(data, "sasToken", dev_token);
    raw_data = cJSON_PrintUnformatted(data);
    cJSON_Delete(data);
    ret = tm_post_raw(action, raw_data, osl_strlen(raw_data), NULL, 0, timeout_ms);
    osl_free(raw_data);

    return ret;
}

int32_t tm_subdev_add(const int8_t *product_id, const int8_t *dev_name, const int8_t *access_key, uint32_t timeout_ms)
{
    return subdev_topo_add_delete(TM_TOPIC_SUBDEV_ADD, product_id, dev_name, access_key, timeout_ms);
}

int32_t tm_subdev_delete(const int8_t *product_id, const int8_t *dev_name, const int8_t *access_key,
                         uint32_t timeout_ms)
{
    return subdev_topo_add_delete(TM_TOPIC_SUBDEV_DELETE, product_id, dev_name, access_key, timeout_ms);
}

int32_t tm_subdev_topo_get(uint32_t timeout_ms)
{
    void *topo_data = NULL;
    uint32_t topo_data_len = 0;

    return tm_post_raw(TM_TOPIC_SUBDEV_TOPO_GET, NULL, 0, NULL, 0, timeout_ms);
}

static int32_t subdev_login_logout(const int8_t *action, const int8_t *product_id, const int8_t *dev_name,
                                   uint32_t timeout_ms)
{
    cJSON *data = cJSON_CreateObject();
    int8_t *raw_data = NULL;
    int32_t ret = ERR_OK;

    if(NULL == data)
    {
        return ERR_NO_MEM;
    }

    cJSON_AddStringToObject(data, "productID", product_id);
    cJSON_AddStringToObject(data, "deviceName", dev_name);
    raw_data = cJSON_PrintUnformatted(data);
    cJSON_Delete(data);
    ret = tm_post_raw(action, raw_data, osl_strlen(raw_data), NULL, 0, timeout_ms);
    osl_free(raw_data);

    return ret;
}

int32_t tm_subdev_login(const int8_t *product_id, const int8_t *dev_name, uint32_t timeout_ms)
{
    return subdev_login_logout(TM_TOPIC_SUBDEV_LOGIN, product_id, dev_name, timeout_ms);
}

int32_t tm_subdev_logout(const int8_t *product_id, const int8_t *dev_name, uint32_t timeout_ms)
{
    return subdev_login_logout(TM_TOPIC_SUBDEV_LOGOUT, product_id, dev_name, timeout_ms);
}

int32_t tm_subdev_post_data(const int8_t *product_id, const int8_t *dev_name, int8_t *prop_json, int8_t *event_json,
                            uint32_t timeout_ms)
{
    void *data = tm_pack_device_data(NULL, product_id, dev_name, prop_json, event_json, 1);

    tm_post_pack_data(data, 3000);

    return ERR_OK;
}

#endif
