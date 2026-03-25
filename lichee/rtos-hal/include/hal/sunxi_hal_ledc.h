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

#ifndef __HAL_LEDC_H
#define __HAL_LEDC_H
#include "sunxi_hal_common.h"
#include "ledc/platform_ledc.h"
#include "ledc/common_ledc.h"
#include <hal_sem.h>
#include <hal_clk.h>
#include <hal_reset.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SUNXI_LEDC_FIFO_DEPTH 32
#define RESULT_COMPLETE 1
#define RESULT_ERR      2

enum ledc_output_mode_val {
	LEDC_OUTPUT_GRB = 0 << 6,
	LEDC_OUTPUT_GBR = 1 << 6,
	LEDC_OUTPUT_RGB = 2 << 6,
	LEDC_OUTPUT_RBG = 3 << 6,
	LEDC_OUTPUT_BGR = 4 << 6,
	LEDC_OUTPUT_BRG = 5 << 6
};

enum {
	DEBUG_INIT    = 1U << 0,
	DEBUG_SUSPEND = 1U << 1,
	DEBUG_INFO    = 1U << 2,
	DEBUG_INFO1   = 1U << 3,
	DEBUG_INFO2   = 1U << 4,
};

struct ledc_config {
	unsigned int led_count;
	unsigned int reset_ns;
	unsigned int t1h_ns;
	unsigned int t1l_ns;
	unsigned int t0h_ns;
	unsigned int t0l_ns;
	unsigned int wait_time0_ns;
	unsigned long long wait_time1_ns;
	unsigned int wait_data_time_ns;
	char *output_mode;
	unsigned int *align_dma_buf;
	unsigned int *data;
	unsigned int length;
};

enum ledc_irq_ctrl_reg {
	LEDC_TRANS_FINISH_INT_EN     = (1 << 0),
	LEDC_FIFO_CPUREQ_INT_EN      = (1 << 1),
	LEDC_WAITDATA_TIMEOUT_INT_EN = (1 << 3),
	LEDC_FIFO_OVERFLOW_INT_EN    = (1 << 4),
	LEDC_GLOBAL_INT_EN           = (1 << 5),
};

enum ledc_irq_status_reg {
	LEDC_TRANS_FINISH_INT     = (1 << 0),
	LEDC_FIFO_CPUREQ_INT      = (1 << 1),
	LEDC_WAITDATA_TIMEOUT_INT = (1 << 3),
	LEDC_FIFO_OVERFLOW_INT    = (1 << 4),
	LEDC_FIFO_FULL            = (1 << 16),
	LEDC_FIFO_EMPTY           = (1 << 17),
};

static u32 sunxi_ledc_regs_offset[] = {
	LEDC_CTRL_REG,
	LED_RST_TIMING_CTRL_REG,
	LED_T01_TIMING_CTRL_REG,
	LEDC_WAIT_TIME0_CTRL_REG,
	LEDC_WAIT_TIME1_CTRL_REG,
	LEDC_INTC_REG,
	LEDC_DATA_REG,
	LEDC_DMA_CTRL_REG,
};

struct sunxi_led {
	struct reset_control *reset;
	hal_clk_t mod_clk;
	hal_clk_t bus_clk;
	u8 result;
	struct ledc_config config;
	u32 regs_backup[ARRAY_SIZE(sunxi_ledc_regs_offset)];
};

int hal_ledc_init(void);
void hal_ledc_deinit(void);
int hal_ledc_trans_data(struct ledc_config *ledc);
void hal_ledc_clear_all_irq(void);
unsigned int hal_ledc_get_irq_status(void);
void hal_ledc_dma_callback(void *para);
void hal_ledc_reset(void);
int sunxi_led_init(void);
int sunxi_set_led_brightness(int led_num, unsigned int brightness);
int sunxi_set_all_led(int led_num, unsigned int brightness);
#ifdef __cplusplus
}
#endif
#endif
