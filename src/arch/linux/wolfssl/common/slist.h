/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        slist.h
 * @brief       单向链表，使用方式与双向链表相同。
 */

#ifndef __SLIST_H__
#define __SLIST_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "plat_osl.h"

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
/**
 * @brief 单向链表节点定义，使用方式与双向链表相同。
 * 
 */
struct slist_node
{
    /** 指向下一个数据节点，当前节点为末节点时为NULL*/
    struct slist_node *next;
};

/**
 * @brief 单向链表头定义。
 * 
 */
struct slist_head
{
    /** 指向链表的首节点，链表为空时为NULL*/
    struct slist_node *head;
    /** 指向链表的末节点，链表为空时为NULL*/
    struct slist_node *tail;
    /** 当前链表中的节点数量*/
    uint32_t cnt;
};
/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * 初始化单向链表表头
 * @param head 单向链表头结构
 * @retval  0 - 成功
 * @retval -1 - 失败
 */
int32_t slist_init(struct slist_head *head);

/**
 * 查询指定链表中节点数量
 * @param head 单向链表头结构
 * @retval  -1 - 错误
 * @retval >=0 - 节点数量
 */
uint32_t slist_get_cnt(struct slist_head *head);

/**
 * 在指定节点之后插入新节点
 * @param head 单向链表头
 * @param current 指定需要插入节点的位置
 * @param node 指定需要插入的新节点
 * @retval  0 - 插入成功
 * @retval -1 - 插入失败
 */
int32_t slist_insert_after(struct slist_head *head, struct slist_node *current,
                           struct slist_node *node);

/**
 * 在链表头插入新节点
 * @param head 单向链表头
 * @param node 需要插入的新节点
 * @retval  0 - 插入成功
 * @retval -1 - 插入失败
 */
int32_t slist_insert_head(struct slist_head *head, struct slist_node *node);

/**
 * 在链表尾插入新节点
 * @param head 单向链表头
 * @param node 需要插入的新节点
 * @retval  0 - 插入成功
 * @retval -1 - 插入失败
 */
int32_t slist_insert_tail(struct slist_head *head, struct slist_node *node);

/**
 * 删除头节点
 * @param head 单向链表头
 * @retval  0 - 删除成功
 * @retval -1 - 删除失败
 */
int32_t slist_remove_head(struct slist_head *head);

/**
 * 获取当前节点的下一个节点
 * @param current 指定当前节点
 * @return 非NULL - 下一节点指针
 */
struct slist_node* slist_get_next(struct slist_node *current);

/**
 * 获取单向链表的头节点
 * @param head 单向链表头
 * @return 非NULL - 头节点指针
 */
struct slist_node* slist_get_head(struct slist_head *head);

/**
 * 获取单向链表的尾节点
 * @param head 单向链表头
 * @return 非NULL - 尾节点指针
 */
struct slist_node* slist_get_tail(struct slist_head *head);


#ifdef __cplusplus
}
#endif

#endif

