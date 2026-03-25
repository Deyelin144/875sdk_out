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

#include "video_stack.h"
#include "stdlib.h"
#include <stdio.h>
#include "sunxi_hal_common.h"
#include "video_debug/vd_log.h"

int mrb_open(MemHandle *mp, uint32_t period_size, uint32_t period_count)
{
    if (mp == NULL) {
        return -1;
    }

    uint64_t buff_size = period_size * period_count;
    // maybe use for DMA
    uint8_t *buff = hal_malloc_coherent(buff_size);
    if (buff == NULL) {
        vlog_error("malloc fail.");
        goto stack_malloc_fail;
    }

    SemaphoreHandle_t p_sem = xSemaphoreCreateCounting(1, 0);
    if (p_sem == NULL)
        goto stack_dmux_fail;

    SemaphoreHandle_t d_mux = xSemaphoreCreateMutex();
    if (d_mux == NULL)
        goto stack_pmux_fail;

    SemaphoreHandle_t r_sem = xSemaphoreCreateCounting(period_count, 0);
    if (r_sem == NULL)
        goto stack_wmux_fail;

    SemaphoreHandle_t w_sem = xSemaphoreCreateCounting(period_count, period_count);
    if (w_sem == NULL)
        goto stack_wsem_fail;

    mp->addr = buff;
    mp->mem_count = period_count;
    mp->mem_len = period_size;
    mp->avail = 0;
    mp->wp = 0;
    mp->rp = 0;
    mp->pause = 0;
    mp->p_sem = p_sem;
    mp->d_mutex = d_mux;
    mp->r_sem = r_sem;
    mp->w_sem = w_sem;
    return 0;

stack_wsem_fail:
    vSemaphoreDelete(r_sem);
stack_wmux_fail:
    vSemaphoreDelete(d_mux);
stack_pmux_fail:
    vSemaphoreDelete(p_sem);
stack_dmux_fail:
    hal_free_coherent(buff);
stack_malloc_fail:
    return -1;
}

int mrb_isEmpty(MemHandle *mp)
{
    if (mp == NULL) {
        return -1;
    }

    return ((mp->avail == 0) ? 1 : 0);
}

int mrb_isFull(MemHandle *mp)
{
    if (mp == NULL) {
        return -1;
    }

    return ((mp->avail == mp->mem_count) ? 1 : 0);
}

void mrb_close(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return;

    hal_free_coherent(mp->addr);
    mp->addr = NULL;

    vSemaphoreDelete(mp->d_mutex);
    vSemaphoreDelete(mp->r_sem);
    vSemaphoreDelete(mp->w_sem);
}

void mrb_pause(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return;

    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        mp->pause = 1;
        xSemaphoreGive(mp->d_mutex);
        printf("pause %p\n", mp);
    }
}

void mrb_continue(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return;

    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        if (mp->pause == 1) {
            mp->pause = 0;
        }
        xSemaphoreGive(mp->d_mutex);
        xSemaphoreGive(mp->p_sem);
        printf("continue %p\n", mp);
    }
}

void *mrb_write_data(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return NULL;

    uint8_t *r_buff = NULL;

    xSemaphoreTake(mp->w_sem, portMAX_DELAY);
    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        r_buff = (mp->addr + (mp->wp * mp->mem_len));
        xSemaphoreGive(mp->d_mutex);
        return (void *)r_buff;
    } else {
        return NULL;
    }
}

int mrb_write_finish(MemHandle *mp)
{
    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        mp->avail++;
        mp->wp++;
        if (mp->wp >= mp->mem_count) {
            mp->wp = 0;
        }
        xSemaphoreGive(mp->d_mutex);
        xSemaphoreGive(mp->r_sem);
        return 0;
    } else {
        return -1;
    }
}


void *mrb_insert_data_lock(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return NULL;

    uint8_t *r_buff = NULL;

    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        printf("%s %d\n", __func__, __LINE__);
        r_buff = (mp->addr + (mp->rp * mp->mem_len));
        return (void *)r_buff;
    } else {
        return NULL;
    }
}


void mrb_insert_data_unlock(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return;
    printf("%s %d\n", __func__, __LINE__);
    xSemaphoreGive(mp->d_mutex);
}

void *mrb_read_data(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return NULL;

    uint8_t *r_buff = NULL;

    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        if (mp->pause == 1) {
            xSemaphoreGive(mp->d_mutex);
            xSemaphoreTake(mp->p_sem, portMAX_DELAY);
        } else {
            xSemaphoreGive(mp->d_mutex);
        }
        
    }

    xSemaphoreTake(mp->r_sem, portMAX_DELAY);
    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        r_buff = (mp->addr + (mp->rp * mp->mem_len));
        xSemaphoreGive(mp->d_mutex);
        return (void *)r_buff;
    } else {
        return NULL;
    }
}

int mrb_read_finish(MemHandle *mp)
{
    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        mp->avail--;
        mp->rp++;
        if (mp->rp >= mp->mem_count) {
            mp->rp = 0;
        }
        xSemaphoreGive(mp->d_mutex);
        xSemaphoreGive(mp->w_sem);
        return 0;
    } else {
        return -1;
    }
}

void mrb_reset(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return;

    uint8_t pause = 0;

    if (xSemaphoreTake(mp->d_mutex, portMAX_DELAY) == pdTRUE) {
        pause = mp->pause;
        mp->avail = 0;
        mp->wp = 0;
        mp->rp = 0;
        mp->pause = 0;
        while(xSemaphoreTake(mp->r_sem, 0) == pdTRUE);
        while(xSemaphoreTake(mp->w_sem, 0) == pdTRUE);
        for (int i = 0; i < mp->mem_count; i++) {
            xSemaphoreGive(mp->w_sem);
        }
        xSemaphoreGive(mp->d_mutex);
        if (pause) {
            xSemaphoreGive(mp->p_sem);
        }
    }
}

/******************************************** end of file ********************************************/
