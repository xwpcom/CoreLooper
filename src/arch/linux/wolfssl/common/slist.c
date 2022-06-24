/**
 * Copyright (c), 2012~2018 iot.10086.cn All Rights Reserved
 * @file        slist.c
 * @brief 
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "slist.h"

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
int32_t slist_init(struct slist_head *head)
{
    if (NULL == head)
        return -1;

    head->head = NULL;
    head->tail = NULL;
    head->cnt = 0;

    return 0;
}

uint32_t slist_get_cnt(struct slist_head *head)
{
    if (NULL == head)
        return 0;
    return (head->cnt);
}

int32_t slist_insert_after(struct slist_head *head, struct slist_node *current,
        struct slist_node *node)
{
    if ((NULL == head) || (NULL == current) || (NULL == node))
        return -1;

    osl_memset(node, 0, sizeof(struct slist_node));
    node->next = current->next;
    if (NULL == node->next)
        head->tail = node;
    current->next = node;
    head->cnt++;

    return 0;
}

int32_t slist_insert_head(struct slist_head *head, struct slist_node *node)
{
    if ((NULL == head) || (NULL == node))
        return -1;

    osl_memset(node, 0, sizeof(struct slist_node));
    node->next = head->head;
    if (NULL == node->next)
        head->tail = node;
    head->head = node;
    head->cnt++;

    return 0;
}

int32_t slist_insert_tail(struct slist_head *head, struct slist_node *node)
{
    if ((NULL == head) || (NULL == node))
        return -1;

    osl_memset(node, 0, sizeof(struct slist_node));
    if(NULL != head->tail)
        head->tail->next = node;
    else
        head->head = node;
    head->tail = node;
    head->cnt++;

    return 0;
}

int32_t slist_remove_head(struct slist_head *head)
{
    if (NULL == head)
        return -1;

    head->head = head->head->next;

    if(head->cnt == 1)
    {
        head->tail = NULL;
    }
    head->cnt--;

    return 0;
}

struct slist_node* slist_get_next(struct slist_node *current)
{
    if (NULL == current)
        return NULL;

    return current->next;
}

struct slist_node* slist_get_head(struct slist_head *head)
{
    return head->head;
}

struct slist_node* slist_get_tail(struct slist_head *head)
{
    return head->tail;
}
