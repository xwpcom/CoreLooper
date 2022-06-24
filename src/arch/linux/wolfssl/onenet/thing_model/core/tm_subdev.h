/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_subdev.h
 * @brief
 */

#ifndef __TM_SUBDEV_H__
#define __TM_SUBDEV_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "config.h"
#include "data_types.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef FEATURE_TM_GATEWAY_ENABLED

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
struct tm_subdev_cbs
{
    int32_t (*subdev_props_get)(const int8_t *product_id, const int8_t *dev_name, const int8_t *props_list,
                                int8_t **props_data);
    int32_t (*subdev_props_set)(const int8_t *product_id, const int8_t *dev_name, int8_t *props_data);
    int32_t (*subdev_service_invoke)(const int8_t *product_id, const int8_t *dev_name, const int8_t *svc_id,
                                     int8_t *in_data, int8_t **out_data);
    int32_t (*subdev_topo)(int8_t *topo_data);
};

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
int32_t tm_subdev_init(struct tm_subdev_cbs callbacks);
int32_t tm_subdev_add(const int8_t *product_id, const int8_t *dev_name, const int8_t *access_key,
                              uint32_t timeout_ms);
int32_t tm_subdev_delete(const int8_t *product_id, const int8_t *dev_name, const int8_t *access_key,
                                 uint32_t timeout_ms);
int32_t tm_subdev_topo_get(uint32_t timeout_ms);
int32_t tm_subdev_login(const int8_t *product_id, const int8_t *dev_name, uint32_t timeout_ms);
int32_t tm_subdev_logout(const int8_t *product_id, const int8_t *dev_name, uint32_t timeout_ms);

int32_t tm_subdev_post_data(const int8_t *product_id, const int8_t *dev_name, int8_t *prop_json,
                                    int8_t *event_json, uint32_t timeout_ms);

#endif
#ifdef __cplusplus
}
#endif

#endif
