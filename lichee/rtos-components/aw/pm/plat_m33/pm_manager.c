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
#include <string.h>
#include <osal/hal_interrupt.h>

#include <errno.h>

#include <pm_adapt.h>
#include <pm_debug.h>
#include <pm_suspend.h>
#include <pm_wakecnt.h>
#include <pm_wakesrc.h>
#include <pm_wakelock.h>
#include <pm_testlevel.h>
#include <pm_devops.h>
#include <pm_syscore.h>
#include <pm_notify.h>
#include <pm_task.h>
#include <pm_platops.h>
#include <pm_subsys.h>
#include <pm_init.h>
#include <pm_m33_platops.h>
#include <pm_m33_wakesrc.h>
#include <pm_rpcfunc.h>
#include <pm_testlevel.h>

#undef   PM_DEBUG_MODULE
#define  PM_DEBUG_MODULE  PM_DEBUG_M33

extern uint32_t trigger_task_flag;

int pm_trigger_suspend(suspend_mode_t mode)
{
	uint32_t m33 = 0, rv = 0, dsp = 0;
	uint32_t flags;
	int ret;
	struct pm_task *ptr;

	pm_time_record_clr ();

	if (!pm_suspend_mode_valid(mode))
		return -EINVAL;

	// only for ococci!!!
	void pm_record_clear(void);
	pm_record_clear();
	
	if (pm_task_freeze(PM_TASK_TYPE_FREEZE_AT_ONCE)) {
		pm_err("freeze at once failed, trigger suspend failed.\n");
		return -EFAULT;
	}

	m33 = pm_wakelocks_refercnt(1);
#ifdef CONFIG_PM_SUBSYS_RISCV_SUPPORT
	rv  = pm_wakelocks_getcnt_riscv(1);
#endif
#ifdef CONFIG_PM_SUBSYS_DSP_SUPPORT
	dsp  = pm_wakelocks_getcnt_dsp(1);
#endif

	if (m33 + rv + dsp) {
		pm_warn("wakelock refuse to suspend, events m33: %d, rv: %d, dsp: %d\n",
				m33, rv, dsp);
		if (pm_task_restore(PM_TASK_TYPE_FREEZE_AT_ONCE)) {
			pm_err("wakelock check failed and trigger restore task at once failed\n");
			return -EFAULT;
		}
		return -EPERM;
	}

	ret = pm_suspend_request(mode);
	if (ret) {
		if (pm_task_restore(PM_TASK_TYPE_FREEZE_AT_ONCE)) {
			pm_err("suspend request failed and trigger restore task at once failed\n");
			return -EFAULT;
		}
	}

	ptr = pm_task_check_inarray(xTaskGetCurrentTaskHandle());
	if (ptr && !ret && (ptr->type == PM_TASK_TYPE_FREEZE_AT_ONCE)) {
		flags = hal_interrupt_disable_irqsave();
		if (!trigger_task_flag) {
			trigger_task_flag = 1;
			vTaskSuspend(NULL);
		} else
			trigger_task_flag = 0;
		hal_interrupt_enable_irqrestore(flags);
	}

	return ret;
}


int pm_init(int argc, char **argv)
{
	/* creat timer*/
	pm_wakelocks_init();

	/* before arch initialization of hardware devices */
	/* pm_wakesrc_init();
	   pm_wakecnt_init();
	*/

	/* create queue set*/
	pm_subsys_init();

	/* creat mutex, put pm_devops_init() and pm_syscore_init() before arch initialization of hardware devices
	pm_syscore_init();

	pm_devops_init();
	*/

	/* creat mutex*/
	pm_notify_init();

	/* protect Idle/Timer task */
	pm_task_init();

	/* register ops  */
	pm_m33_platops_init();

#ifdef CONFIG_PM_SUBSYS_RISCV_SUPPORT
	extern int pm_riscv_init(void);
	pm_riscv_init();
#endif

#ifdef CONFIG_PM_SUBSYS_DSP_SUPPORT
	extern int pm_dsp_init(void);
	pm_dsp_init();
#endif

	pm_wakeup_ops_m33_init();

#ifdef CONFIG_COMPONENTS_PM_TEST_TOOLS
	pm_test_tools_init();
#endif

#ifndef CONFIG_PM_SIMULATED_RUNNING
	extern int pm_lpsram_para_init(void);
	extern int pm_hpsram_para_init(void);
	pm_lpsram_para_init();
	pm_hpsram_para_init();
#endif

	/* creat pm_task and register it, then create queue*/
	pm_suspend_init();

	return 0;
}


