/**
 * @file basic_test.c
 * @author hubertxxu (hubertxxu@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-23
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

#include "libevent_test.h"

// 定时器回调函数
static void timeout_cb(evutil_socket_t fd, short event, void *arg)
{
    static int count = 0;
    Log_d("Timeout occurred %d\n", ++count);
    // 退出事件循环
    if(count >= 5){
        struct event_base *base = (struct event_base *)arg;
        event_base_loopbreak(base);
    }

}

void basic_libevent_test_entry(void *arg)
{
    struct event_base *base;
    struct event *timeout_event;
    struct timeval tv;
    base = (struct event_base *)arg;
    // 打印当前使用的事件后端
    Log_d("Using event method: %s\n", event_base_get_method(base));

    // 设置定时器的超时时间为5秒
    tv.tv_sec  = 5;
    tv.tv_usec = 0;

    // 创建一个新的定时器事件
    timeout_event = event_new(base, -1, EV_PERSIST, timeout_cb, base);
    if (!timeout_event) {
        Log_e("event_new fail.\n");
        event_base_free(base);
        return ;
    }

    // 添加定时器事件到事件循环中
    event_add(timeout_event, &tv);

    // 运行事件循环
    Log_d("Starting event loop\n");
    event_base_dispatch(base);
    Log_d("Event loop exited\n");

    // 清理
    event_free(timeout_event);
    event_base_free(base);

    return ;
}

int basic_libevent_test(void)
{
    static struct event_base *base;
    init_libevent_global(1);
    evutil_set_ifaddr((char *)get_netif_ip());
    // 创建一个新的事件基础结构
    base = event_base_new();
    if (!base) {
        Log_e("Could not initialize libevent\n");
        return -1;
    }

    static ThreadParams basic_params;

    basic_params.thread_name = "basic_libevent_test";
    basic_params.thread_func = basic_libevent_test_entry;
    basic_params.user_arg    = base;
    basic_params.stack_size  = 1024 * 10;
    basic_params.priority = THREAD_PRIORITY_NORMAL;

    return HAL_ThreadCreate(&basic_params);
}
