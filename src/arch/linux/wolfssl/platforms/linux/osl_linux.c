/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file osl_linux.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "plat_osl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

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
void *osl_malloc(size_t size)
{
    return malloc(size);
}

void osl_free(void *ptr)
{
    free(ptr);
}

void *osl_memcpy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

void *osl_memset(void *dst, int32_t val, size_t n)
{
    return memset(dst, val, n);
}

int8_t *osl_strdup(const int8_t *s)
{
    return strdup(s);
}

int8_t *osl_strndup(const int8_t *s, size_t n)
{
    return strndup(s, n);
}

int8_t *osl_strcpy(int8_t *s1, const int8_t *s2)
{
    return strcpy(s1, s2);
}

uint32_t osl_strlen(const int8_t *s)
{
    return strlen(s);
}

int8_t *osl_strcat(int8_t *dst, const int8_t *src)
{
    return strcat(dst, src);
}

int32_t osl_strcmp(const int8_t *s1, const int8_t *s2)
{
    return strcmp(s1, s2);
}

int8_t *osl_strstr(const int8_t *s1, const int8_t *s2)
{
    return strstr(s1, s2);
}

int32_t osl_sprintf(int8_t *str, const int8_t *format, ...)
{
    va_list ap;
    int ret = 0;

    va_start(ap, format);
    ret = vsprintf(str, format, ap);
    va_end(ap);

    return ret;
}

void osl_assert(bool expression)
{
    assert(expression);
}

void *osl_malloc_with_zero(size_t size)
{
    void *ptr = osl_malloc(size);
    if(ptr != NULL)
    {
        osl_memset(ptr, 0, size);
    }
    return ptr;
}

void osl_sleep_ms(uint32_t ms)
{
    struct timespec request = {0, 0};
    request.tv_sec          = ms / 1000;
    request.tv_nsec         = (ms % 1000) * 1000000;
    nanosleep(&request, NULL);
}

int32_t osl_get_random(unsigned char* buf, size_t len)
{
    int i;
    srand((unsigned)time(NULL));
    for (i = 0; i < len; i++)
    {
        buf[i] = (rand() & 0xFF);
    }
    return 0;
}

int32_t osl_atoi(const int8_t *ptr)
{
    return atoi(ptr);
}
