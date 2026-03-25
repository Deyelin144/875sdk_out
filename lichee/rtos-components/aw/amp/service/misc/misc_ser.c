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
#include <barrier.h>

#include <hal_cache.h>

int dsp_misc_ioctrl(int cmd, void *data, int data_len);
int rv_misc_ioctrl(int cmd, void *data, int data_len);
int m33_misc_ioctrl(int cmd, void *data, int data_len);

static int _misc_ioctrl(int cmd, void *data, int data_len)
{
    int ret = 0;
    int *buffer;
    hal_dcache_invalidate((unsigned long)data, data_len);

    switch (cmd)
    {
        case AMP_MISC_CMD_RV_CALL_M33_STRESS_TEST:
        case AMP_MISC_CMD_DSP_CALL_M33_STRESS_TEST:
        case AMP_MISC_CMD_M33_CALL_RV_STRESS_TEST:
        case AMP_MISC_CMD_DSP_CALL_RV_STRESS_TEST:
        case AMP_MISC_CMD_M33_CALL_DSP_STRESS_TEST:
        case AMP_MISC_CMD_RV_CALL_DSP_STRESS_TEST:
            buffer = (int *)data;
            if ((*(buffer + 1) % 100) == 0)
                printf("%s receive cmd(%d) (%d:%d)\r\n", SELF_NAME, cmd, *buffer, *(buffer + 1));
            break;
        case AMP_MISC_CMD_RV_CALL_M33_CALL_DSP_STRESS_TEST:
#ifdef CONFIG_ARCH_ARM_CORTEX_M33
            dsp_misc_ioctrl(cmd, data, data_len);
#elif defined(CONFIG_ARCH_DSP)
            buffer = (int *)data;
            if ((*(buffer + 1) % 100) == 0)
                printf("%s receive cmd(%d) (%d:%d)\r\n", SELF_NAME, cmd, *buffer, *(buffer + 1));
#endif
            break;
        case AMP_MISC_CMD_RV_CALL_M33_CALL_RV_STRESS_TEST:
#ifdef CONFIG_ARCH_ARM_CORTEX_M33
            rv_misc_ioctrl(cmd, data, data_len);
#elif defined(CONFIG_ARCH_RISCV_C906)
            buffer = (int *)data;
            if ((*(buffer + 1) % 100) == 0)
                printf("%s receive cmd(%d) (%d:%d)\r\n", SELF_NAME, cmd, *buffer, *(buffer + 1));
#endif
            break;
        case AMP_MISC_CMD_RV_CALL_DSP_CALL_RV_STRESS_TEST:
#if defined(CONFIG_ARCH_DSP)
            rv_misc_ioctrl(cmd, data, data_len);
#elif defined(CONFIG_ARCH_RISCV_C906)
            buffer = (int *)data;
            if ((*(buffer + 1) % 100) == 0)
                printf("%s receive cmd(%d) (%d:%d)\r\n", SELF_NAME, cmd, *buffer, *(buffer + 1));
#endif
            break;
        case AMP_MISC_CMD_RV_CALL_M33_CALL_DSP_CALL_RV_STRESS_TEST:
#ifdef CONFIG_ARCH_ARM_CORTEX_M33
            dsp_misc_ioctrl(cmd, data, data_len);
#elif defined(CONFIG_ARCH_DSP)
            rv_misc_ioctrl(cmd, data, data_len);
#elif defined(CONFIG_ARCH_RISCV_C906)
            buffer = (int *)data;
            if ((*(buffer + 1) % 100) == 0)
                printf("%s receive cmd(%d) (%d:%d)\r\n", SELF_NAME, cmd, *buffer, *(buffer + 1));
#endif
            break;
        default:
            break;
    }
    hal_dcache_clean((unsigned long)data, data_len);

    return ret;
}

sunxi_amp_func_table misc_table[] =
{
    {.func = (void *) &_misc_ioctrl, .args_num = 3, .return_type = RET_POINTER},
};
