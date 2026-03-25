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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sunxi_hal_power.h>
#include <hal_cmd.h>
#include <hal_log.h>

static int cmd_test_power_get_bat(int argc, const char **argv)
{
	int ret;
	struct power_dev rdev;
	printf("[power] get power\n");
	hal_power_get(&rdev);

	printf("[power] get bat cap\n");
	ret = hal_power_get_bat_cap(&rdev);
	printf("[power] bat cap:%d\n", ret);

	printf("[power] get_coulumb_counter\n");
	ret = hal_power_get_coulumb_counter(&rdev);
	printf("[power] coulumb_counter:%d\n", ret);

	printf("[power] get bat_present\n");
	pr_err("pmu_bat_unused:%d\n", rdev.config->pmu_bat_unused);
	pr_err("pmu_version:%d\n", rdev.config->pmu_version);
	ret = hal_power_get_bat_present(&rdev);
	printf("[power] bat_present:%d\n", ret);

	printf("[power] get online\n");
	ret = hal_power_get_bat_online(&rdev);
	printf("[power] bat online:%d\n", ret);

	printf("[power] get status\n");
	ret = hal_power_get_bat_status(&rdev);
	printf("[power] bat status:%d\n", ret);

	printf("[power] get bat health\n");
	ret = hal_power_get_bat_health(&rdev);
	printf("[power] bat health:%d\n", ret);

	printf("[power] get vbat\n");
	ret = hal_power_get_vbat(&rdev);
	printf("[power] bat vbat:%d\n", ret);

	printf("[power] get ibat\n");
	ret = hal_power_get_ibat(&rdev);
	printf("[power] bat ibat:%d\n", ret);

	printf("[power] get disibat\n");
	ret = hal_power_get_disibat(&rdev);
	printf("[power] bat disibat:%d\n", ret);

	printf("[power] get temp\n");
	ret = hal_power_get_temp(&rdev);
	printf("[power] bat temp:%d\n", ret);

	printf("[power] bat get test finish\n");
	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_test_power_get_bat, test_power_get_bat, power hal APIs tests)

static int cmd_test_power_set_bat(int argc, const char **argv)
{
	int ret;
	struct power_dev rdev;
    int cur = strtol(argv[1], NULL, 0);
    int vol = strtol(argv[2], NULL, 0);

	printf("[power] set bat power:cur:%d vol:%d\n", cur, vol);
	hal_power_get(&rdev);

	printf("[power] set bat cur:%d\n", cur);
	ret = hal_power_set_chg_cur(&rdev, cur);
	printf("[power] chg_cur:%d\n", ret);

	printf("[power] set bat vol:%d\n", vol);
	ret = hal_power_set_chg_vol(&rdev, vol);
	printf("[power] _chg_vol:%d\n", ret);

	printf("[power] bat set test finish\n");
	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_test_power_set_bat, test_power_set_bat, power hal APIs tests)

static int cmd_test_power_get_usb(int argc, const char **argv)
{
	int ret;
	struct power_dev rdev;
	printf("[power] get power\n");
	hal_power_get(&rdev);

	printf("[power] get usb_status\n");
	ret = hal_power_get_usb_status(&rdev);
	printf("[power] usb_status:%d\n", ret);

	printf("[power] usb_ihold\n");
	ret = hal_power_get_usb_ihold(&rdev);
	printf("[power] usb_ihold:%d\n", ret);

	printf("[power] get usb_vhold\n");
	ret = hal_power_get_usb_vhold(&rdev);
	printf("[power] usb_vhold:%d\n", ret);

	printf("[power] get cc_status\n");
	ret = hal_power_get_cc_status(&rdev);
	printf("[power] cc_status:%d\n", ret);

	printf("[power] usb get test finish\n");
	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_test_power_get_usb, test_power_get_usb, power hal APIs tests)

static int cmd_test_power_set_usb(int argc, const char **argv)
{
	int ret;
	struct power_dev rdev;
    int cur = strtol(argv[1], NULL, 0);
    int vol = strtol(argv[2], NULL, 0);

	printf("[power] set usb power:cur:%d vol:%d\n", cur, vol);
	hal_power_get(&rdev);

	printf("[power] set usb_ihold:%d\n", cur);
	ret = hal_power_set_usb_ihold(&rdev, cur);
	printf("[power] usb_ihold:%d\n", ret);

	printf("[power] set usb_vhold:%d\n", vol);
	ret = hal_power_set_usb_vhold(&rdev, vol);
	printf("[power] usb_vhold:%d\n", ret);

	printf("[power] usb set test finish\n");
	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_test_power_set_usb, test_power_set_usb, power hal APIs tests)








