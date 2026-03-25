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

#include <../../hal/source/spinor/inter.h>

#include "sunxi_amp.h"
#include <hal_cache.h>

static int _nor_read(unsigned int addr, char *buf, unsigned int size)
{
	int ret;
    ret = nor_read(addr, buf, size);
    hal_dcache_clean((unsigned long)buf, size);
    return ret;
}

static int _nor_write(unsigned int addr, char *buf, unsigned int size)
{
    int ret;
    hal_dcache_invalidate((unsigned long)buf, size);
    ret = nor_write(addr, buf, size);
    hal_dcache_invalidate((unsigned long)buf, size);
    return ret;
}

static int _nor_erase(unsigned int addr, unsigned int size)
{
    return nor_erase(addr, size);
}

static int _nor_ioctrl(int cmd, void *buf, int size)
{
    int ret;
    ret = nor_ioctrl(cmd, buf, size);
    hal_dcache_clean((unsigned long)buf, size);
    return ret;
}

sunxi_amp_func_table flashc_table[] =
{
    {.func = (void *)&_nor_read, .args_num = 3, .return_type = RET_POINTER},
    {.func = (void *)&_nor_write, .args_num = 3, .return_type = RET_POINTER},
    {.func = (void *)&_nor_erase, .args_num = 2, .return_type = RET_POINTER},
    {.func = (void *)&_nor_ioctrl, .args_num = 3, .return_type = RET_POINTER},
};
