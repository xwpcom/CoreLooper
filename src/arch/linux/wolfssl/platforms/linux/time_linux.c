/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file time_linux.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "plat_osl.h"
#include "plat_time.h"

#include <sys/time.h>
#include <time.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
struct countdown_tmr_t
{
    uint64_t time_end;
};
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
uint64_t time_count_ms(void)
{
    uint64_t cnt_ms = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    cnt_ms = ((uint64_t)(tv.tv_sec)) * 1000 + tv.tv_usec / 1000;
    return cnt_ms;
}

uint64_t time_count(void)
{
    return (uint64_t)time(NULL);
}

void time_delay_ms(uint32_t m_sec)
{
    usleep(m_sec * 1000);
}

void time_delay(uint32_t sec)
{
    sleep(sec);
}

handle_t countdown_start(uint32_t ms)
{
    struct countdown_tmr_t *tmr = NULL;

    tmr = osl_malloc(sizeof(*tmr));

    if(tmr)
    {
        countdown_set((handle_t)tmr, ms);
    }

    return (handle_t)tmr;
}

void countdown_set(handle_t handle, uint32_t new_ms)
{
    struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)handle;

    if(tmr)
    {
        tmr->time_end = time_count_ms() + new_ms;
    }
}

uint32_t countdown_left(handle_t handle)
{
    struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)handle;
    int64_t left = 0;

    if(tmr)
    {
        left = tmr->time_end - time_count_ms();
    }

    return ((left > 0) ? left : 0);
}

uint32_t countdown_is_expired(handle_t handle)
{
    struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)handle;

    return ((!tmr || (tmr->time_end <= time_count_ms())) ? 1 : 0);
}

void countdown_stop(handle_t handle)
{
    if(handle)
    {
        osl_free((void*)handle);
    }
}
