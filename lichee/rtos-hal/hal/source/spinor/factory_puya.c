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

/* all about gd factory */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <hal_timer.h>

#include "inter.h"

#define NOR_PUYA_QE_BIT BIT(1)
#define NOR_PUYA_CMD_RDSR2 0x35
#define NOR_PUYA_CMD_WRSR2 0x31

static struct nor_info idt_puya[] =
{
    {
        .model = "P25Q64H",
        .id = {0x85, 0x60, 0x17},
        .total_size = SZ_8M,
        .flag = SUPPORT_GENERAL,
    },
    {
        .model = "P25Q80H",
        .id = {0x85, 0x60, 0x14},
        .total_size = SZ_1M,
        .flag = SUPPORT_ALL_ERASE_BLK,
    },
    {
        .model = "P25Q32H",
        .id = {0x85, 0x60, 0x16},
        .total_size = SZ_4M,
        .flag = SUPPORT_ALL_ERASE_BLK,
    },
    {
        .model = "py25q64ha",
        .id = {0x85, 0x20, 0x17},
        .total_size = SZ_8M,
        .flag = SUPPORT_GENERAL,
    },
    {
        .model = "p25q128ha",
        .id = {0x85, 0x20, 0x18},
        .total_size = SZ_16M,
        .flag = SUPPORT_GENERAL,
    },
    {
        .model = "p25q256ha",
        .id = {0x85, 0x20, 0x19},
        .total_size = SZ_32M,
        .flag = SUPPORT_GENERAL,
    },
};

static int nor_puya_quad_mode(struct nor_flash *unused)
{
    int ret;
    unsigned char cmd[3];
    char reg[2] = {0};

    cmd[0] = NOR_PUYA_CMD_RDSR2;
    ret = nor_transfer(1, cmd, 1, reg, 2);
    if (ret) {
        SPINOR_ERR("read status register2 fail\n");
        return ret;
    }

    if (reg[1] & NOR_PUYA_QE_BIT)
        return 0;

    ret = nor_write_enable();
    if (ret)
        return ret;

    cmd[0] = NOR_PUYA_CMD_WRSR2;
    cmd[1] = reg[1] | NOR_PUYA_QE_BIT;
    ret = nor_transfer(2, cmd, 2, NULL, 0);
    if (ret) {
        SPINOR_ERR("set status register fail\n");
        return ret;
    }

    if (nor_wait_ready(0, 500)) {
        SPINOR_ERR("wait set qd mode failed\n");
        return -EBUSY;
    }

	reg[0] = 0;
	reg[1] = 0;
    cmd[0] = NOR_PUYA_CMD_RDSR2;
    ret = nor_transfer(1, cmd, 1, reg, 2);
    if (ret) {
        SPINOR_ERR("read status register2 fail\n");
        return ret;
    }


    if (!(reg[1] & NOR_PUYA_QE_BIT)) {
        SPINOR_ERR("set gd QE failed\n");
        return -EINVAL;
    }
    return 0;
}

static struct nor_factory nor_puya = {
    .factory = FACTORY_PUYA,
    .idt = idt_puya,
    .idt_cnt = sizeof(idt_puya),

    .init = NULL,
    .deinit = NULL,
    .init_lock = NULL,
    .deinit_lock = NULL,
    .lock = NULL,
    .unlock = NULL,
    .islock = NULL,
    .set_quad_mode = nor_puya_quad_mode,
    .set_4bytes_addr = NULL,
};

int nor_register_factory_puya(void)
{
    return nor_register_factory(&nor_puya);
}
