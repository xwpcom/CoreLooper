/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file dev_token.h
 * @brief
 */

#ifndef __DEV_TOKEN_H__
#define __DEV_TOKEN_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

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
enum sig_method_e
{
    SIG_METHOD_MD5,
    SIG_METHOD_SHA1,
    SIG_METHOD_SHA256
};

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief 生成OneNET登录鉴权Token
 * 
 * @param token 用于存放生成的鉴权token的数据缓冲区地址
 * @param method 指定token计算过程的加密算法
 * @param exp_time Unix时间戳的形式指定token到期时间
 * @param product_id 设备所属的产品ID
 * @param dev_name 设备唯一标识
 * @param access_key 设备所属产品的唯一访问密钥
 * @retval 0 - token生成成功
 */
int32_t dev_token_generate(int8_t *token, enum sig_method_e method, uint32_t exp_time, const int8_t *product_id,
                           const int8_t *dev_name, const int8_t *access_key);

#ifdef __cplusplus
}
#endif

#endif
