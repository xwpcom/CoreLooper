/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file 		dlist.h
 * @brief		双向链表。用户可以将dl_node增加到自定义的结构体中，通过对该node的操作实现自定义结构体的链表操作。
 */

#ifndef __DLIST_H__
#define __DLIST_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

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
 * @brief 双向链表节点定义，用户需要将该节点包含到自定义的结构体中，实现自定义结构体的链表化操作。
 * 
 */
struct dlist_node_t
{
    /** 指向上一个数据节点，当前节点为首节点时为NULL*/
    struct dlist_node_t *prev;
    /** 指向下一个数据节点，当前节点为末节点时为NULL*/
    struct dlist_node_t *next;
};

/**
 * @brief 双向链表头定义。
 * 
 */
struct dlist_head_t
{
    /** 指向链表的首节点，链表为空时为NULL*/
    struct dlist_node_t *head;
    /** 指向链表的末节点，链表为空时为NULL*/
    struct dlist_node_t *tail;
    /** 当前链表中的节点数量*/
    uint32_t cnt;
};
/*****************************************************************************/
/* External Function Prototypes                                              */
/*****************************************************************************/
/**
 * 初始化双向链表表头
 * @param head 双向链表头结构
 * @retval 0 成功
 * @retval -1 失败
 */
int32_t dlist_init(struct dlist_head_t *head);

/**
 * 查询指定链表是否为空
 * @param head 双向链表头结构
 * @retval -1 - 错误
 * @retval  0 - 链表内有节点
 * @retval  1 - 链表内没有节点
 */
int32_t dlist_is_empty(struct dlist_head_t *head);

/**
 * 在指定节点之后插入新节点
 * @param head 双向链表头
 * @param current 指定需要插入节点的位置
 * @param node 指定需要插入的新节点
 * @retval   0 - 插入成功
 * @retval  -1 - 插入失败
 */
int32_t dlist_insert_after(struct dlist_head_t *head, struct dlist_node_t *current,
                           struct dlist_node_t *node);

/**
 * 在链表头插入新节点
 * @param head 双向链表头
 * @param node 需要插入的新节点
 * @retval   0 - 插入成功
 * @retval  -1 - 插入失败
 */
int32_t dlist_insert_head(struct dlist_head_t *head, struct dlist_node_t *node);

/**
 * 在链表尾插入新节点
 * @param head 双向链表头
 * @param node 需要插入的新节点
 * @retval   0 - 插入成功
 * @retval  -1 - 插入失败
 */
int32_t dlist_insert_tail(struct dlist_head_t *head, struct dlist_node_t *node);

/**
 * 删除指定节点
 * @param head 双向链表头
 * @param node 需要删除的节点
 * @retval   0 - 删除成功
 * @retval  -1 - 删除失败
 */
int32_t dlist_remove_node(struct dlist_head_t *head, struct dlist_node_t *node);

/**
 * 获取当前节点的下一个节点
 * @param current 指定当前节点
 * @return 非NULL - 下一节点指针
 */
struct dlist_node_t* dlist_get_next(struct dlist_node_t *current);

/**
 * 获取双向链表的头节点
 * @param head 双向链表头
 * @return 非NULL - 头节点指针
 */
struct dlist_node_t* dlist_get_head(struct dlist_head_t *head);

/**
 * 获取双向链表的尾节点
 * @param head 双向链表头
 * @return 非NULL - 尾节点指针
 */
struct dlist_node_t* dlist_get_tail(struct dlist_head_t *head);

/**
 * @brief 链表轮询处理函数定义，仅dlist_each使用
 * @param node 轮询处理的当前节点
 * @param arg dlist_each传入的参数arg
 * @retval    0 - 继续轮询处理
 * @retval  非0 - 终止轮询。
 * 
 */
typedef int (*dlist_cb)(struct dlist_node_t* /*node*/, void* /*arg*/);

/**
 * 轮询链表内所有的节点并调用同一个函数，当函数返回为非0时退出轮询
 * @param head 双向链表头
 * @param handle 需要调用的函数回调（在回调函数内执行删除当前节点的操作是安全的）
 * @param arg 函数执行的参数
 * @return 非NULL - 退出轮询时所在节点
 */
struct dlist_node_t* dlist_each(struct dlist_head_t *head, dlist_cb handle, void *arg);


#ifdef __cplusplus
}
#endif  

#endif /* __DLIST_H__ */
