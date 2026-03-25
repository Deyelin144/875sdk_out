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

#include <FreeRTOS.h>
#include <errno.h>
#include <hal_osal.h>
#include <aw_io.h>

#include "pm_suspend.h"
#include "pm_debug.h"
#include "pm_wakecnt.h"
#include "pm_wakesrc.h"
#include "pm_subsys.h"
#include "pm_syscore.h"
#include "pm_platops.h"
#include "pm_systeminit.h"

extern int g_xtensa_sleep_flag;
extern void suspend_xtensa_processor(void);
extern void _suspend_save_intlevel(void);
extern void _resume_restore_intlevel(void);

static int pm_dsp_valid(suspend_mode_t mode)
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

static int pm_dsp_begin(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		/* suspend all subsys */
		/* ret = pm_subsys_suspend_sync(mode); */
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int pm_dsp_prepare(suspend_mode_t mode)
{
	/* hal_msleep(500); */

	return 0;
}

static int pm_dsp_prepare_late(suspend_mode_t mode)
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
	writel(0x16aaf5f5, 0x40050000 + 0x204);

	return 0;
}

static int pm_dsp_enter(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		_suspend_save_intlevel();
		suspend_xtensa_processor();

		if (g_xtensa_sleep_flag)
		{
			/* after function suspend_xtensa_processor return, CPU will execute this branch */
			hal_dcache_clean_all();
			pm_dbg("dsp suspened\n");
			set_finished_dummy();
			while (readl(0x40050000 + 0x204) != 0) {
				__asm__ volatile("waiti 3\n" ::: "memory");
			}
			g_xtensa_sleep_flag = 0;
		}
		else
		{
			/* after function resume_xtensa_processor_nw return, CPU will execute this branch */
			while (readl(0x40050000 + 0x204) != 0) {
			}
			pm_dbg("dsp resume\n");
		}

		/* If Interrupt Option, then WAITI imm4 means PS.INTLEVEL <- imm4 */
		_resume_restore_intlevel();
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	if (mode != PM_MODE_SLEEP)
		pm_systeminit();

	return ret;
}

static int pm_dsp_wake(suspend_mode_t mode)
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

static int pm_dsp_finish(suspend_mode_t mode)
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

static int pm_dsp_end(suspend_mode_t mode)
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

static int pm_dsp_recover(suspend_mode_t mode)
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

static int pm_dsp_again(suspend_mode_t mode)
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

static suspend_ops_t pm_dsp_suspend_ops = {
	.name  = "pm_dsp_suspend_ops",
	.valid = pm_dsp_valid,
	.begin = pm_dsp_begin,
	.prepare = pm_dsp_prepare,
	.prepare_late = pm_dsp_prepare_late,
	.enter = pm_dsp_enter,
	.wake = pm_dsp_wake,
	.finish = pm_dsp_finish,
	.end = pm_dsp_end,
	.recover = pm_dsp_recover,
	.again = pm_dsp_again,
};

int pm_dsp_platops_init(void)
{
	return pm_platops_register(&pm_dsp_suspend_ops);
}

int pm_dsp_platops_deinit(void)
{
	return pm_platops_register(NULL);
}

