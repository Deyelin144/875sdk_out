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

/* all about winbond factory */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <hal_timer.h>

#include "inter.h"

#define NOR_WINBOND_CMD_BLK_LOCK_STATUS (0x3D)
#define NOR_WINBOND_CMD_BLK_LOCK (0x36)
#define NOR_WINBOND_CMD_BLK_UNLOCK (0x39)
#define NOR_WINBOND_CMD_WRITE_SR3 0x11
#define NOR_WINBOND_CMD_READ_SR3 0x15
#define NOR_WINBOND_WPS_MASK BIT(2)
#define NOR_WINBOND_RCV_BIT BIT(3)

static struct nor_info idt_winbond[] =
{
    {
        .model = "w25q64jv",
        .id = {0xef, 0x40, 0x17},
        .total_size = SZ_8M,
        .flag = SUPPORT_GENERAL,
    },
    {
        .model = "w25q128jv",
        .id = {0xef, 0x40, 0x18},
        .total_size = SZ_16M,
        .flag = SUPPORT_GENERAL | SUPPORT_INDIVIDUAL_PROTECT,
    },
};

static int nor_winbond_set_wps(int val)
{
    int ret;
    unsigned char tbuf[2], reg[2], sr3;

    /* read status register-3 */
    tbuf[0] = NOR_WINBOND_CMD_READ_SR3;
    ret = nor_transfer(1, tbuf, 1, reg, 2);
    if (ret) {
        SPINOR_ERR("winbond read status register-3 fail\n");
        return ret;
    }

    sr3 = reg[0];
    if (val)
        sr3 |= NOR_WINBOND_WPS_MASK;
    else
        sr3 &= ~NOR_WINBOND_WPS_MASK;

    /* already set before, no need to set again */
    if (sr3 == reg[0])
        return 0;

    /* write status regsiter-3 */
    ret = nor_write_enable();
    if (ret)
        return ret;
    tbuf[0] = NOR_WINBOND_CMD_WRITE_SR3;
    tbuf[1] = sr3;
    ret = nor_transfer(2, tbuf, 2, NULL, 0);
    if (ret)
        return ret;

    return nor_wait_ready(10, 100 * 1000);
}

static int nor_winbond_set_rcv(int val)
{
    int ret;
    unsigned char tbuf[2], reg[2], sr3;

    /* read status register-3 */
    tbuf[0] = NOR_WINBOND_CMD_READ_SR3;
    ret = nor_transfer(1, tbuf, 1, reg, 2);
    if (ret) {
        SPINOR_ERR("winbond read status register-3 fail\n");
        return ret;
    }

    sr3 = reg[0];
    if (val)
        sr3 |= NOR_WINBOND_RCV_BIT;
    else
        sr3 &= ~NOR_WINBOND_RCV_BIT;

    /* already set before, no need to set again */
    if (sr3 == reg[0])
        return 0;

    /* write status regsiter-3 */
    ret = nor_write_enable();
    if (ret)
        return ret;
    tbuf[0] = NOR_WINBOND_CMD_WRITE_SR3;
    tbuf[1] = sr3;
    ret = nor_transfer(2, tbuf, 2, NULL, 0);
    if (ret)
        return ret;

    return nor_wait_ready(10, 100 * 1000);
}

static int nor_winbond_init(struct nor_flash *nor)
{
    return nor_winbond_set_rcv(true);
}

static int nor_winbond_init_lock(struct nor_flash *nor)
{
    return nor_winbond_set_wps(true);
}

static void nor_winbond_deinit_lock(struct nor_flash *nor)
{
    nor_winbond_set_wps(false);
}

static bool nor_winbond_blk_islock(unsigned int addr)
{
    int ret;
    unsigned char tbuf[4], st;

    tbuf[0] = NOR_WINBOND_CMD_BLK_LOCK_STATUS;
    tbuf[1] = addr >> 16;
    tbuf[2] = addr >> 8;
    tbuf[3] = addr & 0xFF;
    ret = nor_transfer(4, tbuf, 4, &st, 1);
    if (ret)
        return ret;
    return st & 0x1 ? true : false;
}

static int nor_winbond_blk_unlock(unsigned int addr)
{
    int ret;
    unsigned char tbuf[4];

    ret = nor_write_enable();
    if (ret)
        return ret;

    tbuf[0] = NOR_WINBOND_CMD_BLK_UNLOCK;
    tbuf[1] = addr >> 16;
    tbuf[2] = addr >> 8;
    tbuf[3] = addr & 0xFF;
    ret = nor_transfer(4, tbuf, 4, NULL, 0);
    if (ret)
        return ret;
    if (nor_winbond_blk_islock(addr) == true)
        return -EBUSY;
    return 0;
}

static int nor_winbond_blk_lock(unsigned int addr)
{
    int ret;
    unsigned char tbuf[4];

    ret = nor_write_enable();
    if (ret)
        return ret;

    tbuf[0] = NOR_WINBOND_CMD_BLK_LOCK;
    tbuf[1] = addr >> 16;
    tbuf[2] = addr >> 8;
    tbuf[3] = addr & 0xFF;
    ret = nor_transfer(4, tbuf, 4, NULL, 0);
    if (ret)
        return ret;
    if (nor_winbond_blk_islock(addr) == true)
        return 0;
    return -EBUSY;
}

static int nor_winbond_lock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return nor_winbond_blk_lock(addr);
}

static int nor_winbond_unlock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return nor_winbond_blk_unlock(addr);
}

static bool nor_winbond_islock(struct nor_flash *nor, unsigned int addr,
        unsigned int len)
{
    return nor_winbond_blk_islock(addr);
}

static struct nor_factory nor_winbond = {
    .factory = FACTORY_WINBOND,
    .idt = idt_winbond,
    .idt_cnt = sizeof(idt_winbond),

    .init = nor_winbond_init,
    .deinit = NULL,
    .init_lock = nor_winbond_init_lock,
    .deinit_lock = nor_winbond_deinit_lock,
    .lock = nor_winbond_lock,
    .unlock = nor_winbond_unlock,
    .islock = nor_winbond_islock,
    .set_quad_mode = NULL,
    .set_4bytes_addr = NULL,
};

int nor_register_factory_winbond(void)
{
    return nor_register_factory(&nor_winbond);
}
