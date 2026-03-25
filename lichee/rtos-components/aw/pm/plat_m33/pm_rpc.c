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

#include "pm_adapt.h"
#include "pm_debug.h"
#include "pm_suspend.h"
#include "pm_wakecnt.h"
#include "pm_wakesrc.h"
#include "pm_wakelock.h"
#include "pm_testlevel.h"
#include "pm_devops.h"
#include "pm_syscore.h"
#include "pm_notify.h"
#include "pm_task.h"
#include "pm_platops.h"
#include "pm_subsys.h"

#include "pm_manager.h"
#include "pm_rpcfunc.h"
#include "pm_m33_wakesrc.h"

/* for m33 */
int pm_set_wakesrc(int id, int core, int status)
{
	if (status)
		return pm_wakesrc_active(id, core);
	else
		return pm_wakesrc_deactive(id, core);
}

int pm_subsys_soft_wakeup(int affinity, int irq, int action)
{
	/* wakecnt will increase on its own processer. */
	pm_warn("soft wakeup require: affinity: %d, irq: %d, action: %d\n",
		affinity, irq, action);
	pm_wakecnt_subcnt_inc(irq);

	return 0;
}

/*
 * The following functions are called by the AMP-RPC module,
 * and the argument should be int.
 * Check the parameter conversion here.
 *  int  ->  pointer,enum etc
 */

#ifdef CONFIG_AMP_PMOFM33_SERVICE
int rpc_pm_set_wakesrc(int wakesrc_id, int core, int status)
{
       int ret = -1;

       if (status) {
               ret = pm_wakesrc_active(wakesrc_id, core);
       } else {
               ret = pm_wakesrc_deactive(wakesrc_id, core);
       }

       return ret;
}

int rpc_pm_trigger_suspend(int mode)
{
       return pm_trigger_suspend(mode);
}

int rpc_pm_report_subsys_action(int subsys_id, int action)
{
       return pm_subsys_update(subsys_id, action);
}

int rpc_pm_subsys_soft_wakeup(int affinity, int irq, int action)
{
	/* if wakeup wakecnt inc to quit again judge 
	 * else just recorde event
	 */
	return pm_subsys_soft_wakeup(affinity, irq, action);
}
#endif

/*
 * The following functions are called by other modules,
 * So arguments should be of the full type.
 * Check the parameter conversion here.
 *  pointer,enum etc  -> int
 */
#ifdef CONFIG_AMP_PMOFRV_STUB
uint32_t pm_wakelocks_getcnt_riscv(int stash)
{
       return (uint32_t)rpc_pm_wakelocks_getcnt_riscv(!!stash);
}

int pm_msgtorv_trigger_notify(int mode, int event)
{
       return rpc_pm_msgtorv_trigger_notify(mode, event);
}

int pm_msgtorv_trigger_suspend(int mode)
{
       return rpc_pm_msgtorv_trigger_suspend(mode);
}

int pm_msgtorv_check_subsys_assert(int mode)
{
	return rpc_pm_msgtorv_check_subsys_assert(mode);
}

int pm_msgtorv_check_wakesrc_num(int type)
{
	return rpc_pm_msgtorv_check_wakesrc_num(type);
}
#endif

#ifdef CONFIG_AMP_PMOFDSP_STUB
uint32_t pm_wakelocks_getcnt_dsp(int stash)
{
       return (uint32_t)rpc_pm_wakelocks_getcnt_dsp(!!stash);
}

int pm_msgtodsp_trigger_notify(int mode, int event)
{
       return rpc_pm_msgtodsp_trigger_notify(mode, event);
}

int pm_msgtodsp_trigger_suspend(int mode)
{
       return rpc_pm_msgtodsp_trigger_suspend(mode);
}

int pm_msgtodsp_check_subsys_assert(int mode)
{
	return rpc_pm_msgtodsp_check_subsys_assert(mode);
}

int pm_msgtodsp_check_wakesrc_num(int type)
{
	return rpc_pm_msgtodsp_check_wakesrc_num(type);
}
#endif

