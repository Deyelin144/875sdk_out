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
#include <reent.h>

#include "sunxi_amp.h"
#include <pthread.h>
#include <console.h>

#include "pm_debug.h"
#include "pm_rpcfunc.h"

int rpc_pm_set_wakesrc(int wakesrc_id, int core, int status)
{
	void *args[3] = {0};
	args[0] = (void *)(intptr_t)wakesrc_id;
	args[1] = (void *)(intptr_t)core;
	args[2] = (void *)(intptr_t)status;
	pm_stub_trace3(wakesrc_id, core, status);
	return func_stub(RPCCALL_PMOFM33(_rpc_pm_set_wakesrc), 1, ARRAY_SIZE(args), args);
}

int rpc_pm_trigger_suspend(int mode)
{
	void *args[1] = {0};
	args[0] = (void *)(intptr_t)mode;
	pm_stub_trace1(mode);
	return func_stub(RPCCALL_PMOFM33(_rpc_pm_trigger_suspend), 1, ARRAY_SIZE(args), args);
}

int rpc_pm_report_subsys_action(int subsys_id, int action)
{
	void *args[2] = {0};
	args[0] = (void *)(intptr_t)subsys_id;
	args[1] = (void *)(intptr_t)action;
	pm_stub_trace2(subsys_id, action);
	return func_stub(RPCCALL_PMOFM33(_rpc_pm_report_subsys_action), 1, ARRAY_SIZE(args), args);
}

int rpc_pm_subsys_soft_wakeup(int affinity, int irq, int action)
{
	void *args[3] = {0};
	args[0] = (void *)(intptr_t)affinity;
	args[1] = (void *)(intptr_t)irq;
	args[2] = (void *)(intptr_t)action;
	pm_stub_trace3(affinity, irq, action);
	return func_stub(RPCCALL_PMOFM33(_rpc_pm_subsys_soft_wakeup), 1, ARRAY_SIZE(args), args);
}
