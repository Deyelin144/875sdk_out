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

#ifndef __SPI_CAMERA_H__
#define __SPI_CAMERA_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sunxi_hal_twi.h"
#include <hal_waitqueue.h>
#include "hal_timer.h"
#include "hal_time.h"
#include "hal_log.h"
#include "hal/sdmmc/hal/hal_def.h"
#include "hal_atomic.h"

#include "hal_gpio.h"
#include "hal_mutex.h"
#include "hal_reset.h"
#include <hal_cmd.h>
#include <hal_mem.h>
#include <sunxi_hal_spi.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spi_fmt {
    unsigned int width;
    unsigned int height;
};

struct spi_ipeg_buf {
    unsigned int size;
    void *addr;
};

struct spi_ipeg_mem {
    unsigned char index;
    struct spi_ipeg_buf buf;

    struct list_head list;
};

typedef void (*SpiCapStatusCb)(struct spi_ipeg_mem *jpeg_mem);

struct spi_sensor_fmt {
    unsigned int width;
    unsigned int height;
    SpiCapStatusCb cb;
    unsigned char fps;
};

#define MAX_BUF_NUM 5
typedef struct {
    struct spi_ipeg_mem spi_mem[MAX_BUF_NUM];
    unsigned char spi_mem_count;
    struct list_head spi_active;
    struct list_head spi_done;
    bool spi_stream;
    int sem;

    hal_mutex_t lock;
    hal_spinlock_t slock;
    hal_waitqueue_head_t spi_waitqueue;
    hal_sem_t xSemaphore_rx;
    struct spi_sensor_fmt output_fmt;
} spi_camera_private;

HAL_Status hal_spi_sensor_probe(void);
HAL_Status hal_spi_sensor_remove(void);
HAL_Status hal_sensor_stream(unsigned int on);
void hal_spi_sensor_s_stream(unsigned int on);
int hal_spi_sensor_reqbuf(unsigned int count);
int hal_spi_sensor_freebuf(void);

struct spi_ipeg_mem *hal_spi_dqbuf(struct spi_ipeg_mem *spi_mem, unsigned int timeout_msec);
void hal_spi_qbuf(void);
HAL_Status hal_spi_set_fmt(struct spi_fmt *spi_output_fmt);
void disable_irq();
void enable_irq();

#ifdef __cplusplus
}
#endif
#endif
