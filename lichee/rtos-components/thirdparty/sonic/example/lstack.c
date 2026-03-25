/**
  *****************************************************************************
  * @file    lstack.c
  * @author  LHH
  * @version V1.0.0
  * @date    2021-03-15
  * @brief   build a special stack for mp4
  *****************************************************************************
  * @history
  *
  * 1. Date:2021-03-15
  *    Author:lhh
  *    Modification:build
  *
  *****************************************************************************
  */

#include "lstack.h"
#include "stdlib.h"
#include <stdio.h>
#include "sunxi_hal_common.h"

int mrb_open(MemHandle *mp, uint32_t period_size, uint32_t period_count)
{
    if (mp == NULL) {
        return -1;
    }

    uint64_t buff_size = period_size * period_count;
    // maybe use for DMA
    uint8_t *buff = hal_malloc_coherent(buff_size);
    if (buff == NULL) {
        printf("malloc fail.\n");
        goto stack_malloc_fail;
    }

    SemaphoreHandle_t d_mux = xSemaphoreCreateMutex();
    if (d_mux == NULL)
        goto stack_dmux_fail;

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
    mp->d_mutex = d_mux;
    mp->r_sem = r_sem;
    mp->w_sem = w_sem;
    return 0;

stack_wsem_fail:
    vSemaphoreDelete(r_sem);
stack_wmux_fail:
    vSemaphoreDelete(d_mux);
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

int mrb_write_drop(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return -1;

    xSemaphoreGive(mp->w_sem);
    return 0;
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

void *mrb_read_data(MemHandle *mp)
{
    if ((mp == NULL) || (mp->addr == NULL))
        return NULL;

    uint8_t *r_buff = NULL;

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

/******************************************** end of file ********************************************/
