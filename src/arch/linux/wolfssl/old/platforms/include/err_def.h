/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 * 
 * @file err_def.h
 * @brief 错误码定义。
 */

#ifndef __ERR_DEF_H__
#define __ERR_DEF_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition（Constant and Macro )                                 */
/*****************************************************************************/
/**
 * @name 操作成功
 * @{
 */
/** 成功*/
#define ERR_OK 0
/** @} */

/**
 * @name 系统错误
 * @{
 */
/** 内存不足*/
#define ERR_NO_MEM -1
/** 接口参数错误*/
#define ERR_INVALID_PARAM -2
/** 数据错误*/
#define ERR_INVALID_DATA -3
/** 资源忙*/
#define ERR_RESOURCE_BUSY -4
/** 设备未初始化*/
#define ERR_UNINITIALIZED -5
/** 设备初始化未完成*/
#define ERR_INITIALIZING -6
/** 重复操作*/
#define ERR_REPETITIVE -7
/** IO/通信错误*/
#define ERR_IO -8
/** 功能不支持*/
#define ERR_NOT_SUPPORT -9
/** 操作超时*/
#define ERR_TIMEOUT -10
/** 其他错误*/
#define ERR_OTHERS -11
/** @} */

/**
 * @name 网络错误
 * @{
 */
/** 模组通信超时*/
#define ERR_MOD_TIMEOUT -100
/** 功能不支持*/
#define ERR_MOD_NOT_SUPPORT -101
/** 功能参数错误*/
#define ERR_MOD_INVALID_PARAM -102
/** 设备不在线*/
#define ERR_MOD_OFFLINE -105
/** 模组忙*/
#define ERR_MOD_BUSY -106
/** @} */
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
