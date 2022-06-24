/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_user.c
 * @date 2020/05/14
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_data.h"
#include "tm_api.h"
#include "tm_user.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
/*************************** Property Func List ******************************/
struct tm_prop_tbl_t tm_prop_list[] = {
    TM_PROPERTY_RW(dustPM10),
    TM_PROPERTY_RW(tsp)
};
uint16_t tm_prop_list_size = ARRAY_SIZE(tm_prop_list);
/****************************** Auto Generated *******************************/


/***************************** Service Func List *******************************/
struct tm_svc_tbl_t tm_svc_list[] = {0};
uint16_t tm_svc_list_size = 0;
/****************************** Auto Generated *******************************/

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
/**************************** Property Func Read *****************************/
int32_t tm_prop_dustPM10_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */


    tm_data_struct_set_int32(data, "dustPM10", val);

    return 0;
}

int32_t tm_prop_tsp_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */


    tm_data_struct_set_int32(data, "tsp", val);

    return 0;
}

/****************************** Auto Generated *******************************/


/**************************** Property Func Write ****************************/
int32_t tm_prop_dustPM10_wr_cb(void *data)
{
    int32_t val = 0;

    tm_data_get_int32(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */


    return 0;
}

int32_t tm_prop_tsp_wr_cb(void *data)
{
    int32_t val = 0;

    tm_data_get_int32(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */


    return 0;
}

/****************************** Auto Generated *******************************/


/**************************** Service Func Invoke ****************************/
/****************************** Auto Generated *******************************/


/**************************** Property Func Notify ***************************/
int32_t tm_prop_dustPM10_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "dustPM10", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_tsp_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "tsp", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

/****************************** Auto Generated *******************************/


/***************************** Event Func Notify *****************************/
int32_t tm_event_data_report_notify(void *data, struct event_data_report_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    void *structure = tm_data_struct_create();
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    {
        void *array = tm_data_array_create(10);
        int32_t i = 0;

        for(i = 0; i < val.monitors_element_cnt; i++)
        {
            void *tmp_structure = tm_data_struct_create();

            tm_data_struct_set_string(tmp_structure, "monitor_id", val.monitors[i].monitor_id);
            tm_data_struct_set_int32(tmp_structure, "monitor_type", val.monitors[i].monitor_type);
            tm_data_struct_set_double(tmp_structure, "water_level", val.monitors[i].water_level);
            tm_data_struct_set_double(tmp_structure, "dynamo_power", val.monitors[i].dynamo_power);
            tm_data_struct_set_double(tmp_structure, "ecological_flow", val.monitors[i].ecological_flow);
            tm_data_struct_set_double(tmp_structure, "flow_velocity", val.monitors[i].flow_velocity);
            tm_data_struct_set_double(tmp_structure, "gatage", val.monitors[i].gatage);
            tm_data_array_set_struct(array, tmp_structure);
        }
        tm_data_struct_set_data(structure, "monitors", array);
    }
    tm_data_struct_set_string(structure, "station_code", val.station_code);
    tm_data_struct_set_string(structure, "time", val.time);
    tm_data_struct_set_double(structure, "total_flow", val.total_flow);
    tm_data_struct_set_int32(structure, "monitor_num", val.monitor_num);

    tm_data_set_struct(resource, "data_report", structure, timestamp);

    if(NULL == data)
    {
        ret = tm_post_event(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_event_media_report_notify(void *data, struct event_media_report_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    void *structure = tm_data_struct_create();
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_struct_set_string(structure, "time", val.time);
    {
        void *tmp_structure = tm_data_struct_create();

        tm_data_struct_set_string(tmp_structure, "monitor_id", val.monitors.monitor_id);
        tm_data_struct_set_int32(tmp_structure, "type", val.monitors.type);
        tm_data_struct_set_string(tmp_structure, "path", val.monitors.path);
        tm_data_struct_set_data(structure, "monitors", tmp_structure);
    } 
    tm_data_struct_set_string(structure, "station_code", val.station_code);

    tm_data_set_struct(resource, "media_report", structure, timestamp);

    if(NULL == data)
    {
        ret = tm_post_event(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_event_reportAlarm_notify(void *data, struct event_reportAlarm_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    void *structure = tm_data_struct_create();
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_struct_set_string(structure, "alarm", val.alarm);

    tm_data_set_struct(resource, "reportAlarm", structure, timestamp);

    if(NULL == data)
    {
        ret = tm_post_event(resource, timeout_ms);
    }

    return ret;
}

/****************************** Auto Generated *******************************/

