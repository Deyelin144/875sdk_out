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

#ifndef __SPI_SENSOR_H__
#define __SPI_SENSOR_H__

#include <stdio.h>
#include "sunxi_hal_twi.h"
#include <hal_waitqueue.h>
#include "hal_timer.h"
#include "hal_time.h"
#include "hal_log.h"
#include "hal_gpio.h"
#include "hal_mutex.h"
#include "hal/sdmmc/hal/hal_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define sensor_info(fmt, arg...) hal_log_info("[%s]%s():%d:" fmt, SENSOR_NAME, __func__, __LINE__, ##arg)
#define sensor_err(fmt, arg...) hal_log_err("[%s]%s():%d:" fmt, SENSOR_NAME, __func__, __LINE__, ##arg)

#define spi_sensor_info(fmt, arg...) hal_log_info("%s():%d:" fmt , __func__, __LINE__, ##arg)
#define spi_sensor_err(fmt, arg...) hal_log_err("%s():%d:" fmt, __func__, __LINE__, ##arg)

#define SPI_DEV_DBG_EN 0
#if (SPI_DEV_DBG_EN == 1)
#define spi_camera_dbg(fmt, arg...) hal_log_info("[SC_DBG]%s():%d:" fmt, __func__, __LINE__, ##arg)
#else
#define spi_camera_dbg(fmt, arg...)
#endif

#define spi_camera_info(fmt, arg...) hal_log_info("%s():%d:" fmt, __func__, __LINE__, ##arg)
#define spi_camera_err(fmt, arg...) hal_log_err("[SC_ERR]%s():%d:" fmt, __func__, __LINE__, ##arg)

#define REG_DLY             0xffff
#define TWI_BITS_8          8
#define TWI_BITS_16         16

#if CONFIG_DRIVER_SYSCONFIG
#define SPI_SENSOR_BOARD                "spi_board0"
#define SPI_SENSOR_MODALIAS             "modalias"
#define SPI_SENSOR_MAX_SPEED            "max_speed_hz"
#define SPI_SENSOR_TWI_ID               "sensor_twi_id"
#define SPI_SENSOR_TWI_ADDR             "sensor_twi_addr"
#define SPI_SENSOR_SPI_ID               "sensor_spi_id"
#define SPI_SENSOR_PWDN                 "sensor_pwdn"
#define SPI_SENSOR_IRQ                  "sensor_irq"
#endif

/* sensor cfg defult*/
#define SPI_SENSOR_MAX_SPEED_NONE       96000000
#define SPI_SENSOR_TWI_ID_NONE          TWI_MASTER_1
#define SPI_SENSOR_TWI_ADDR_NONE        0x6E
#define SPI_SENSOR_SPI_ID_NONE          1
#define SPI_SENSOR_PWDN_NONE            GPIO_PA13
#define SPI_SENSOR_IRQ_NONE             GPIO_PA20

#if defined CONFIG_SPI_CAMERA_BF3901
#define SENSOR_NAME "bf3901"
#define SPI_SENSOR_FUNC_INIT            hal_bf3901_init
#define SPI_SENSOR_FUNC_DEINIT          hal_bf3901_deinit
#define SPI_SENSOR_FUNC_WIN_SIZE        hal_bf3901_win_sizes
#endif

struct sensor_power_cfg {
    gpio_pin_t reset_pin;
    gpio_pin_t pwdn_pin;
};

struct sensor_config {
    struct sensor_power_cfg pwcfg;
    twi_port_t twi_port;
    uint8_t twi_addr;
};

typedef enum  {
    SENSOR_SET_CLK_POL    = 0,
    SENSOR_SET_OUTPUT_FMT,
    SENSOR_SET_PIXEL_SIZE,
    SENSOR_SET_SUBSAMP,
} SENSOR_IoctrlCmd;

struct sensor_function {
    struct sensor_win_size *sensor_win_sizes;
    struct spi_sensor_win_size *sensor_spi_sizes;
    HAL_Status (*init)(struct sensor_config *cfg);
    HAL_Status (*deinit)(struct sensor_config *cfg);
    void (*g_mbus_config)(unsigned int *flags);
    void (*suspend)(void);
    void (*resume)(void);
};

struct regval_list {
    unsigned short addr;
    unsigned short data;
};

struct spi_cfg_param {
    uint16_t hor_len;
    uint16_t hor_start;
    uint16_t ver_len;
    uint16_t ver_start;
};

typedef struct {
    uint8_t cap_mode;    /* 0:still, 1:video */
    uint8_t cap_start;
    hal_mutex_t lock;
    struct sensor_function sensor_func;
    struct sensor_config sensor_cfg;
    struct spi_cfg_param spi_cfg;
    uint32_t irq;
} spi_private;

typedef struct {
    twi_port_t twi_port;
    unsigned char addr_width;
    unsigned char data_width;
    unsigned char slave_addr;
    hal_clk_t mclk;
    hal_clk_t mclk_src;
    unsigned int mclk_rate;
    char name[32];

} sensor_private;

struct sensor_win_size {
    unsigned int width;
    unsigned int height;
    unsigned int hoffset;
    unsigned int voffset;
    unsigned int mbus_code;
};

struct spi_sensor_win_size {
    int width;
    int height;
    void * regs;
    int regs_size;
    int code;
};

extern struct sensor_win_size hal_bf3901_win_sizes;
HAL_Status hal_bf3901_init(struct sensor_config *cfg);
HAL_Status hal_bf3901_deinit(struct sensor_config *cfg);

twi_status_t spi_sensor_twi_init(twi_port_t port, unsigned char sccb_id);
twi_status_t spi_sensor_twi_exit(twi_port_t port);

int spi_sensor_read(sensor_private *sensor_priv, unsigned short addr, unsigned short *value);
int spi_sensor_write(sensor_private *sensor_priv, unsigned short addr, unsigned short value);
int spi_sensor_write_array(sensor_private *sensor_priv, struct regval_list *regs, int array_size);
void spi_set_mclk_rate(sensor_private *sensor_priv, unsigned int clk_rate);
int spi_set_mclk(sensor_private *sensor_priv, unsigned int on);

#ifdef __cplusplus
}
#endif
#endif
