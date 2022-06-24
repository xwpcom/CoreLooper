/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_onejson.h
 * @brief
 */

#ifndef __TM_ONEJSON_H__
#define __TM_ONEJSON_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#include "tm_data.h"
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/
#define TM_ONEJSON_PAYLOAD_TYPE_REQUEST     0
#define TM_ONEJSON_PAYLOAD_TYPE_REPLY       1

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
void *tm_onejson_create_data(void);
void *tm_onejson_create_array(uint32_t size);
void *tm_onejson_create_struct(void);
void tm_onejson_delete_data(void *data);
int32_t tm_onejson_pack_bool_with_timestamp(void *data, const int8_t *name, boolean val, int64_t ts_in_ms);
int32_t tm_onejson_pack_number_with_timestamp(void *data, const int8_t *name, float64_t val, int64_t ts_in_ms);
int32_t tm_onejson_pack_float32_with_timestamp(void *data, const int8_t *name, float32_t val, int64_t ts_in_ms);
int32_t tm_onejson_pack_string_with_timestamp(void *data, const int8_t *name, int8_t *val, int64_t ts_in_ms);
int32_t tm_onejson_pack_struct_with_timestamp(void *data, const int8_t *name, void *val, int64_t ts_in_ms);

int32_t tm_onejson_pack_bool(void *data, const int8_t *name, boolean val);
int32_t tm_onejson_pack_number(void *data, const int8_t *name, float64_t val);
int32_t tm_onejson_pack_float32(void *data, const int8_t *name, float32_t val);
int32_t tm_onejson_pack_string(void *data, const int8_t *name, int8_t *val);
int32_t tm_onejson_pack_struct(void *data, const int8_t *name, void *val);

int32_t tm_onejson_get_array_size(void *array);
void *tm_onejson_get_array_element_by_index(void *data, uint32_t index);
void *tm_onejson_get_data_by_name(void *data, const int8_t *name);
int32_t tm_onejson_list_each(void *data, tm_list_cb callback);

int32_t tm_onejson_parse_bool(void *data, boolean *val);
int32_t tm_onejson_parse_number(void *data, float64_t *val);
int32_t tm_onejson_parse_string(void *data, int8_t **val);

void *tm_onejson_pack_props_and_events(void *data, const int8_t *product_id, const int8_t *dev_name, void *props, void *events, uint8_t as_raw);

uint32_t tm_onejson_pack_request(uint8_t *payload, int32_t msg_id, void *params, uint8_t as_raw);
void *tm_onejson_parse_request(uint8_t *payload, uint32_t payload_len, int8_t *msg_id, uint8_t as_raw);
uint32_t tm_onejson_pack_reply(uint8_t *payload, int8_t *msg_id, int32_t msg_code, void *data, uint8_t as_raw);
void *tm_onejson_parse_reply(uint8_t *payload, uint32_t payload_len, int8_t *msg_id, int32_t *msg_code, uint8_t as_raw);

#ifdef __cplusplus
}
#endif

#endif
