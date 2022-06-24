/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file plat_time.h
 * @brief
 */

#ifndef __PLAT_TIME_H__
#define __PLAT_TIME_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 时间计时，单位毫秒
 *
 * @return uint64_t 返回当前毫秒级计数
 */
uint64_t time_count_ms(void);

/**
 * @brief 时间计时，单位秒
 *
 * @return uint64_t 返回当前秒级计数
 */
uint64_t time_count(void);

/**
 * @brief 延迟指定时间，单位毫秒
 *
 * @param m_sec
 */
void time_delay_ms(uint32_t m_sec);

/**
 * @brief 延迟指定时间，单位秒
 *
 * @param sec
 */
void time_delay(uint32_t sec);

/**
 * @brief 启动倒计时
 *
 * @param ms 设置倒计时时间，单位ms
 * @retval  0 - 失败
 * @retval 其它 - 成功，返回倒计时器操作句柄   
 */
handle_t countdown_start(uint32_t ms);

/**
 * @brief 重新设置倒计时超时时间
 *
 * @param handle 倒计时器操作句柄
 * @param ms 新的倒计时器超时时间
 */
void countdown_set(handle_t handle, uint32_t new_ms);

/**
 * @brief 返回倒计时器剩余时间
 *
 * @param handle 倒计时器操作句柄
 * @return  倒计时器剩余时间
 */
uint32_t countdown_left(handle_t handle);

/**
 * @brief 判断倒计时器是否已超时
 *
 * @param handle 倒计时器操作句柄
 * @return  0 - 未超时
 *          1 - 已超时
 */
uint32_t countdown_is_expired(handle_t handle);

/**
 * @brief 停止倒计时器，销毁资源
 *
 * @param handle 倒计时器操作句柄
 */
void countdown_stop(handle_t handle);


#ifdef __cplusplus
}
#endif

#endif
