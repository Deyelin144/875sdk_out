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

int rpc_pm_wakelocks_getcnt_riscv(int stash)
{
	void *args[1] = {0};
	args[0] = (void *)stash;
	pm_stub_trace1(stash);
	return func_stub(RPCCALL_PMOFRV(_rpc_pm_wakelocks_getcnt_riscv), 1, ARRAY_SIZE(args), args);
}


int rpc_pm_msgtorv_trigger_notify(int mode, int event)
{
	void *args[2] = {0};
	args[0] = (void *)mode;
	args[1] = (void *)event;
	pm_stub_trace2(mode, event);
	return func_stub(RPCCALL_PMOFRV(_rpc_pm_msgtorv_trigger_notify), 1, ARRAY_SIZE(args), args);
}

int rpc_pm_msgtorv_trigger_suspend(int mode)
{
	void *args[1] = {0};
	args[0] = (void *)mode;
	pm_stub_trace1(mode);
	return func_stub(RPCCALL_PMOFRV(_rpc_pm_msgtorv_trigger_suspend), 1, ARRAY_SIZE(args), args);
}

int rpc_pm_msgtorv_check_subsys_assert(int mode)
{
	void *args[1] = {0};
	args[0] = (void *)mode;
	pm_stub_trace1(mode);
	return func_stub(RPCCALL_PMOFRV(_rpc_pm_msgtorv_check_subsys_assert), 1, ARRAY_SIZE(args), args);
}

int rpc_pm_msgtorv_check_wakesrc_num(int type)
{
	void *args[1] = {0};
	args[0] = (void *)type;
	pm_stub_trace1(type);
	return func_stub(RPCCALL_PMOFRV(_rpc_pm_msgtorv_check_wakesrc_num), 1, ARRAY_SIZE(args), args);
}
