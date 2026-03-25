/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the People's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.
 *
 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY?ˉS TECHNOLOGY (SONY, DTS, DOLBY, AVS OR
 * MPEGLA, ETC.)
 * IN ALLWINNERS?ˉSDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT
 * TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY?ˉS TECHNOLOGY.
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

#ifndef __SENSOR__HELPER__H__
#define __SENSOR__HELPER__H__

#include <stdio.h>
#include "sunxi_hal_twi.h"
#include "hal_timer.h"
#include "hal_time.h"
#include "hal_log.h"
#include "sensor/camera_sensor.h"
#include "../csi.h"

#define REG_DLY             0xffff

#define TWI_BITS_8          8
#define TWI_BITS_16         16

typedef struct {
	twi_port_t twi_port;
	unsigned char addr_width;
	unsigned char data_width;
	unsigned char slave_addr;
	hal_clk_t mclk;
	hal_clk_t mclk_src;
	unsigned int mclk_rate;
	char name[32];
#ifdef CONFIG_PM
	uint8_t suspend;
	struct soc_device dev;
#endif
} sensor_private;


struct regval_list {
	unsigned short addr;
	unsigned short data;
};

#define SENSOR_DEV_DBG_EN   1
#if (SENSOR_DEV_DBG_EN == 1)
#define sensor_dbg(x, arg...) hal_log_info("[%s_debug]"x, SENSOR_NAME, ##arg)
#else
#define sensor_dbg(x, arg...)
#endif

#define sensor_err(x, arg...)  hal_log_err("[%s_err]"x, SENSOR_NAME, ##arg)
#define sensor_print(x, arg...) hal_log_info("[%s]"x, SENSOR_NAME, ##arg)

twi_status_t sensor_twi_init(twi_port_t port, unsigned char sccb_id);
twi_status_t sensor_twi_exit(twi_port_t port);

int sensor_read(sensor_private *sensor_priv, unsigned short addr, unsigned short *value);
int sensor_write(sensor_private *sensor_priv, unsigned short addr, unsigned short value);
int sensor_write_array(sensor_private *sensor_priv, struct regval_list *regs, int array_size);
void csi_set_mclk_rate(sensor_private *sensor_priv, unsigned int clk_rate);
int csi_set_mclk(sensor_private *sensor_priv, unsigned int on);

#endif

