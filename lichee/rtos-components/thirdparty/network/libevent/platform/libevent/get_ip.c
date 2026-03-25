/**
 * @file get_ip.c
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
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
//#include <netinet/in.h>

#include "tc_iot_log.h"

#if 1
// LwIP 获取Ip地址
#include "lwip/sockets.h"
#include <lwip/netif.h>
#include <lwip/ip_addr.h>

// 传入一个有效的本地网口IP
const char *get_netif_ip(void)
{

    struct netif *pnetif = netif_list;

    if (pnetif != NULL && netif_is_up(pnetif)) {
        Log_i("ip address: %s\r\n", ip4addr_ntoa(netif_ip4_addr(pnetif)));
        return ip4addr_ntoa(netif_ip4_addr(pnetif));
    }
    else {
        Log_i("no active network interface or interface is down\r\n.");
        return "127.0.0.1";
    }
}

#else
// Linux 获取Ip地址
//#include <ifaddrs.h>
#include <sys/socket.h>

// 工作的网卡
#define ETH_INTERFACE "eth1"

static char sg_ip[16] = {0};

static int get_ip(void) 
{
	wlan_sta_ap_t *ap = cmd_malloc(sizeof(wlan_sta_ap_t));
	if (ap == NULL) {
		WMG_ERROR("no mem\n");
		return WMG_STATUS_FAIL;
	}
	ret = wlan_sta_ap_info(ap);
	if (ret == 0) {
		}

    return -1;
}

// 传入一个有效的本地网口IP
const char *get_netif_ip(void)
{
    if(get_ip() == 0){
        return sg_ip;
    }
    return "127.0.0.1";
}


#endif
