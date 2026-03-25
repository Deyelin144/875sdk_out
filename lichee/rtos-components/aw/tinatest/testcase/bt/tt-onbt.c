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
#include <errno.h>
#include <tinatest.h>
#include "tt-btcommon.h"
#include "bt_manager.h"
#include "btmg_dev_list.h"

btmg_callback_t tt_btcbs;
dev_list_t *scan_devices = NULL;

int tt_onbt(int argc, char **argv)
{
    btmg_err ret;
    btmg_adapter_state_t bt_state;
    char tt_name[64];
    tname(tt_name);

    ttbt_printf("=======TINATEST FOR %s=========", tt_name);
    ttbt_printf("It's in onbt[turn on bt]for tinatest");
    ttips("Currently turn on bluetooth test");

    btmg_adapter_get_state(&bt_state);

    if (bt_state == BTMG_ADAPTER_ON) {
        ttbt_printf("Warn:bt is on, please turn off bt by [tt bt_off]");
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    tt_btcbs.btmg_adapter_cb.state_cb = tt_bt_adapter_status_cb;
    tt_btcbs.btmg_adapter_cb.scan_status_cb = tt_bt_scan_status_cb;
    tt_btcbs.btmg_device_cb.device_add_cb = tt_bt_scan_dev_add_cb;

    btmg_set_loglevel(BTMG_LOG_LEVEL_WARNG);
    btmg_core_init();
    btmg_register_callback(&tt_btcbs);
    btmg_set_profile(BTMG_A2DP_SOURCE | BTMG_GATT_CLIENT);
    ret = btmg_adapter_enable(true);

    tt_ble_advertise_on();
    ttbt_printf("BLE advertise on!");

    if (ret == BT_FAIL) {
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    ttbt_printf("=====TINATEST FOR %s OK======", tt_name);

    return 0;
}

testcase_init(tt_onbt, bt_on, bt turn on for tinatest);
