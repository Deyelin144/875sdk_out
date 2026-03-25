/**
 * @file libevent_test.h
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

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef PLATFORM_SIFLI_RTOS
#include <lwip/sockets.h>
#else
#include <sys/socket.h>
#endif
#include <sys/types.h>
#include <time.h>
#include <arpa/inet.h>

#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/libevent.h>
#include <event2/thread.h>
#include <event2/util.h>

#include "tc_iot_hal.h"
#include "tc_iot_log.h"

int init_libevent_global(int debug);

const char *get_netif_ip(void);

int basic_libevent_test(void);

void libevent_test(char *cmd);