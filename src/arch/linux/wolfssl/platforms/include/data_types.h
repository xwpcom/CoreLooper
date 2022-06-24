/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 * 
 * @file data_types.h
 * @brief 数据类型定义。
 */

#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "config.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/
#ifndef boolean
typedef unsigned char boolean;
#define TRUE 1
#define FALSE 0
#endif

#ifndef PLAT_HAS_STDINT
typedef char                int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;
typedef long long           int64_t;
typedef unsigned long long  uint64_t;
#else
#include <stdint.h>
#endif

typedef float               float32_t;
typedef double              float64_t;
typedef long                handle_t;
typedef unsigned int        ptr_t;

#ifndef NULL
#define NULL 0
#endif

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/


#ifdef __cplusplus
}
#endif

#endif
