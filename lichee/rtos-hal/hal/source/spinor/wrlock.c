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

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <hal_timer.h>
#include <hal_cmd.h>

#include "inter.h"

#define NOR_CMD_BLK_LOCK_ALL (0x7E)
#define NOR_CMD_BLK_UNLOCK_ALL (0x98)

/* #define DEBUG_WRLOCK */

#ifdef DEBUG_WRLOCK
int skip_lock = 0;
#endif

int nor_wr_lock_init(struct nor_flash *nor)
{
    if (nor->factory->init_lock)
        return nor->factory->init_lock(nor);
    return 0;
}

void nor_wr_lock_deinit(struct nor_flash *nor)
{
    if (nor->factory->deinit_lock)
        nor->factory->deinit_lock(nor);
}

bool nor_wr_islock(struct nor_flash *nor, unsigned int addr, unsigned int len)
{
    unsigned int align_size;
    unsigned int align_addr;
    bool ret = false;

    if (nor->factory->islock) {
        while (len) {
            /* the first and the last block (64K) is lock on sector (4K) unit */
            if (addr >= nor->total_size - SZ_64K || addr < SZ_64K)
                align_size = SZ_4K;
            else
                align_size = SZ_64K;
            align_addr = ALIGN_DOWN(addr, align_size);

            ret = nor->factory->islock(nor, align_addr, align_size);
            if (ret)
                break;

            len -= MIN(align_size, len);
            addr += MIN(align_size, len);
        }
    }
    return ret;
}

int nor_wr_lock(struct nor_flash *nor, unsigned int addr, unsigned int len)
{
    unsigned int align_size;
    unsigned int align_addr;
    int ret = 0;

#ifdef DEBUG_WRLOCK
    if (skip_lock == 1) {
        printf("skip handle lock\n");
        return 0;
    }
#endif

    if (nor->factory->lock) {
        while (len) {
            /* the first and the last block (64K) is lock on sector (4K) unit */
            if (addr >= nor->total_size - SZ_64K || addr < SZ_64K)
                align_size = SZ_4K;
            else
                align_size = SZ_64K;
            align_addr = ALIGN_DOWN(addr, align_size);

            ret = nor->factory->lock(nor, align_addr, align_size);
            if (ret)
                break;

            len -= MIN(align_size, len);
            addr += MIN(align_size, len);
        }
    }
    return ret;
}

int nor_wr_unlock(struct nor_flash *nor, unsigned int addr, unsigned int len)
{
    unsigned int align_size;
    unsigned int align_addr;
    int ret = 0;

#ifdef DEBUG_WRLOCK
    if (skip_lock == 1) {
        printf("skip handle lock\n");
        return 0;
    }
#endif

    if (nor->factory->unlock) {
        while (len) {
            /* the first and the last block (64K) is lock on sector (4K) unit */
            if (addr >= nor->total_size - SZ_64K || addr < SZ_64K)
                align_size = SZ_4K;
            else
                align_size = SZ_64K;
            align_addr = ALIGN_DOWN(addr, align_size);

            ret = nor->factory->unlock(nor, align_addr, align_size);
            if (ret)
                break;

            len -= MIN(align_size, len);
            addr += MIN(align_size, len);
        }
    }
    return ret;
}

int nor_wr_unlock_all(struct nor_flash *nor)
{
    int ret;

#ifdef DEBUG_WRLOCK
    if (skip_lock == 1) {
        printf("skip handle lock\n");
        return 0;
    }
#endif

    ret = nor_write_enable();
    if (ret)
        return ret;

    ret = nor_send_cmd(NOR_CMD_BLK_UNLOCK_ALL);
    if (ret) {
        SPINOR_ERR("unlock all block failed - %d\n", ret);
        return ret;
    }
    return 0;
}

int nor_wr_lock_all(struct nor_flash *nor)
{
    int ret;

#ifdef DEBUG_WRLOCK
    if (skip_lock == 1) {
        printf("skip handle lock\n");
        return 0;
    }
#endif

    ret = nor_write_enable();
    if (ret)
        return ret;

    ret = nor_send_cmd(NOR_CMD_BLK_LOCK_ALL);
    if (ret) {
        SPINOR_ERR("lock all block failed - %d\n", ret);
        return ret;
    }
    return 0;
}


#ifdef DEBUG_WRLOCK
int cmd_skip_lock(int argc, char ** argv)
{
    printf("skip lock\n");
    skip_lock = 1;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_skip_lock, skip_lock, skip lock)

int cmd_not_skip_lock(int argc, char ** argv)
{
    printf("not skip lock\n");
    skip_lock = 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_not_skip_lock, not_skip_lock, not skip lock)
#endif
