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
#ifndef	__SUN55IW3_I2S_H_
#define	__SUN55IW3_I2S_H_

#define	SUNXI_I2S_BASE (0x07112000)

#define I2S_NUM_MAX	1

/*------------------------ PIN CONFIG FOR NORMAL ---------------------------*/
#define I2S0_PIN_MCLK \
{.gpio_pin = GPIOB(29), .mux = 2, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S0_PIN_BCLK \
{.gpio_pin = GPIOB(23), .mux = 2, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S0_PIN_LRCK \
{.gpio_pin = GPIOB(24), .mux = 2, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S0_PIN_DOUT0 \
{.gpio_pin = GPIOB(25), .mux = 2, .driv_level = GPIO_DRIVING_LEVEL1}
#define I2S0_PIN_DIN0 \
{.gpio_pin = GPIOB(22), .mux = 2, .driv_level = GPIO_DRIVING_LEVEL1}

/*
 * i2s Params Setting
 *
 *i2s_master:
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
.pcm_lrck_period = 64, .slot_width_select = 32, \
.msb_lsb_first	= 0, .frametype = 0, \
.tx_data_mode = 0, .rx_data_mode = 0, \
.tdm_config = 1, .mclk_div = 2,\
.rx_sync_en = false,\
.rx_sync_ctl = false,\
}

struct sunxi_i2s_param i2s_param[] = {
	I2S0_PARAMS,
};

struct i2s_pinctrl i2s0_pinctrl[] = {
	I2S0_PIN_MCLK,
	I2S0_PIN_BCLK,
	I2S0_PIN_LRCK,
	I2S0_PIN_DOUT0,
	I2S0_PIN_DIN0,
};

struct pa_config i2s_pa_cfg[] = {
};

/*------------------------ CLK CONFIG FOR SUN20IW2 ---------------------------*/
struct sunxi_i2s_clk {
	struct reset_control *clk_rst;		/* RESET: RST_I2S */
	hal_clk_t clk_bus;			/* CCU: CLK_BUS_I2S */
};

static inline int snd_sunxi_i2s_clk_enable(struct sunxi_i2s_clk *clk, uint8_t tdm_num)
{
	int ret;

	snd_debug("\n");

	/* rst & bus */
	ret = hal_reset_control_deassert(clk->clk_rst);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("i2s clk_rst clk_deassert failed.\n");
		goto err_deassert_clk_rst;
	}
	ret = hal_clock_enable(clk->clk_bus);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("i2s clk_bus enable failed.\n");
		goto err_enable_clk_bus;
	}

	return HAL_CLK_STATUS_OK;

err_enable_clk_bus:
	hal_reset_control_assert(clk->clk_rst);
err_deassert_clk_rst:
	return HAL_CLK_STATUS_ERROR;
}

static inline void snd_sunxi_i2s_clk_disable(struct sunxi_i2s_clk *clk)
{
	snd_debug("\n");

	hal_clock_disable(clk->clk_bus);
	hal_reset_control_assert(clk->clk_rst);

	return;
}

static inline int snd_sunxi_i2s_clk_init(struct sunxi_i2s_clk *clk, uint8_t tdm_num)
{
	int ret;

	snd_debug("\n");

	/* rst & bus */
	clk->clk_rst = hal_reset_control_get(HAL_SUNXI_DSP_RESET, RST_BUS_DSP_I2S0);
	if (!clk->clk_rst) {
		snd_err("i2s clk_rst hal_reset_control_get failed\n");
		goto err_get_clk_rst;
	}
	clk->clk_bus = hal_clock_get(HAL_SUNXI_DSP, CLK_BUS_DSP_I2S0);
	if (!clk->clk_bus) {
		snd_err("i2s clk_bus hal_clock_get failed\n");
		goto err_get_clk_bus;
	}

	/* note: Enable and then set the freq to avoid clock lock errors */
	ret = snd_sunxi_i2s_clk_enable(clk, tdm_num);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("i2s snd_sunxi_i2s_clk_enable failed.\n");
		goto err_clk_enable;
	}

	return HAL_CLK_STATUS_OK;

err_clk_enable:
	hal_clock_put(clk->clk_bus);
err_get_clk_bus:
	hal_reset_control_put(clk->clk_rst);
err_get_clk_rst:
	return HAL_CLK_STATUS_ERROR;
}

static inline void snd_sunxi_i2s_clk_exit(struct sunxi_i2s_clk *clk)
{
	snd_debug("\n");

	snd_sunxi_i2s_clk_disable(clk);
	hal_clock_put(clk->clk_bus);
	hal_reset_control_put(clk->clk_rst);

	return;
}

static inline int snd_sunxi_i2s_clk_set_rate(struct sunxi_i2s_clk *clk, int stream,
						unsigned int freq_in, unsigned int freq_out)
{
	return HAL_CLK_STATUS_OK;
}

#endif	/* __SUN55IW3_I2S_H_ */
