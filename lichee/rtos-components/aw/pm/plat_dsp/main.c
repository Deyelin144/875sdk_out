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

#include <stdlib.h>
#include <errno.h>
#include <console.h>

#include <mpu_wrappers.h>

#include "pm_debug.h"
#include "pm_suspend.h"

#ifdef CONFIG_AMP_PMOFM33_STUB
extern int pm_trigger_suspend(suspend_mode_t mode);
#else
int pm_trigger_suspend(suspend_mode_t mode) {
	pm_err("do not support trigger system suspend\n");
	return 0;
}
#endif
int pm_enter_sleep(int argc, char **argv)
{
	int ret = 0;
	pm_raw("the dsp to suspend\n");
	ret = pm_suspend_request(PM_MODE_SLEEP);
	pm_raw("%s return %d\n", __func__, ret);

	return 0;
}

int pm_enter_standby(int argc, char **argv)
{
	int ret = 0;
	pm_raw("the dsp to suspend\n");
	ret = pm_suspend_request(PM_MODE_STANDBY);
	pm_raw("%s return %d\n", __func__, ret);

	return 0;
}

int pm_enter_hibernation(int argc, char **argv)
{
	int ret = 0;
	ret = pm_trigger_suspend(PM_MODE_HIBERNATION);
	pm_raw("%s return %d\n", __func__, ret);

	return 0;
}

#if 0
/* if core is not standby control core, pm_test is used for debugging only */
int pm_test(int argc, char **argv)
{
	int ret = 0;
	int lvl = 0;


	if (argc != 2)
		goto err_inval;

	lvl = atoi(argv[1]);

	pm_raw("lvl: %d\n", lvl);
	if (!pm_suspend_testlevel_valid(lvl))
		goto err_inval;

	ret = pm_suspend_test_set(lvl);
	pm_raw("%s return %d\n", __func__, ret);

	return 0;

err_inval:
	pm_raw("%s args invalid.\n", __func__);
	pm_raw("argc: %d.\n", argc);
	for (int i=0; i<argc; i++) {
		pm_raw("argv[i]: %s.\n", argv[i]);
	}
	pm_raw("\n");
	return -EINVAL;
}
FINSH_FUNCTION_EXPORT_CMD(pm_test, pm_test, PM APIs tests004)
#endif

FINSH_FUNCTION_EXPORT_CMD(pm_enter_sleep, sleep, PM APIs tests001)
FINSH_FUNCTION_EXPORT_CMD(pm_enter_standby, standby, PM APIs tests002)
FINSH_FUNCTION_EXPORT_CMD(pm_enter_hibernation, hibernation, PM APIs tests003)

