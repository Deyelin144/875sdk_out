/**
 * @file main.c
 * @author hubertxxu (hubertxxu@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright
 * Tencent is pleased to support the open source community by making IoT Hub available. 
 * Copyright(C) 2021 - 2026 THL A29 Limited, a Tencent company.All rights reserved.
 * Licensed under the MIT License(the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "tc_iot_hal.h"
#include "tc_iot_log.h"
#include "libevent_test.h"
#include <console.h>
#include "tc_iot_log.h"

extern int os_test(void);
extern int tls_test(void);

void main_test(void *arg)
{
    int loop = 0;
    int rc = 0;
    char buf[32] = "log dump test...0123456789abcdef";
    IOT_Log_Set_Level(eLOG_DEBUG);
    Log_d("debug log test.");
    Log_i("info log test.");
    Log_w("warn log test.");
    Log_e("error log test.");
    Log_dump(buf,sizeof(buf));

    rc = tls_test();
    if(rc){
        Log_e("tls test failed\n");
       // return rc;
    }
    rc = os_test();
    if(rc){
        Log_e("os test failed\n");
        //return rc;
    }

    HAL_Printf("\r\n============================>>> libevent start test <<<=========================>>>\r\n\r\n");
    libevent_test("start");
    do
    {
        HAL_SleepMs(1000);
    }while (loop++ < 10);
 	HAL_Printf("\r\n============================>>> libevent end <<<=========================>>>\r\n\r\n");
	libevent_test("stop");
	vTaskDelete(NULL);
}

TaskHandle_t event_test_handle;
int event_test(int argc, char *argv[])
{
    int ret = xTaskCreate(main_test, "event_test", 1024 * 10, NULL,
			20, &event_test_handle);
    if (ret != pdPASS) {
        Log_e("xTaskCreate failed: %d", ret);
        return -1;
    }
	printf("create event_test success!!\n");
}


FINSH_FUNCTION_EXPORT_CMD(event_test, event_test, libevent tests)


