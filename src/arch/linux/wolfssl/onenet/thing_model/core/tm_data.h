/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_data.h
 * @brief 数据是物模型设备具有的可以细分的属性、服务，分为Property(属性)、Event(事件)、Service(服务)。
 */

#ifndef _TM_DATA_H_
#define _TM_DATA_H_

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
typedef int32_t (*tm_list_cb)(const int8_t * /**data_name*/, void * /** data*/);

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 创建一个数据实例
 *
 * @return void* 数据实例句柄
 */
void *tm_data_create();

/**
 * @brief 创建一个结构体数据实例
 *
 * @return
 */
void *tm_data_struct_create(void);

/**
 * @brief 创建一个数组数据实例
 *
 * @param size 指定数组元素个数
 * @return
 */
void *tm_data_array_create(uint32_t size);
int32_t tm_data_array_set_bool(void *array, boolean val);
int32_t tm_data_array_set_enum(void *array, int32_t val);
int32_t tm_data_array_set_int32(void *array, int32_t val);
int32_t tm_data_array_set_int64(void *array, int64_t val);
int32_t tm_data_array_set_date(void *array, int64_t val);
int32_t tm_data_array_set_float(void *array, float32_t val);
int32_t tm_data_array_set_double(void *array, float64_t val);
int32_t tm_data_array_set_string(void *array, int8_t *val);
int32_t tm_data_array_set_bitmap(void *array, int32_t val);
int32_t tm_data_array_set_struct(void *array, void *val);

/**
 * @brief 销毁创建的数据实例
 *
 * @param res 需要销毁的数据实例
 */
void tm_data_delete(void *data);

/**
 * @brief 向数据实例中添加一个布尔类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_bool(void *data, const int8_t *name, boolean val, uint64_t timestamp);
int32_t tm_data_get_bool(void *data, boolean *val);

/**
 * @brief 向数据实例中添加一个枚举类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_enum(void *data, const int8_t *name, int32_t val, uint64_t timestamp);
int32_t tm_data_get_enum(void *data, int32_t *val);

/**
 * @brief 向数据实例中添加一个32位整数类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_int32(void *data, const int8_t *name, int32_t val, uint64_t timestamp);
int32_t tm_data_get_int32(void *data, int32_t *val);

/**
 * @brief 向数据实例中添加一个64位整数类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_int64(void *data, const int8_t *name, int64_t val, uint64_t timestamp);
int32_t tm_data_get_int64(void *data, int64_t *val);

/**
 * @brief 向数据实例中添加一个时间类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值，UTC时间戳，单位ms
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_date(void *data, const int8_t *name, int64_t val, uint64_t timestamp);
int32_t tm_data_get_date(void *data, int64_t *val);

/**
 * @brief 向数据实例中添加一个的单精度浮点类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_float(void *data, const int8_t *name, float32_t val, uint64_t timestamp);
int32_t tm_data_get_float(void *data, float32_t *val);

/**
 * @brief 向数据实例中添加一个双精度浮点类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_double(void *data, const int8_t *name, float64_t val, uint64_t timestamp);
int32_t tm_data_get_double(void *data, float64_t *val);

/**
 * @brief 向数据实例中添加一个位图类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_bitmap(void *data, const int8_t *name, uint32_t val, uint64_t timestamp);
int32_t tm_data_get_bitmap(void *data, uint32_t *val);

/**
 * @brief 向数据实例中添加一个字符串类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值，需要以'\0'结尾
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_string(void *data, const int8_t *name, int8_t *val, uint64_t timestamp);
int32_t tm_data_get_string(void *data, int8_t **val);

/**
 * @brief 向数据实例中添加一个结构体或数组类型的数据
 *
 * @param res 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 由tm_data_struct_create或者tm_data_array_create创建的结构体数据
 * @param timestamp 指定数据时间戳，为0则无效
 * @return int32_t
 */
int32_t tm_data_set_data(void *data, const int8_t *name, void *val, uint64_t timestamp);
int32_t tm_data_get_data(void *data, const int8_t *name, void **val);

int32_t tm_data_set_struct(void *data, const int8_t *name, void *val, uint64_t timestamp);

int32_t tm_data_set_array(void *data, const int8_t *name, void *val, uint64_t timestamp);

/**
 * @brief 向结构体数据实例中添加一个布尔类型的数据
 *
 * @param structure 指定结构体数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @return int32_t
 */
int32_t tm_data_struct_set_bool(void *structure, const int8_t *name, boolean val);
int32_t tm_data_struct_get_bool(void *structure, const int8_t *name, boolean *val);

/**
 * @brief 向结构体数据实例中添加一个枚举类型的数据
 *
 * @param structure 指定结构体数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @return int32_t
 */
int32_t tm_data_struct_set_enum(void *structure, const int8_t *name, int32_t val);
int32_t tm_data_struct_get_enum(void *structure, const int8_t *name, int32_t *val);

/**
 * @brief 向结构体数据实例中添加一个32位整数类型的数据
 *
 * @param structure 指定结构体数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @return int32_t
 */
int32_t tm_data_struct_set_int32(void *structure, const int8_t *name, int32_t val);
int32_t tm_data_struct_get_int32(void *structure, const int8_t *name, int32_t *val);

/**
 * @brief 向结构体数据实例中添加一个64位整数类型的数据
 *
 * @param structure 指定结构体数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @return int32_t
 */
int32_t tm_data_struct_set_int64(void *structure, const int8_t *name, int64_t val);
int32_t tm_data_struct_get_int64(void *structure, const int8_t *name, int64_t *val);

/**
 * @brief 向结构体数据实例中添加一个时间类型的数据
 *
 * @param structure 指定数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值，UTC时间戳，单位ms
 * @return int32_t
 */
int32_t tm_data_struct_set_date(void *structure, const int8_t *name, int64_t val);
int32_t tm_data_struct_get_date(void *structure, const int8_t *name, int64_t *val);

/**
 * @brief 向结构体数据实例中添加一个的单精度浮点数类型的数据
 *
 * @param structure 指定结构体数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @return int32_t
 */
int32_t tm_data_struct_set_float(void *structure, const int8_t *name, float32_t val);
int32_t tm_data_struct_get_float(void *structure, const int8_t *name, float32_t *val);

/**
 * @brief 向结构体数据实例中添加一个双精度浮点数类型的数据
 *
 * @param structure 指定结构体数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @return int32_t
 */
int32_t tm_data_struct_set_double(void *structure, const int8_t *name, float64_t val);
int32_t tm_data_struct_get_double(void *structure, const int8_t *name, float64_t *val);

/**
 * @brief 向结构体数据实例中添加一个位图类型的数据
 *
 * @param structure 指定结构体数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @return int32_t
 */
int32_t tm_data_struct_set_bitmap(void *structure, const int8_t *name, uint32_t val);
int32_t tm_data_struct_get_bitmap(void *structure, const int8_t *name, uint32_t *val);

/**
 * @brief 向结构体数据实例中添加一个字符串类型的数据
 *
 * @param structure 指定结构体数据实例
 * @param name 指定需要添加的数据标识
 * @param val 指定数据值
 * @return int32_t
 */
int32_t tm_data_struct_set_string(void *structure, const int8_t *name, int8_t *val);
int32_t tm_data_struct_get_string(void *structure, const int8_t *name, int8_t **val);

int32_t tm_data_struct_set_data(void *structure, const int8_t *name, void *val);
int32_t tm_data_struct_get_data(void *structure, const int8_t *name, void **val);

int32_t tm_data_list_each(void *data, tm_list_cb callback);

int32_t tm_data_array_size(void *array);
void *tm_data_array_get_element(void *array, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
