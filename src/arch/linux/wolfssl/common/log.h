/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        log.h
 * @brief       统一日志输出接口。
 */

#ifndef __LOG_H__
#define __LOG_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/* External Definitionï¼ˆConstant and Macro )                                 */
/*****************************************************************************/
#define LOG_LEVEL LOG_LEVEL_DEBUG

/**
 * @brief 日志等级 - 异常
 *
 */
#define LOG_LEVEL_ERROR 0

/**
 * @brief 日志等级 - 消息
 *
 */
#define LOG_LEVEL_INFO  1

/**
 * @brief 日志等级 - 调试信息
 *
 */
#define LOG_LEVEL_DEBUG 2

#ifdef DEBUG
#define log_error(mod, ...) log_output(mod, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__);
#define log_info(mod, ...) log_output(mod, __FUNCTION__, __LINE__, LOG_LEVEL_INFO, __VA_ARGS__);
#define log_debug(mod, ...) log_output(mod, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__);
#else
#define log_error(fmt, ...) log_output("", __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, fmt,##__VA_ARGS__);
#define log_info(fmt, ...) log_output("", __FUNCTION__, __LINE__, LOG_LEVEL_INFO, fmt,##__VA_ARGS__);
#define log_debug(fmt, ...) log_output("", __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, fmt,##__VA_ARGS__);
#endif

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
void log_output(const int8_t *module, const int8_t *function, const uint16_t line, 
                const int8_t level, const int8_t *fmt, ...);


#ifdef __cplusplus
}
#endif

#endif

