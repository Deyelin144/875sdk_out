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

#include <hal_cache.h>
#include "sunxi_amp.h"
#include "pm_debug.h"
#include "pm_rpcfunc.h"

static int _rpc_pm_wakelocks_getcnt_dsp(int stash)
{
	pm_ser_trace1(stash);
	return rpc_pm_wakelocks_getcnt_dsp(!!stash);
}

static int _rpc_pm_msgtodsp_trigger_notify(int mode, int event)
{
	pm_ser_trace2(mode, event);
	return rpc_pm_msgtodsp_trigger_notify(mode, event);
}

static int _rpc_pm_msgtodsp_trigger_suspend(int mode)
{
	pm_ser_trace1(mode);
	return rpc_pm_msgtodsp_trigger_suspend(mode);
}

static int _rpc_pm_msgtodsp_check_subsys_assert(int mode)
{
	pm_ser_trace1(mode);
	return rpc_pm_msgtodsp_check_subsys_assert(mode);
}

static int _rpc_pm_msgtodsp_check_wakesrc_num(int type)
{
	pm_ser_trace1(type);
	return rpc_pm_msgtodsp_check_wakesrc_num(type);
}

sunxi_amp_func_table pmofdsp_table[] = {
	{.func = (void *)&_rpc_pm_wakelocks_getcnt_dsp, .args_num = 1, .return_type = RET_POINTER},
	{.func = (void *)&_rpc_pm_msgtodsp_trigger_notify, .args_num = 2, .return_type = RET_POINTER},
	{.func = (void *)&_rpc_pm_msgtodsp_trigger_suspend, .args_num = 1, .return_type = RET_POINTER},
    	{.func = (void *)&_rpc_pm_msgtodsp_check_subsys_assert, .args_num = 1, .return_type = RET_POINTER},
    	{.func = (void *)&_rpc_pm_msgtodsp_check_wakesrc_num, .args_num = 1, .return_type = RET_POINTER},
};
