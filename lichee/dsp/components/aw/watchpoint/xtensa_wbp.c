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
#include <stdlib.h>
#include <string.h>

#include <bitops.h>

#include "aw_breakpoint.h"
#include "xtensa_wbp.h"

#include "xtensa/config/specreg.h"

#define RSR(reg, at)         asm volatile ("rsr %0, %1" : "=r" (at) : "i" (reg))
#define WSR(reg, at)         asm volatile ("wsr %0, %1" : : "r" (at), "i" (reg))
#define XSR(reg, at)         asm volatile ("xsr %0, %1" : "+r" (at) : "i" (reg))

#define RER(reg, at)         asm volatile ("rer %0, %1" : "=r" (at) : "r" (reg))

#define WITLB(at, as)        asm volatile ("witlb  %0, %1; \n isync \n " : : "r" (at), "r" (as))
#define WDTLB(at, as)        asm volatile ("wdtlb  %0, %1; \n dsync \n " : : "r" (at), "r" (as))

int monitor_mode_enabled(void)
{
    return 1;
}

int enable_monitor_mode(void)
{
    return 0;
}

int get_num_brp_resources(void)
{
    return 2;
}

int get_num_wrp_resources(void)
{
    return 2;
}

int xtensa_install_hw_breakpoint(int i, unsigned long addr)
{
    uint32_t en;

    if (i == 0)
    {
        WSR(IBREAKA_0, addr);
    }
    else if (i == 1)
    {
        WSR(IBREAKA_1, addr);
    }

    RSR(IBREAKENABLE, en);
    en |= BIT(i);
    WSR(IBREAKENABLE, en);
    return 0;
}

int xtensa_install_hw_watchpoint(enum gdb_bptype type, int i, unsigned long addr)
{
    uint32_t dbreakc = 0x3F;
    int x;

    for (x = 0; x < 7; x++)
    {
        if (4 == (size_t)(1U << x))
        {
            break;
        }
        dbreakc <<= 1;
    }

    dbreakc = (dbreakc & 0x3F);
    dbreakc |= BIT(27);

    switch (type)
    {
        case BP_WRITE_WATCHPOINT:
            dbreakc |= BIT(31);
            break;
        case BP_READ_WATCHPOINT:
            dbreakc |= BIT(30);
            break;
        case BP_ACCESS_WATCHPOINT:
            dbreakc |= BIT(31);
            dbreakc |= BIT(30);
            break;
        default:
            break;
    }

    if (i == 1)
    {
        WSR(DBREAKA_1, addr);
        WSR(DBREAKC_1, dbreakc);
    }
    else if (i == 0)
    {
        WSR(DBREAKA_0, addr);
        WSR(DBREAKC_0, dbreakc);
    }

    return 0;
}

void xtensa_uninstall_hw_watchpoint(int i)
{
    if (i == 1)
    {
        WSR(DBREAKA_1, 0);
        WSR(DBREAKC_1, 0);
    }
    else if (i == 0)
    {
        WSR(DBREAKA_0, 0);
        WSR(DBREAKC_0, 0);
    }
}

void xtensa_uninstall_hw_breakpoint(int i)
{
    uint32_t en = 0;
    uint32_t pc = 0;

    if (i == 0)
    {
        WSR(IBREAKA_0, pc);
    }
    else if (i == 1)
    {
        WSR(IBREAKA_1, pc);
    }

    RSR(IBREAKENABLE, en);
    en &= ~BIT(i);
    WSR(IBREAKENABLE, en);
}
