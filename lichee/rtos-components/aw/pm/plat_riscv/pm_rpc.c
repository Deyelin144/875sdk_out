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
#include <pm_rpcfunc.h>

extern uint32_t trigger_task_flag;

/*
 * The following functions are called by other modules,
 * So arguments should be of the full type.
 * Check the parameter conversion here.
 *  pointer,enum etc  -> int
 */
#ifdef CONFIG_AMP_PMOFM33_STUB
int pm_set_wakesrc(int id, int core, int status)
{
	return rpc_pm_set_wakesrc(id, core, status);
}

int pm_trigger_suspend(suspend_mode_t mode)
{
	int ret;
	unsigned long flags;
	struct pm_task *ptr;

	pm_time_record_clr ();

	if (pm_task_freeze(PM_TASK_TYPE_FREEZE_AT_ONCE)) {
		pm_err("freeze at once failed, trigger suspend failed.\n");
		return -EFAULT;
	}

	ret = rpc_pm_trigger_suspend((int)mode);
	if (ret) {
		if (pm_task_restore(PM_TASK_TYPE_FREEZE_AT_ONCE)) {
			pm_err("trigger restore task at once failed\n");
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

int pm_report_subsys_action(int subsys_id, int action)
{
	return rpc_pm_report_subsys_action(subsys_id, action);
}

int pm_subsys_soft_wakeup(int affinity, int irq, int action)
{
	return rpc_pm_subsys_soft_wakeup(affinity, irq, action);
}
#endif

/*
 * The following functions are called by the AMP-RPC module,
 * and the argument should be int.
 * Check the parameter conversion here.
 *  int  ->  pointer,enum etc
 */
#ifdef CONFIG_AMP_PMOFRV_SERVICE
int rpc_pm_wakelocks_getcnt_riscv(int stash)
{
	return pm_wakelocks_refercnt(!!stash);
}

int rpc_pm_msgtorv_trigger_notify(int mode, int event)
{
	int ret = 0;

	ret = pm_notify_event(mode, event);
	if (ret) {
		pm_err("%s(%d): pm notify event %d return failed value: %d\n",
			__func__, __LINE__, event, ret);
		return ret;
	}

	return ret;
}

int rpc_pm_msgtorv_trigger_suspend(int mode)
{
	return pm_suspend_request(mode);
}

int rpc_pm_msgtorv_check_subsys_assert(int mode)
{
	int ret = 0;

	ret = pm_suspend_assert();

	return ret;
}

uint8_t soft_wakesrc_reported = 0;
int rpc_pm_msgtorv_check_wakesrc_num(int type)
{
	int ret = 0;
	int num = 0;

	/* soft wakesrc must be enabled before system suspend entry to stop the suspend trigger */
	num = pm_wakesrc_type_check_num(type);
	if (type == PM_WAKESRC_SOFT_WAKEUP) {
		if (!soft_wakesrc_reported && num) {
			ret = pm_report_subsys_action(PM_SUBSYS_ID_RISCV, PM_SUBSYS_ACTION_KEEP_AWAKE);
			if (ret) {
				pm_err("%s(%d): pm_report_subsys_action failed, return: %d\n", __func__, __LINE__, ret);
			} else
				soft_wakesrc_reported = 1;
		} else if (soft_wakesrc_reported && !num) {
			ret = pm_report_subsys_action(PM_SUBSYS_ID_RISCV, PM_SUBSYS_ACTION_TO_NORMAL);
			if (ret) {
				pm_err("%s(%d): pm_report_subsys_action failed, return: %d\n", __func__, __LINE__, ret);
			} else
				soft_wakesrc_reported = 0;
		}
	}

	return num;
}
#endif

