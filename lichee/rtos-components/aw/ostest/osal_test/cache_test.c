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
#include <hal_log.h>
#include <hal_cmd.h>
#include <hal_mem.h>
#include <hal_cache.h>
#include <hal_dma.h>
#include <hal_osal.h>
#include <sunxi_hal_common.h>

#include "cache_test.h"

static void dma_test_cb(void *param)
{
}

int cmd_test_cache(int argc, char **argv)
{
    int ret, i;
    unsigned long *hdma = NULL;
    char *src = NULL, *dst = NULL;
    char *src_rev = NULL;
    char *dst_rev = NULL;
    struct dma_slave_config config = {0};
    uint32_t size = 0;
    int loop_cnt = 0;

    dst = hal_malloc_coherent(DCACHE_TEST_LEN);
    src = hal_malloc_coherent(DCACHE_TEST_LEN);

    dst_rev = hal_malloc_coherent(DCACHE_TEST_LEN);
    src_rev = hal_malloc_coherent(DCACHE_TEST_LEN);

    if (src == NULL || dst == NULL || dst_rev == NULL || src_rev == NULL)
    {
        printf("malloc src error!");
        ret = -CACHE_TEST_MALLOC_FAILED;
        goto end;
    }

    memset(src, 0, DCACHE_TEST_LEN);
    memset(dst, 0, DCACHE_TEST_LEN);

    for (i = 0; i < DCACHE_TEST_LEN; i++)
    {
        src[i] = i & 0xff;
    }

    memcpy(src_rev, src, DCACHE_TEST_LEN);
    memcpy(dst_rev, dst, DCACHE_TEST_LEN);

    hal_dcache_clean((unsigned long)src, DCACHE_TEST_LEN);

    /* request dma chan */
    ret = hal_dma_chan_request((struct sunxi_dma_chan **)&hdma);
    if (ret == -HAL_DMA_CHAN_STATUS_BUSY)
    {
        printf("dma channel busy!");
        ret = -CACHE_TEST_DMA_BUSY;
        goto end;
    }

    /* register dma callback */
    ret = hal_dma_callback_install((struct sunxi_dma_chan *)hdma, dma_test_cb, (struct sunxi_dma_chan *)hdma);
    if (ret != HAL_DMA_STATUS_OK)
    {
        printf("register dma callback failed!");
        ret = -CACHE_TEST_DMA_CALLBACK_FAILED;
        hal_dma_chan_free((struct sunxi_dma_chan *)hdma);
        goto end;
    }

    config.direction = DMA_MEM_TO_MEM;
    config.dst_addr_width = DMA_SLAVE_BUSWIDTH_8_BYTES;
    config.src_addr_width = DMA_SLAVE_BUSWIDTH_8_BYTES;
    config.dst_maxburst = DMA_SLAVE_BURST_16;
    config.src_maxburst = DMA_SLAVE_BURST_16;
    config.slave_id = sunxi_slave_id(DRQDST_SDRAM, DRQSRC_SDRAM);

    ret = hal_dma_slave_config((struct sunxi_dma_chan *)hdma, &config);

    if (ret != HAL_DMA_STATUS_OK)
    {
        printf("dma config error, ret:%d", ret);
        ret = -CACHE_TEST_DMA_CONFIG_ERR;
        hal_dma_chan_free((struct sunxi_dma_chan *)hdma);
        goto end;
    }

    ret = hal_dma_prep_memcpy((struct sunxi_dma_chan *)hdma, (uintptr_t)dst, (uintptr_t)src, DCACHE_TEST_LEN);
    if (ret != HAL_DMA_STATUS_OK)
    {
        printf("dma prep error, ret:%d", ret);
        ret = -CACHE_TEST_DMA_PREP_ERR;
        hal_dma_chan_free((struct sunxi_dma_chan *)hdma);
        goto end;
    }

    ret = hal_dma_start((struct sunxi_dma_chan *)hdma);
    if (ret != HAL_DMA_STATUS_OK)
    {
        printf("dma start error, ret:%d", ret);
        ret = -CACHE_TEST_DMA_START_ERR;
        hal_dma_chan_free((struct sunxi_dma_chan *)hdma);
        goto end;
    }

    while (hal_dma_tx_status((struct sunxi_dma_chan *)hdma, &size) != 0) {
        loop_cnt ++;
        if (loop_cnt == 10000) {
            printf("%s(%d) dms wait timeout\n", __func__, __LINE__);
            ret = -CACHE_TEST_DMA_WAIT_ERR;
            hal_dma_chan_free((struct sunxi_dma_chan *)hdma);
            goto end;
        }
		hal_msleep(10);
    }

    ret = hal_dma_stop((struct sunxi_dma_chan *)hdma);
    if (ret != HAL_DMA_STATUS_OK)
    {
        printf("dma stop error, ret:%d", ret);
        ret = -CACHE_TEST_DMA_STOP_ERR;
        hal_dma_chan_free((struct sunxi_dma_chan *)hdma);
        goto end;
    }

    ret = hal_dma_chan_free((struct sunxi_dma_chan *)hdma);
    if (ret != HAL_DMA_STATUS_OK)
    {
        printf("dma free error, ret:%d", ret);
        ret = -CACHE_TEST_DMA_FREE_ERR;
        goto end;
    }

    if (!memcmp(dst, src, DCACHE_TEST_LEN))
    {
        printf("test1: meet error, dcache maybe not open or dma work faild !!!!\n");
        printf("src buf:\n");
        for (i = 0; i < DCACHE_TEST_LEN; i++)
        {
            printf("0x%x ", src[i]);
        }

        printf("\ndst buf:\n");
        for (i = 0; i < DCACHE_TEST_LEN; i++)
        {
            printf("0x%x ", dst[i]);
        }
        printf("\n\n");
        goto end;
    }
    else
    {
        for (i = 0; i < DCACHE_TEST_LEN; i++) {
            if (dst[i] == 0) {
                continue;
            } else {
                printf("test1: data error, dcache updata!\n");
                ret = -CACHE_TEST_FAILED;
                goto end;
            }

        }
        printf("test1: dcache open ok, src & dst data all 0\n");
    }

    hal_dcache_invalidate((unsigned long)src, DCACHE_TEST_LEN);
    hal_dcache_invalidate((unsigned long)dst, DCACHE_TEST_LEN);

    if (memcmp(dst, src, DCACHE_TEST_LEN))
    {
        printf("test2: meet error, invalidate dcache failed !!!!\n");
        printf("src buf:\n");
        for (i = 0; i < DCACHE_TEST_LEN; i++)
        {
            printf("0x%x ", src[i]);
        }

        printf("\ndst buf:\n");
        for (i = 0; i < DCACHE_TEST_LEN; i++)
        {
            printf("0x%x ", dst[i]);
        }
        printf("\n\n");
        ret = -CACHE_TEST_FAILED;
        goto end;
    }
    else
    {
        for (i = 0; i < DCACHE_TEST_LEN; i++) {
            if (dst[i] == src[i]){
                continue;
            } else {
                printf("test2: data error, dst[%d]:%x data error!\n", i, dst[i]);
                for (i = 0; i < DCACHE_TEST_LEN; i++){
                    printf("0x%x ", dst[i]);
                }
                ret = -CACHE_TEST_FAILED;
                goto end;
            }

        }
    printf("test2: dcache clean & invalidate range successfully!\n");
    ret = CACHE_TEST_RET_OK;
    }

	printf("Cache test success!\n");

end:
    if (src)
    {
        hal_free_coherent(src);
    }

    if (dst)
    {
        hal_free_coherent(dst);
    }

    return ret;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_test_cache, test_cache, cache tests);
