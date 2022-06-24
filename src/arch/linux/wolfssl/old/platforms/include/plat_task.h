/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file plat_task.h
 * @brief
 */

#ifndef __PLAT_TASK_H__
#define __PLAT_TASK_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition (Constant and Macro)                                  */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
typedef unsigned long int osl_task_handle_t;

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 创建一个线程
 *
 * @param newthread_handle 线程标识符指针
 * @param attr 线程属性
 * @param func - 线程函数起始地址
 * @param arg - 线程函数参数
 */
int task_create(osl_task_handle_t *newthread_handle, void *attr, void *(*func)(void *),
                void *arg);

#ifdef __cplusplus
}
#endif

#endif
