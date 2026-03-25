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

#include "sunxi_amp.h"
#include <hal_cache.h>

int rv_misc_ioctrl(int cmd, void *data, int data_len)
{
    int ret;
    void *args[3] = {0};

    args[0] = (void *)(unsigned long)cmd;
    args[1] = data;
    args[2] = (void *)(unsigned long)data_len;

    hal_dcache_clean((unsigned long)data, data_len);
    ret = func_stub(RPCCALL_RV_MISC(misc_ioctrl), 1, 3, (void *)&args);
    hal_dcache_invalidate((unsigned long)data, data_len);

    return ret;
}

int m33_misc_ioctrl(int cmd, void *data, int data_len)
{
    int ret;
    void *args[3] = {0};

    args[0] = (void *)(unsigned long)cmd;
    args[1] = data;
    args[2] = (void *)(unsigned long)data_len;

    hal_dcache_clean((unsigned long)data, data_len);
    ret = func_stub(RPCCALL_M33_MISC(misc_ioctrl), 1, 3, (void *)&args);
    hal_dcache_invalidate((unsigned long)data, data_len);

    return ret;
}

int dsp_misc_ioctrl(int cmd, void *data, int data_len)
{
    int ret;
    void *args[3] = {0};

    args[0] = (void *)(unsigned long)cmd;
    args[1] = data;
    args[2] = (void *)(unsigned long)data_len;

    hal_dcache_clean((unsigned long)data, data_len);
    ret = func_stub(RPCCALL_DSP_MISC(misc_ioctrl), 1, 3, (void *)&args);
    hal_dcache_invalidate((unsigned long)data, data_len);

    return ret;
}
