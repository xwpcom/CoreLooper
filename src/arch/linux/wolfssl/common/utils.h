/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        utils.h
 * @brief       一些常用数据处理函数封装。
 */

#ifndef __UTILS_H__
#define __UTILS_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External DefinitionÃ¯Â¼Ë†Constant and Macro )                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
uint16_t set_16bit_le(uint8_t *buf, uint16_t val);
uint16_t get_16bit_le(uint8_t *buf, uint16_t *val);

uint16_t set_16bit_be(uint8_t *buf, uint16_t val);
uint16_t get_16bit_be(uint8_t *buf, uint16_t *val);

uint16_t set_32bit_le(uint8_t *buf, uint32_t val);
uint16_t get_32bit_le(uint8_t *buf, uint32_t *val);

uint16_t set_32bit_be(uint8_t *buf, uint32_t val);
uint16_t get_32bit_be(uint8_t *buf, uint32_t *val);

int32_t str_to_hex(uint8_t *data_str, uint8_t *data_hex, uint16_t len);
void hex_to_str(uint8_t *data_str, uint8_t *data_hex, uint16_t Len);


#ifdef __cplusplus
}
#endif

#endif

