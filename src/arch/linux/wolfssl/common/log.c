/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 * 
 * @file log.c
 * @brief 
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "log.h"
#include "plat_osl.h"

#include <stdarg.h>
#include <stdio.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define LOG_BUF_MAX_LEN 2048
/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
char log_buf[LOG_BUF_MAX_LEN] = {0};

int8_t* log_level_str[] = {
    "ERROR",
    "INFO",
    "DEBUG"
};

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
void log_output(const int8_t *module, const int8_t *function, const uint16_t line,
                const int8_t level, const int8_t *fmt, ...)
{
    va_list ap;
    uint16_t offset = 0;

    if((level > LOG_LEVEL) || !module)
        return;

    osl_memset(log_buf, 0, sizeof(log_buf));
    sprintf(log_buf, "[%s][%s]%s L%d: ", log_level_str[level], module, function, line);
    offset = osl_strlen(log_buf);
    va_start(ap, fmt);
    vsnprintf(log_buf + offset, LOG_BUF_MAX_LEN - offset, fmt, ap);
    va_end(ap);
    offset = osl_strlen(log_buf);
    if(log_buf[offset - 1] != '\n')
        log_buf[offset] = '\n';
    printf("%s", log_buf);
}
