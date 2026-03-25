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

extern sunxi_amp_func_table fsys_table[];
extern sunxi_amp_func_table net_table[];
extern sunxi_amp_func_table DEMO_table[];
extern sunxi_amp_func_table console_table[];
extern sunxi_amp_func_table bt_table[];
extern sunxi_amp_func_table pmofm33_table[];
extern sunxi_amp_func_table pmofrv_table[];
extern sunxi_amp_func_table pmofdsp_table[];
extern sunxi_amp_func_table flashc_table[];
extern sunxi_amp_func_table misc_table[];
extern sunxi_amp_func_table audio_table[];
extern sunxi_amp_func_table rpdata_table[];
extern sunxi_amp_func_table tfm_table[];
extern sunxi_amp_func_table versioninfo_arm_table[];
extern sunxi_amp_func_table versioninfo_dsp_table[];
extern sunxi_amp_func_table jpeg_table[];
extern sunxi_amp_func_table xvid_table[];

sunxi_amp_func_table *func_table[] =
{
#ifdef CONFIG_AMP_FSYS_SERVICE
    (void *)&fsys_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_NET_SERVICE
    (void *)&net_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_BT_SERVICE
    (void *)&bt_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_DEMO_SERVICE
    (void *)&DEMO_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_CONSOLE_SERVICE
    (void *)&console_table,
    (void *)&console_table,
    (void *)&console_table,
#else
    NULL,
    NULL,
    NULL,
#endif
#ifdef CONFIG_AMP_PMOFM33_SERVICE
    (void *)&pmofm33_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_PMOFRV_SERVICE
    (void *)&pmofrv_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_PMOFDSP_SERVICE
    (void *)&pmofdsp_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_FLASHC_SERVICE
    (void *)&flashc_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_MISC_SERVICE
    (void *)&misc_table,
    (void *)&misc_table,
    (void *)&misc_table,
#else
    NULL,
    NULL,
    NULL,
#endif
#ifdef CONFIG_AMP_AUDIO_SERVICE
    (void *)&audio_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_RPDATA_SERVICE
    (void *)&rpdata_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_TFM_SERVICE
    (void *)&tfm_table,
#else
    NULL,
#endif
#if (defined CONFIG_ARCH_ARM_ARMV8M && !defined CONFIG_PROJECT_BUILD_RECOVERY)
    (void *)&versioninfo_arm_table,
#else
    NULL,
#endif
#ifdef CONFIG_ARCH_DSP
    (void *)&versioninfo_dsp_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_JPEG_SERVICE
    (void *)&jpeg_table,
#else
    NULL,
#endif
#ifdef CONFIG_AMP_XVID_SERVICE
    (void *)&xvid_table,
#else
    NULL,
#endif
};
