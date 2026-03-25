/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdint.h>
#include <stdio.h>
#include <tinatest.h>
#include <stdbool.h>
#include "tt-btcommon.h"
#include "bt_manager.h"
#include "btmg_dev_list.h"

int tt_scanbt(int argc, char **argv)
{
    btmg_err ret;
    int sleep_ms = 1000 * 10;
    dev_node_t *dev_node = NULL;
    char tt_name[64];
    btmg_adapter_state_t bt_state;

    tname(tt_name);

    ttbt_printf("=======TINATEST FOR %s=========", tt_name);
    ttbt_printf("It's in onff[turn off bt]for tinatest");
    ttips("Currently bluetooth scan test");

    btmg_adapter_get_state(&bt_state);
    if (bt_state != BTMG_ADAPTER_ON) {
        ttbt_printf("Warn:bt is not turned on, please turn on bt by [tt bt_on]");
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    if (scan_devices != NULL) {
        btmg_dev_list_free(scan_devices);
        scan_devices = NULL;
    }

    scan_devices = btmg_dev_list_new();
    if (scan_devices == NULL) {
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    ret = btmg_adapter_start_scan();
    if (ret == BT_FAIL) {
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    ttbt_printf("Start scanning devices for 10 seconds\n");
    usleep(sleep_ms * 1000);
    ret = btmg_adapter_stop_scan();
    if (ret == BT_FAIL) {
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    if (scan_devices == NULL) {
        ttbt_printf("Scan list is empty");
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    usleep(1000 * 1000);
    ttbt_printf("==========Scanned Device List==========\n");
    dev_node = scan_devices->head;
    while (dev_node != NULL) {
        ttbt_printf("addr: %s, name: %s", dev_node->dev_addr, dev_node->dev_name);
        dev_node = dev_node->next;
    }

    ttbt_printf("=====TINATEST FOR %s OK======", tt_name);

    return 0;
}

testcase_init(tt_scanbt, bt_scan, bt scan for tinatest);
