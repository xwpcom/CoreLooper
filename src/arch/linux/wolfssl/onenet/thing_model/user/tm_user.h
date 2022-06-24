/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_user.h
 * @date 2020/05/14
 * @brief
 */

#ifndef __TM_USER_H__
#define __TM_USER_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "tm_api.h"

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
/****************************** Structure type *******************************/
#pragma pack(1)
struct event_data_report_monitors_t
{
    int8_t *monitor_id;
    int32_t monitor_type;
    float64_t water_level;
    float64_t dynamo_power;
    float64_t ecological_flow;
    float64_t flow_velocity;
    float64_t gatage;
};


struct event_data_report_t
{
    struct event_data_report_monitors_t monitors[10];
    uint32_t monitors_element_cnt;
    int8_t *station_code;
    int8_t *time;
    float64_t total_flow;
    int32_t monitor_num;
};
struct event_media_report_monitors_t
{
    int8_t *monitor_id;
    int32_t type;
    int8_t *path;
};


struct event_media_report_t
{
    int8_t *time;
    struct event_media_report_monitors_t monitors;
    int8_t *station_code;
};

struct event_reportAlarm_t
{
    int8_t *alarm;
};


#pragma pack()
/****************************** Auto Generated *******************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/*************************** Property Func List ******************************/
extern struct tm_prop_tbl_t tm_prop_list[];
extern uint16_t tm_prop_list_size;
/****************************** Auto Generated *******************************/


/**************************** Service Func List ******************************/
extern struct tm_svc_tbl_t tm_svc_list[];
extern uint16_t tm_svc_list_size;
/****************************** Auto Generated *******************************/


/**************************** Property Func Read ****************************/
int32_t tm_prop_dustPM10_rd_cb(void *data);
int32_t tm_prop_tsp_rd_cb(void *data);
/****************************** Auto Generated *******************************/


/**************************** Property Func Write ****************************/
int32_t tm_prop_dustPM10_wr_cb(void *data);
int32_t tm_prop_tsp_wr_cb(void *data);
/****************************** Auto Generated *******************************/


/**************************** Service Func Invoke ****************************/
/****************************** Auto Generated *******************************/


/**************************** Property Func Notify ***************************/
int32_t tm_prop_dustPM10_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_tsp_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
/****************************** Auto Generated *******************************/


/***************************** Event Func Notify *****************************/
int32_t tm_event_data_report_notify(void *data, struct event_data_report_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_event_media_report_notify(void *data, struct event_media_report_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_event_reportAlarm_notify(void *data, struct event_reportAlarm_t val, uint64_t timestamp, uint32_t timeout_ms);
/****************************** Auto Generated *******************************/

#ifdef __cplusplus
}
#endif

#endif
