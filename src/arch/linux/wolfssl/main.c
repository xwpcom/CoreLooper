#if 1
int main(int arg, char** argv[])
{
	printf("hello from wolfssl\r\n");
	return 0;
}
#else
/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file main.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stdio.h>
#include <time.h>

#include "tm_data.h"
#include "data_types.h"
#include "tm_api.h"
#include "tm_user.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
/**
 * 用户可在OneNET Studio创建新的产品，导入目录中model-schema.json定义的物模型后，
 * 直接使用SDK内的tm_user.[ch]进行调试，同时还可以从平台导出tm_user.[ch]与SDK
 * 内置的文件进行对比，熟悉物模型功能点的开发方式
*/

#if 0
#define PRODUCT_ID    "lsuNMOH7kf"
#define DEVICE_NAME   "Dust1"
#define ACCESS_KEY    "tHKqz8cK5lPxG5qWx4Gn/gpHa6peufKQOgESzw5DcqU="
#elif 1
/*
电站名称：台山市都斛镇金星农场城门水电站

电站代码：4407221055

行政区域：江门市台山市
设备ID：4407221055
设备key：a563LDmhflkbLafB/CZr7c+34AxxEs2Y9YlcbTFefqU=
核定生态流量：0.02
测流方式：有压管道恒定流
*/
//广东小水电 正式账号
//#define PRODUCT_ID    "aQjwGxPJ1w"
#define PRODUCT_ID    "i0uL6Ar2LI"
#define DEVICE_NAME   "4407221055"
#define ACCESS_KEY    "a563LDmhflkbLafB/CZr7c+34AxxEs2Y9YlcbTFefqU="
#else
//广东小水电 测试账号
#define PRODUCT_ID    "aQjwGxPJ1w"
#define DEVICE_NAME   "4401831001"
#define ACCESS_KEY    "xJYHW0QfuDGxoLYcjeQT1i/TGpnRx7IRwSxMkJn4iD0="
#endif
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
int main(int arg, char **argv[])
{
    struct tm_downlink_tbl_t downlink_tbl = {0};
    //struct event_alert_t alert = {0};
    int32_t i = 0;

    downlink_tbl.prop_tbl = tm_prop_list;
    downlink_tbl.prop_tbl_size = tm_prop_list_size;
    downlink_tbl.svc_tbl = tm_svc_list;
    downlink_tbl.svc_tbl_size = tm_svc_list_size;

    if(0 == tm_init(&downlink_tbl))
    {
        printf("tm init ok\n");
    }

    if (0 == tm_login(PRODUCT_ID,DEVICE_NAME,ACCESS_KEY,5000))
    {
        printf("tm api login ok\n");
    }
    else
    {
        return -1;
    }

    while(i < 100)
    {
        if(0 != tm_step(200))
        {
            printf("step failed\n");
            break;
        }

        if(0 == i++ % 20)
        {
            printf("notify#1\r\n");
            //tm_prop_float32_notify(NULL, 3.33, 0, 3000);
            //tm_prop_int32_notify(NULL, 88, 0, 3000);
            //tm_prop_tsp_notify(NULL,125 , time_count(), 3000);
            printf("notify#2\r\n");
            //alert.cnt = 10;
            //tm_event_alert_notify(NULL, alert, 0, 3000);
            //printf("notify#3\r\n");

            if (1)
            {
                struct event_data_report_t val = {0};
                {
                    struct event_data_report_monitors_t* item = &val.monitors[0];
                    char mid[64];
                    
                    item = &val.monitors[0];
                    {
                        strcpy(mid, DEVICE_NAME);
                        strcat(mid, "_0");

                        item->monitor_id = mid;
                        item->monitor_type = 0;
                        item->water_level = 1.2;
                        item->dynamo_power = 3.4;
                        item->ecological_flow = 4.5;
                        item->flow_velocity = 5.6;
                        item->gatage = 6.7;
                    }
                    /*
                    item = &val.monitors[1];
                    {
                        strcpy(mid, DEVICE_NAME);
                        strcat(mid, "_1");

                        item->monitor_id = mid;
                        item->monitor_type = 0;
                        item->water_level = 1.2;
                        item->dynamo_power = 3.4;
                        item->ecological_flow = 4.5;
                        item->flow_velocity = 5.6;
                        item->gatage = 6.7;
                    }
                    */

                    val.monitors_element_cnt = 1;
                    val.station_code = DEVICE_NAME;
                    val.time = "2022-02-28 15:34:00";
                    val.total_flow = 4.5;
                    val.monitor_num = 1;
                }
                tm_event_data_report_notify(NULL, val, 0, 3000);
            }

            if (0)
            {
                struct event_reportAlarm_t val = {0};
                val.alarm = "test";
                tm_event_reportAlarm_notify(NULL, val, 0, 5000);
            }
        }

        usleep(50000); // 50ms
    }
    tm_logout(3000);

    return 0;
}

#endif
