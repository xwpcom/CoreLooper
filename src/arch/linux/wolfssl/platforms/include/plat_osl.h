/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file plat_osl.h
 * @brief 标准库通用接口封装。
 */

#ifndef __PLAT_OSL_H__
#define __PLAT_OSL_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition（Constant and Macro )                                 */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
void *osl_malloc(size_t size);
void osl_free(void *ptr);
void *osl_memcpy(void *dst, const void *src, size_t n);
void *osl_memset(void *dst, int32_t val, size_t n);
int8_t *osl_strdup(const int8_t *s);
int8_t *osl_strndup(const int8_t *s, size_t n);

int8_t *osl_strcpy(int8_t *s1, const int8_t *s2);
uint32_t osl_strlen(const int8_t *s);
int8_t *osl_strcat(int8_t *dst, const int8_t *src);
int32_t osl_strcmp(const int8_t *s1, const int8_t *s2);
int8_t *osl_strstr(const int8_t *s1, const int8_t *s2);
int32_t osl_sprintf(int8_t *str, const int8_t *format, ...);
void osl_assert(bool expression);
void *osl_malloc_with_zero(size_t size);
void osl_sleep_ms(uint32_t ms);
int32_t osl_get_random(unsigned char* buf, size_t len);

int32_t osl_atoi(const int8_t *ptr);

#ifdef __cplusplus
}
#endif

#endif
