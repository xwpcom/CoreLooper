/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file 		dlist.c
 * @brief		双向链表
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "dlist.h"
#include "plat_osl.h"

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
/* External Functions and Variables                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
int32_t dlist_init(struct dlist_head_t *head)
{
    if (NULL == head)
        return -1;

    head->head = NULL;
    head->tail = NULL;
    head->cnt = 0;

    return 0;
}

int32_t dlist_is_empty(struct dlist_head_t *head)
{
    if (NULL == head)
        return -1;
    return ((head->cnt) ? 0 : 1);
}

int32_t dlist_insert_after(struct dlist_head_t *head, struct dlist_node_t *current,
        struct dlist_node_t *node)
{
    if ((NULL == head) || (NULL == current) || (NULL == node))
        return -1;

    osl_memset(node, 0, sizeof(struct dlist_node_t));
    node->next = current->next;
    node->prev = current;
    if (node->next)
        node->next->prev = node;
    else
        head->tail = node;
    current->next = node;
    head->cnt++;

    return 0;
}

int32_t dlist_insert_head(struct dlist_head_t *head, struct dlist_node_t *node)
{
    if ((NULL == head) || (NULL == node))
        return -1;

    osl_memset(node, 0, sizeof(struct dlist_node_t));
    node->next = head->head;
    if (node->next)
        node->next->prev = node;
    else
        head->tail = node;
    node->prev = NULL;
    head->head = node;
    head->cnt++;

    return 0;
}

int32_t dlist_insert_tail(struct dlist_head_t *head, struct dlist_node_t *node)
{
    if ((NULL == head) || (NULL == node))
        return -1;

    osl_memset(node, 0, sizeof(struct dlist_node_t));
    node->prev = head->tail;
    if (node->prev)
        node->prev->next = node;
    else
        head->head = node;
    node->next = NULL;
    head->tail = node;
    head->cnt++;

    return 0;
}

int32_t dlist_remove_node(struct dlist_head_t *head, struct dlist_node_t *node)
{
    if ((NULL == head) || (NULL == node))
        return -1;

    if (NULL == node->prev)
        head->head = node->next;
    else
        node->prev->next = node->next;

    if (NULL == node->next)
        head->tail = node->prev;
    else
        node->next->prev = node->prev;

    head->cnt--;

    return 0;
}

struct dlist_node_t* dlist_get_next(struct dlist_node_t *current)
{
    if (NULL == current)
        return NULL;

    return current->next;
}

struct dlist_node_t* dlist_get_head(struct dlist_head_t *head)
{
    return head->head;
}

struct dlist_node_t* dlist_get_tail(struct dlist_head_t *head)
{
    return head->tail;
}

struct dlist_node_t* dlist_each(struct dlist_head_t *head, dlist_cb handle, void *arg)
{
    struct dlist_node_t *node = NULL;
    struct dlist_node_t *nextNode = NULL;

    if ((NULL == head) || (NULL == handle))
        return NULL;

    node = head->head;

    while (NULL != node)
    {
        nextNode = node->next;
        if (handle(node, arg))
            return node;

        node = nextNode;
    }

    return NULL;
}
