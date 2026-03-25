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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <console.h>

#include "rpcconsole.h"
#include "sunxi_amp.h"
#include <hal_cache.h>

static int cmd_rpcconsole(int argc, char **argv)
{
    int ret = -1;
    int i;

    if (argc <= 2)
    {
        printf("Usage: rpccli [arm|dsp|rv] commandname [arg0 ...] \n");
        return -1;
    }
    if (argc > SUNXI_AMP_SH_MAX_CMD_ARGS)
    {
        printf("maximum number of arg:%d\n", SUNXI_AMP_SH_MAX_CMD_ARGS);
        return -1;
    }

    struct rpcconsole_arg_t *rpc_arg = amp_align_malloc(sizeof(struct rpcconsole_arg_t));
    if (rpc_arg == NULL)
    {
        printf("Alloc memory failed!\n");
        return -1;
    }
    memset(rpc_arg, 0, sizeof(struct rpcconsole_arg_t));

    char *rpc_args_cmd = amp_align_malloc(SH_MAX_CMD_LEN);
    if (rpc_args_cmd == NULL)
    {
        printf("Alloc memory failed!\n");
        amp_align_free(rpc_arg);
        return -1;
    }
    memset(rpc_args_cmd, 0, SH_MAX_CMD_LEN);

    rpc_arg->argv_cmd = rpc_args_cmd;
    rpc_arg->argc = argc - 2;

    for (i = 2; i < argc; i++)
    {
        rpc_arg->argv_32[i - 2] = (uint32_t)(unsigned long)rpc_args_cmd;
        rpc_arg->argv_64[i - 2] = (uint32_t)(unsigned long)rpc_args_cmd;
        memcpy(rpc_args_cmd, argv[i], strlen(argv[i]));
        rpc_args_cmd += ALIGN_UP((strlen(argv[i]) + 1), 4);
    }

    void *args[1] = {0};
    args[0] = rpc_arg;

    if (!strcmp("arm", argv[1]))
    {
        rpc_arg->flag = SEND_TO_32BIT;
        hal_dcache_clean((unsigned long)rpc_arg, sizeof(*rpc_arg));
        hal_dcache_clean((unsigned long)rpc_arg->argv_cmd, SH_MAX_CMD_LEN);

        ret = func_stub(RPCCALL_ARM_CONSOLE(exe_cmd), 1, 1, (void *)&args);
    }
    else if (!strcmp("dsp", argv[1]))
    {
        rpc_arg->flag = SEND_TO_32BIT;
        hal_dcache_clean((unsigned long)rpc_arg, sizeof(*rpc_arg));
        hal_dcache_clean((unsigned long)rpc_arg->argv_cmd, SH_MAX_CMD_LEN);

        ret = func_stub(RPCCALL_DSP_CONSOLE(exe_cmd), 1, 1, (void *)&args);
    }
    else if (!strcmp("rv", argv[1]))
    {
        rpc_arg->flag = SEND_TO_64BIT;
        hal_dcache_clean((unsigned long)rpc_arg, sizeof(*rpc_arg));
        hal_dcache_clean((unsigned long)rpc_arg->argv_cmd, SH_MAX_CMD_LEN);

        ret = func_stub(RPCCALL_RV_CONSOLE(exe_cmd), 1, 1, (void *)&args);
    }

    amp_align_free(rpc_arg->argv_cmd);
    amp_align_free(rpc_arg);

    return ret;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_rpcconsole, rpccli, exe);

int cmd_rpcconsole_ext(int argc, char **argv)
{
	return cmd_rpcconsole(argc, argv);
}
