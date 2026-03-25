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

#include <console.h>

#include "rpcconsole.h"
#include "sunxi_amp.h"
#include <barrier.h>

#include <hal_cache.h>

static int _execute_cmd(struct rpcconsole_arg_t *rpc_arg)
{
    char *command_name;
    struct finsh_syscall *call;
    int ret = -1;

    hal_dcache_invalidate((unsigned long)rpc_arg, (unsigned long)sizeof(*rpc_arg));
#ifndef CONFIG_ARCH_DSP
    dsb();
    isb();
#endif
    hal_dcache_invalidate((unsigned long)rpc_arg->argv_cmd, SH_MAX_CMD_LEN);

    if (rpc_arg->flag == SEND_TO_32BIT) {
        command_name = (char *)(unsigned long)(rpc_arg->argv_32[0]);
    } else {
        command_name = (char *)(unsigned long)(rpc_arg->argv_64[0]);
    }

    call = finsh_syscall_lookup(command_name);
    if (call == NULL || call->func == NULL)
    {
        printf("The command(%s) no exist !\n", command_name);
        return -1;
    }

    if (rpc_arg->flag == SEND_TO_32BIT) {
        ret = call->func(rpc_arg->argc, (char **)(unsigned long)rpc_arg->argv_32);
    } else {
        ret = call->func(rpc_arg->argc, (char **)(unsigned long)rpc_arg->argv_64);
    }
    hal_dcache_invalidate((unsigned long)rpc_arg, (unsigned long)sizeof(*rpc_arg));
    hal_dcache_invalidate((unsigned long)rpc_arg->argv_cmd, SH_MAX_CMD_LEN);
    return ret;
}

sunxi_amp_func_table console_table[] =
{
    {.func = (void *)&_execute_cmd, .args_num = 1, .return_type = RET_POINTER},
};
