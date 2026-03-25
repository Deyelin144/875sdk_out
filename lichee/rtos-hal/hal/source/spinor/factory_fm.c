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

/* all about fm factory */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <hal_timer.h>

#include "inter.h"

#define NOR_FM_QE_BIT BIT(1)
#define NOR_FM_WPS_BIT BIT(3)
/* sr1 here actually is sr2 in datasheet */
#define NOR_FM_CMD_READ_SR1 (0x35)
#define NOR_FM_CMD_BLK_LOCK_STATUS (0x3D)
#define NOR_FM_CMD_BLK_LOCK (0x36)
#define NOR_FM_CMD_BLK_UNLOCK (0x39)

static struct nor_info idt_fm[] =
{
    {
        .model = "fm25w128",
        .id = {0xa1, 0x28, 0x18},
        .total_size = SZ_16M,
        .flag = SUPPORT_GENERAL | SUPPORT_INDIVIDUAL_PROTECT,
    },
    {
        .model = "fm25q128a",
        .id = {0xa1, 0x40, 0x18},
        .total_size = SZ_16M,
        .flag = SUPPORT_GENERAL | SUPPORT_INDIVIDUAL_PROTECT,
    },
};

static int nor_fm_read_status1(unsigned char *sr1)
{
    int ret;
    char cmd[1] = {NOR_FM_CMD_READ_SR1};
    char reg[2] = {0};

    ret = nor_transfer(1, cmd, 1, reg, 2);
    if (ret) {
        SPINOR_ERR("read fm status1 register fail\n");
        return ret;
    }

    *sr1 = reg[1];
    return 0;
}

static int nor_fm_quad_mode(struct nor_flash *unused)
{
    int ret;
    unsigned char sr[2];

    ret = nor_fm_read_status1(&sr[1]);
    if (ret)
        return ret;

    if (sr[1] & NOR_FM_QE_BIT)
        return 0;

    sr[1] |= NOR_FM_QE_BIT;

    ret = nor_read_status(&sr[0]);
    if (ret)
        return ret;

    ret = nor_write_status(sr, 2);
    if (ret)
        return ret;

    ret = nor_fm_read_status1(&sr[1]);
    if (ret)
        return ret;
    if (!(sr[1] & NOR_FM_QE_BIT)) {
        SPINOR_ERR("set fm QE failed (0x%x)\n", sr[1]);
        return -EINVAL;
    }
    return 0;
}

static int nor_fm_set_wps(int wps)
{
    int ret;
    unsigned char sr[2];
    unsigned char sr1;

    ret = nor_fm_read_status1(&sr1);
    if (ret)
        return ret;

    if (wps)
        sr[1] = sr1 | NOR_FM_WPS_BIT;
    else
        sr[1] = sr1 & ~NOR_FM_WPS_BIT;

    if (sr1 == sr[1])
        return 0;

    ret = nor_read_status(&sr[0]);
    if (ret)
        return ret;

    ret = nor_write_status(sr, 2);
    if (ret)
        return ret;

    ret = nor_fm_read_status1(&sr1);
    if (ret)
        return ret;

    if ((sr1 & NOR_FM_WPS_BIT) != (sr[1] & NOR_FM_WPS_BIT)) {
        SPINOR_ERR("set fm wps %d failed (0x%x)\n", wps, sr1);
        return -EINVAL;
    }
    return 0;
}

static int nor_fm_init_lock(struct nor_flash *nor)
{
    return nor_fm_set_wps(true);
}

static void nor_fm_deinit_lock(struct nor_flash *nor)
{
    nor_fm_set_wps(false);
}

static bool nor_fm_blk_islock(unsigned int addr)
{
    int ret;
    unsigned char tbuf[4], st;

    tbuf[0] = NOR_FM_CMD_BLK_LOCK_STATUS;
    tbuf[1] = addr >> 16;
    tbuf[2] = addr >> 8;
    tbuf[3] = addr & 0xFF;
    ret = nor_transfer(4, tbuf, 4, &st, 1);
    if (ret)
        return ret;
    return st & 0x1 ? true : false;
}

static int nor_fm_blk_unlock(unsigned int addr)
{
    int ret;
    unsigned char tbuf[4];

    ret = nor_write_enable();
    if (ret)
        return ret;

    tbuf[0] = NOR_FM_CMD_BLK_UNLOCK;
    tbuf[1] = addr >> 16;
    tbuf[2] = addr >> 8;
    tbuf[3] = addr & 0xFF;
    ret = nor_transfer(4, tbuf, 4, NULL, 0);
    if (ret)
        return ret;
    if (nor_fm_blk_islock(addr) == true)
        return -EBUSY;
    return 0;
}

static int nor_fm_blk_lock(unsigned int addr)
{
    int ret;
    unsigned char tbuf[4];

    ret = nor_write_enable();
    if (ret)
        return ret;

    tbuf[0] = NOR_FM_CMD_BLK_LOCK;
    tbuf[1] = addr >> 16;
    tbuf[2] = addr >> 8;
    tbuf[3] = addr & 0xFF;
    ret = nor_transfer(4, tbuf, 4, NULL, 0);
    if (ret)
        return ret;
    if (nor_fm_blk_islock(addr) == true)
        return 0;
    return -EBUSY;
}

static int nor_fm_lock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return nor_fm_blk_lock(addr);
}

static int nor_fm_unlock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return nor_fm_blk_unlock(addr);
}

static bool nor_fm_islock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return nor_fm_blk_islock(addr);
}

static struct nor_factory nor_fm = {
    .factory = FACTORY_FM,
    .idt = idt_fm,
    .idt_cnt = sizeof(idt_fm),

    .init = NULL,
    .deinit = NULL,
    .init_lock = nor_fm_init_lock,
    .deinit_lock = nor_fm_deinit_lock,
    .lock = nor_fm_lock,
    .unlock = nor_fm_unlock,
    .islock = nor_fm_islock,
    .set_quad_mode = nor_fm_quad_mode,
    .set_4bytes_addr = NULL,
};

int nor_register_factory_fm(void)
{
    return nor_register_factory(&nor_fm);
}
