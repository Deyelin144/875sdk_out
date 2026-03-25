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
#include "tt-btcommon.h"
#include "bt_manager.h"

int tt_offbt(int argc, char **argv)
{
    btmg_err ret;
    char tt_name[64];
    btmg_adapter_state_t bt_state;
    tname(tt_name);

    ttbt_printf("=======TINATEST FOR %s=========", tt_name);
    ttbt_printf("It's in onff[turn off bt]for tinatest\n");
    ttips("Currently turn off bluetooth test");

    btmg_adapter_get_state(&bt_state);
    if (bt_state == BTMG_ADAPTER_OFF) {
        ttbt_printf("Warn:bt is off, please turn on bt by [tt bt_on]");
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    btmg_le_enable_adv(false);
    btmg_adapter_enable(false);
    ret = btmg_core_deinit();
    if (ret == BT_FAIL) {
        ttbt_printf("=====TINATEST FOR %s FAIL======", tt_name);
        return -1;
    }

    btmg_unregister_callback();

    if (scan_devices != NULL) {
        btmg_dev_list_free(scan_devices);
        scan_devices = NULL;
    }

    ttbt_printf("======TINATEST FOR %s OK=======", tt_name);

    return 0;
}

testcase_init(tt_offbt, bt_off, bt turn off for tinatest);
