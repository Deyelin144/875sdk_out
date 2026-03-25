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

#include <FreeRTOS.h>
#include <stdio.h>
#include <string.h>
#include <hal_interrupt.h>
#include <sunxi_hal_gpadc.h>
#include <sunxi_hal_spi.h>
#include <sunxi_hal_power_protect.h>
#include "platform/gpadc_sun20iw2.h"
#include "../spi/platform_spi.h"
#include "../spi/platform/spi_sun20iw2.h"

#define ADC_CHANNEL                     GP_CH_8

#define POWER_PROTECT_DEBUG
#ifdef POWER_PROTECT_DEBUG
#define pp_info(fmt, args...)  printf("%s()%d - "fmt, __func__, __LINE__, ##args)
#else
#define pp_info(fmt, args...)
#endif
#define VOLTAGE_STABLE_THRESHOLD_MS     10      // 电压稳定阈值时间(ms)

static int flash_ret = 0;
static int gpadc_power_protect_irq_callback(uint32_t data_type, uint32_t data)
{
	uint32_t vol_data;
	uint8_t ch_num;
	static int cnt = 0;
	static int low_power_cnt = 0;
	int i = 0;
	hal_gpadc_channel_t channal;

#ifdef POWER_PROTECT_DEBUG
	data = ((VOL_RANGE / 4096) * data); /* 12bits sample rate */
	vol_data = data / 1000;
	pp_info("channel %d type: %s vol data: %u, raw: %u\n", 
		ADC_CHANNEL, data_type == GPADC_DOWN ? "DOWN" : "UP", vol_data, data);
	if (data_type == GPADC_DOWN) {
		pp_info("电压下降触发保护: 请注意!!!!!!!!!\n");
	}
#endif
	
	if (data_type == GPADC_DOWN) {
		// pp_info("channel %d vol data: %u\n", ADC_CHANNEL, vol_data);
		gpadc_channel_enable_highirq(ADC_CHANNEL);
		flash_ret = 1;
		low_power_cnt++;

		if (low_power_cnt > 3) {
			pp_info("触发低电压回调.\n");
			low_power_cnt = 0;
			// cmd_reboot(1, NULL);
		}
#ifdef CONFIG_DRIVERS_NAND_FLASH
		// 注释掉deinit spi的操作，改为立即复位，不然会导致应用操作spi的地方异常，然后出现死机
		// for (i = 0; i < SPI_MAX_NUM; i++) {
		// 	sunxi_spi_t *sspi = sunxi_get_spi(i);
		// 	sunxi_spi_hw_exit(sspi);
		// }
#endif
	} else if (data_type == GPADC_UP) {
		// pp_info("channel %d vol data: %u\n", ADC_CHANNEL, vol_data);
		flash_ret = 0;
		cnt++;
		if (cnt > 2) {
			gpadc_channel_disable_highirq(ADC_CHANNEL);
			cnt = 0;
		}
		low_power_cnt = 0;
	}
	return 0;
}

int get_flash_stat()
{
	return flash_ret;
}

int sunxi_power_protect_init(void)
{
	int i, ret = -1;
	hal_gpadc_init();
	hal_gpadc_channel_init(ADC_CHANNEL);
	ret = hal_gpadc_register_callback(ADC_CHANNEL, gpadc_power_protect_irq_callback);
	gpadc_channel_enable_lowirq(ADC_CHANNEL);

	return ret;
}
