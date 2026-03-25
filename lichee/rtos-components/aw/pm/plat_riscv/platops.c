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

#include <errno.h>
#include <hal_osal.h>
#include <osal/hal_interrupt.h>

#include <io.h>
#include "pm_suspend.h"
#include "pm_debug.h"
#include "pm_wakecnt.h"
#include "pm_wakesrc.h"
#include "pm_subsys.h"
#include "pm_syscore.h"
#include "pm_platops.h"
#include "pm_systeminit.h"

#define GPRCM_RV_BOOT_FLAG_REG		(0x400501d8)

extern void _rv_cpu_suspend(void);
extern void _rv_cpu_save_boot_flag(void);
extern void _rv_cpu_resume(void);
extern long rv_sleep_flag;
int cmd_rpcconsole_ext(int argc, char **argv);

int pm_risv_call_m33_set_io_to_wakeup(uint32_t val, uint32_t edge_mode, uint32_t time_s)//设置唤醒源到m33
{
	printf("[%s]=%d\n",__func__,val);
	char *argv[6] = {
		"rpccli",
		"arm",
		"pm_set_wupio",
        "0",
		"0",
		"0",
    };
	char s_val[10] = {0};
	char s_edge[10] = {0};
	char s_time[10] = {0};

	itoa(val, s_val, 10);
	argv[3] = s_val;

	itoa(edge_mode, s_edge, 10);
	argv[4] = s_edge;

	itoa(time_s, s_time, 10);
	argv[5] = s_time;

	return cmd_rpcconsole_ext(6, argv);
}

int pm_risv_call_m33_clear_wakeupio(uint32_t val)//通知m33清除唤醒源
{
	printf("[%s]=%d\n",__func__,val);
	char *argv[4] = {
		"rpccli",
		"arm",
		"pm_clear_wupio",
        "0",
    };
	char s_val[10] = {0};

	itoa(val, s_val, 10);
	argv[3] = s_val;

	return cmd_rpcconsole_ext(4, argv);
}

int pm_risv_call_m33_set_time_to_wakeup(int mode)
{
	char *argv[4] = {
		"rpccli",
		"arm",
		"time_to_wakeup_ms",
        "0",
    };
	char string[10] = {0};
	if (mode < 0) mode = 0;
	if (mode) {
		itoa(mode, string, 10);
		argv[3] = string;
	}

	return cmd_rpcconsole_ext(4, argv);
}

static int cmd_set_rv_time_to_wakeup_ms(int argc, char **argv)
{
	int ret;
	int val = -1;

	if (argc != 2) {
		pm_err("%s[%d]: invalid param(argc:%d)\n", __func__, __LINE__, argc);
		return -EINVAL;
	}

	val = atoi(argv[1]);
	if (val < 0) {
		pm_err("%s[%d]: invalid param(val:%d)\n", __func__, __LINE__, val);
		return -EINVAL;
	}

	ret = pm_risv_call_m33_set_time_to_wakeup(val);
	if (ret)
		pm_err("cmd set time_to_wakeup_ms failed\n");

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_set_rv_time_to_wakeup_ms, rv_time_to_wakeup_ms, pm tools)

void pm_riscv_clear_boot_flag(void)
{
	writel(0x429b0000, GPRCM_RV_BOOT_FLAG_REG);
}

static int pm_riscv_valid(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int pm_riscv_begin(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		/* suspend all subsys */
		//ret = pm_subsys_suspend_sync(mode);
		pm_hal_clear_wakeup_irq();
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int pm_riscv_prepare(suspend_mode_t mode)
{

	return 0;
}


static int pm_riscv_prepare_late(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		ret = pm_suspend_assert();
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;

}


static int set_finished_dummy(void)
{
	writel(0x16aaf5f5, 0x40050000 + 0x200);

	return 0;
}

extern void hal_icache_init(void);
extern void hal_dcache_init(void);
static int pm_riscv_enter(suspend_mode_t mode)
{
	int ret = 0;

	pm_debug_watchdog_stop(10);
	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		if (mode != PM_MODE_HIBERNATION)
			_rv_cpu_save_boot_flag();
		_rv_cpu_suspend();

		if (rv_sleep_flag) {
			pm_dbg("suspened\n");
			hal_dcache_clean_all();
			set_finished_dummy();
			while (readl(0x40050000 + 0x200)) {
				__asm__ __volatile__("wfi");
			}
			_rv_cpu_resume();
		}
		while (readl(0x40050000 + 0x200));
		pm_dbg("resume\n");
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}
	pm_debug_watchdog_start(10);

	if (mode != PM_MODE_SLEEP)
		pm_systeminit();

	return ret;
}


static int pm_riscv_wake(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
		break;
	case PM_MODE_STANDBY:
		/* pm_syscore_resume(mode); //call by pm framework */
		break;
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;

}


static int pm_riscv_finish(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
		break;
	case PM_MODE_STANDBY:
		break;
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;

}


static int pm_riscv_end(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
		break;
	case PM_MODE_STANDBY:
		//pm_wakesrc_complete();
		//pm_subsys_resume_sync(mode);
		break;
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int pm_riscv_recover(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
		break;
	case PM_MODE_STANDBY:
		break;
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;

}


static int pm_riscv_again(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
		break;
	case PM_MODE_STANDBY:
		break;
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static suspend_ops_t pm_riscv_suspend_ops = {
	.name  = "pm_riscv_suspend_ops",
	.valid = pm_riscv_valid,
	.begin = pm_riscv_begin,
	.prepare = pm_riscv_prepare,
	.prepare_late = pm_riscv_prepare_late,
	.enter = pm_riscv_enter,
	.wake = pm_riscv_wake,
	.finish = pm_riscv_finish,
	.end = pm_riscv_end,
	.recover = pm_riscv_recover,
	.again = pm_riscv_again,
};

int pm_riscv_platops_init(void)
{
	return pm_platops_register(&pm_riscv_suspend_ops);
}

int pm_riscv_platops_deinit(void)
{
	return pm_platops_register(NULL);
}



