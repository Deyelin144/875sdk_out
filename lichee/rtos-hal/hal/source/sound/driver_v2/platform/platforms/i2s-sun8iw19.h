/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
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
#ifndef	__SUN8IW19_I2S_H_
#define	__SUN8IW19_I2S_H_

#define	SUNXI_I2S_BASE (0x05090000)

#define I2S_NUM_MAX	2

/*------------------------ CLK CONFIG FOR NORMAL ---------------------------*/
#define SUNXI_I2S_CLK_PLL_AUDIO	HAL_CLK_PLL_AUDIO
#define SUNXI_I2S_CLK_PLL_AUDIO1	HAL_CLK_PLL_AUDIOX4
#define SUNXI_I2S_CLK_I2S_ASRC	0

#define SUNXI_I2S_CLK_I2S0		HAL_CLK_PERIPH_I2S0
#define SUNXI_I2S_CLK_BUS_I2S0	0
#define SUNXI_I2S_CLK_RST_I2S0	0

#define SUNXI_I2S_CLK_I2S1		HAL_CLK_PERIPH_I2S1
#define SUNXI_I2S_CLK_BUS_I2S1	0
#define SUNXI_I2S_CLK_RST_I2S1	0

#define SUNXI_I2S_CLK_I2S2		HAL_CLK_PERIPH_I2S2
#define SUNXI_I2S_CLK_BUS_I2S2	0
#define SUNXI_I2S_CLK_RST_I2S2	0

#define SUNXI_I2S_CLK_I2S3		0
#define SUNXI_I2S_CLK_BUS_I2S3	0
#define SUNXI_I2S_CLK_RST_I2S3	0

/*------------------------ PIN CONFIG FOR NORMAL ---------------------------*/
/* GPIO define */
#define I2S0_PIN_MCLK \
{.gpio_pin = GPIOH(0), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S0_PIN_BCLK \
{.gpio_pin = GPIOH(1), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S0_PIN_LRCK \
{.gpio_pin = GPIOH(2), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S0_PIN_DOUT \
{.gpio_pin = GPIOH(3), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S0_PIN_DIN	\
{.gpio_pin = GPIOH(4), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}

#define I2S1_PIN_MCLK \
{.gpio_pin = GPIOD(10), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S1_PIN_BCLK \
{.gpio_pin = GPIOD(11), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S1_PIN_LRCK \
{.gpio_pin = GPIOD(12), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S1_PIN_DOUT \
{.gpio_pin = GPIOD(13), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S1_PIN_DIN \
{.gpio_pin = GPIOD(16), .mux = 3, .driv_level = GPIO_DRIVING_LEVEL1}

/*
 * I2S Params
 *
 *I2S_master:
 *	1: SND_SOC_DAIFMT_CBM_CFM(codec clk & FRM master)		use
 *	2: SND_SOC_DAIFMT_CBS_CFM(codec clk slave & FRM master)		not use
 *	3: SND_SOC_DAIFMT_CBM_CFS(codec clk master & frame slave)	not use
 *	4: SND_SOC_DAIFMT_CBS_CFS(codec clk & FRM slave)		use
 *tdm_config:
 *	0 is pcm; 1 is i2s
 *audio_format:
 *	1:SND_SOC_DAIFMT_I2S(standard i2s format). use
 *	2:SND_SOC_DAIFMT_RIGHT_J(right justfied format).
 *	3:SND_SOC_DAIFMT_LEFT_J(left justfied format)
 *	4:SND_SOC_DAIFMT_DSP_A(pcm. MSB is available on 2nd BCLK rising edge after LRC rising edge). use
 *	5:SND_SOC_DAIFMT_DSP_B(pcm. MSB is available on 1nd BCLK rising edge after LRC rising edge)
 *signal_inversion:
 *	1:SND_SOC_DAIFMT_NB_NF(normal bit clock + frame)  use
 *	2:SND_SOC_DAIFMT_NB_IF(normal BCLK + inv FRM)
 *	3:SND_SOC_DAIFMT_IB_NF(invert BCLK + nor FRM)  use
 *	4:SND_SOC_DAIFMT_IB_IF(invert BCLK + FRM)
 *pcm_lrck_period	:16/32/64/128/256
 *msb_lsb_first		:0: msb first; 1: lsb first
 *sign_extend		:0: zero pending; 1: sign extend
 *slot_width_select	:8 bit width / 16 bit width / 32 bit width
 *frametype		:0: short frame = 1 clock width;  1: long frame = 2 clock width
 *mclk_div		:0: not output(normal setting this);
 *			:1/2/4/6/8/12/16/24/32/48/64/96/128/176/192:
 *			setting mclk as input clock to external codec,
 *			freq is pll_audio/mclk_div
 *tx_data_mode		:0: 16bit linear PCM; (use) 1: reserved;
 *			:2: 8bit u-law; (no use) 3: 8bit a-law (no use)
 *rx_data_mode		:0: 16bit linear PCM; (use) 1: reserved;
 *			:2: 8bit u-law; (no use) 3: 8bit a-law (no use)
 */

#define I2S0_PARAMS \
{.tdm_num = 0, \
.i2s_master = 4, .audio_format = 1, .signal_inversion = 1, \
.pcm_lrck_period = 128, .slot_width_select = 32, \
.msb_lsb_first	= 0, .frametype = 0, \
.tx_data_mode = 0, .rx_data_mode = 0, \
.tdm_config = 1, .mclk_div = 1,\
}

#define I2S1_PARAMS \
{.tdm_num = 1, \
.i2s_master = 4, .audio_format = 1, .signal_inversion = 1, \
.pcm_lrck_period = 128, .slot_width_select = 32, \
.msb_lsb_first	= 1, .frametype = 1, \
.tx_data_mode = 0, .rx_data_mode = 0, \
.tdm_config = 0, .mclk_div = 1,\
}

struct i2s_pinctrl i2s0_pinctrl[] = {
	I2S0_PIN_MCLK,
	I2S0_PIN_BCLK,
	I2S0_PIN_LRCK,
	I2S0_PIN_DOUT,
	I2S0_PIN_DIN,
};

struct i2s_pinctrl i2s1_pinctrl[] = {
	I2S1_PIN_MCLK,
	I2S1_PIN_BCLK,
	I2S1_PIN_LRCK,
	I2S1_PIN_DOUT,
	I2S1_PIN_DIN,
};

struct sunxi_i2s_param i2s_param[] = {
	I2S0_PARAMS,
	I2S1_PARAMS,
};


/* SUNXI_ADC_I2S_SYNC: Whether to enable ADC AEC Drive adaptation */
/* #define SUNXI_ADC_I2S_SYNC */

enum sunxi_pcm_adc_i2s_mode {
	ADC_CODEC_SYNC = 0,
	ADC_I2S_SYNC = 1,
	ADC_I2S_RUNNING = 2,
};

struct sunxi_i2s_clk {
	struct reset_control *rstclk;

	hal_clk_t pllclk;
	hal_clk_t moduleclk;
	hal_clk_t busclk;
	hal_clk_t pllclk1;
	hal_clk_t asrcclk;
};

void sunxi_i2s_rx_drq_enable(bool enable);

#endif	/* __SUN8IW19_I2S_H_ */
