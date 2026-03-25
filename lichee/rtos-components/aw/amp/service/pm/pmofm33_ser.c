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
#include <stdio.h>
#include <string.h>

#include "sunxi_amp.h"
#include <hal_cache.h>

#include "pm_debug.h"
#include "pm_rpcfunc.h"


// server: M33
static int _rpc_pm_set_wakesrc(int wakesrc_id, int core, int status)
{
	pm_ser_trace3(wakesrc_id, core, status);
	return rpc_pm_set_wakesrc(wakesrc_id, core, status);
}

static int _rpc_pm_trigger_suspend(int mode)
{
	pm_ser_trace1(mode);
	return rpc_pm_trigger_suspend(mode);
}

static int _rpc_pm_report_subsys_action(int subsys_id, int action)
{
	pm_ser_trace2(subsys_id, action);

	return rpc_pm_report_subsys_action(subsys_id, action);
}

int _rpc_pm_subsys_soft_wakeup(int affinity, int irq, int action)
{
	pm_ser_trace3(affinity, irq, action);
	return rpc_pm_subsys_soft_wakeup(affinity, irq, action);
}

sunxi_amp_func_table pmofm33_table[] = {
    {.func = (void *)&_rpc_pm_set_wakesrc, .args_num = 3, .return_type = RET_POINTER},
    {.func = (void *)&_rpc_pm_trigger_suspend, .args_num = 1, .return_type = RET_POINTER},
    {.func = (void *)&_rpc_pm_report_subsys_action, .args_num = 2, .return_type = RET_POINTER},
    {.func = (void *)&_rpc_pm_subsys_soft_wakeup, .args_num = 3, .return_type = RET_POINTER},
};

