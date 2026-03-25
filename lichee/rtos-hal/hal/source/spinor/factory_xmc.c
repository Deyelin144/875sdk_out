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

/* all about xmc factory */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <hal_timer.h>

#include "inter.h"

#define NOR_XMC_QE_BIT BIT(1)
#define NOR_XMC_CMD_READ_SR2 (0x35)
#define NOR_XMC_CMD_WRITE_SR2 (0x31)

static struct nor_info idt_xmc[] =
{
    {
        .model = "XM25QH128CHIQ",
        .id = {0x20, 0x40, 0x18},
        .total_size = SZ_16M,
        .flag = SUPPORT_GENERAL,
    },
};

int nor_xmc_read_status2(unsigned char *sr)
{
    int ret;
    char cmd[1] = {NOR_XMC_CMD_READ_SR2};
    char reg[2] = {0};

    ret = nor_transfer(1, cmd, 1, reg, 2);
    if (ret) {
        SPINOR_ERR("read status register fail");
        return ret;
    }

    *sr = reg[1];
    return 0;
}

int nor_xmc_write_status2(unsigned char *sr, unsigned int len)
{
    int ret, i;
    char tbuf[5] = {0};

    ret = nor_write_enable();
    if (ret)
        return ret;

    if (len > 5)
        return -EINVAL;

    tbuf[0] = NOR_XMC_CMD_WRITE_SR2;
    for (i = 0; i < len; i++)
        tbuf[i + 1] = *(sr + i);
    i++;
    ret = nor_transfer(i, tbuf, i, NULL, 0);
    if (ret) {
        SPINOR_ERR("write status register fail");
        return ret;
    }

    return nor_wait_ready(10, MAX_WAIT_LOOP);
}

static int nor_xmc_set_wps(int val)
{
    return 0;
}

static int nor_xmc_init(struct nor_flash *nor)
{
    return 0;
}

static int nor_xmc_init_lock(struct nor_flash *nor)
{
    return nor_xmc_set_wps(true);
}

static void nor_xmc_deinit_lock(struct nor_flash *nor)
{
    nor_xmc_set_wps(false);
}

static int nor_xmc_quad_mode(struct nor_flash *unused)
{
    int ret;
    unsigned char sr;

    ret = nor_xmc_read_status2(&sr);
    if (ret)
    {
        return ret;
    }

    sr |= NOR_XMC_QE_BIT;
    ret = nor_xmc_write_status2(&sr, 1);
    if (ret)
    {
        return ret;
    }

    ret = nor_xmc_read_status2(&sr);
    if (ret)
    {
        return ret;
    }
    if (!(sr & NOR_XMC_QE_BIT))
    {
        SPINOR_ERR("set xmc QE failed (0x%x)\n", sr);
        return -EINVAL;
    }
    return 0;
}

static int nor_xmc_lock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return 0;
}

static int nor_xmc_unlock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return 0;
}

static bool nor_xmc_islock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return 1;
}

static struct nor_factory nor_xmc = {
    .factory = FACTORY_XMC,
    .idt = idt_xmc,
    .idt_cnt = sizeof(idt_xmc),

    .init = nor_xmc_init,
    .deinit = NULL,
    .init_lock = nor_xmc_init_lock,
    .deinit_lock = nor_xmc_deinit_lock,
    .lock = nor_xmc_lock,
    .unlock = nor_xmc_unlock,
    .islock = nor_xmc_islock,
    .set_quad_mode = nor_xmc_quad_mode,
    .set_4bytes_addr = NULL,
};

int nor_register_factory_xmc(void)
{
    return nor_register_factory(&nor_xmc);
}
