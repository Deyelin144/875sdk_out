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
#ifndef	__SUNXI_I2S_H_
#define	__SUNXI_I2S_H_

#include <aw_common.h>
#include <hal_clk.h>
#include <hal_reset.h>
#include <hal_gpio.h>


#define I2S_NAME_LEN		(26)

/* I2S register definition */
#define	SUNXI_I2S_CTL		0x00
#define	SUNXI_I2S_FMT0		0x04
#define	SUNXI_I2S_FMT1		0x08
#define	SUNXI_I2S_INTSTA		0x0C
#define	SUNXI_I2S_RXFIFO		0x10
#define	SUNXI_I2S_FIFOCTL		0x14
#define	SUNXI_I2S_FIFOSTA		0x18
#define	SUNXI_I2S_INTCTL		0x1C
#define	SUNXI_I2S_TXFIFO		0x20
#define	SUNXI_I2S_CLKDIV		0x24
#define	SUNXI_I2S_TXCNT		0x28
#define	SUNXI_I2S_RXCNT		0x2C
#define	SUNXI_I2S_CHCFG		0x30
#define	SUNXI_I2S_TX0CHSEL		0x34
#define	SUNXI_I2S_TX1CHSEL		0x38
#define	SUNXI_I2S_TX2CHSEL		0x3C
#define	SUNXI_I2S_TX3CHSEL		0x40

#define	SUNXI_I2S_TX0CHMAP0		0x44
#define	SUNXI_I2S_TX0CHMAP1		0x48
#define	SUNXI_I2S_TX1CHMAP0		0x4C
#define	SUNXI_I2S_TX1CHMAP1		0x50
#define	SUNXI_I2S_TX2CHMAP0		0x54
#define	SUNXI_I2S_TX2CHMAP1		0x58
#define	SUNXI_I2S_TX3CHMAP0		0x5C
#define	SUNXI_I2S_TX3CHMAP1		0x60
#define	SUNXI_I2S_RXCHSEL		0x64
#define	SUNXI_I2S_RXCHMAP0		0x68
#define	SUNXI_I2S_RXCHMAP1		0x6C
#define	SUNXI_I2S_RXCHMAP2		0x70
#define	SUNXI_I2S_RXCHMAP3		0x74
#define	SUNXI_I2S_DEBUG		0x78
#define	SUNXI_I2S_REV		0x7C

#define SUNXI_I2S_ASRC_MCLKCFG       0x80
#define SUNXI_I2S_ASRC_FSOUTCFG      0x84
#define SUNXI_I2S_ASRC_FSIN_EXTCFG   0x88
#define SUNXI_I2S_ASRC_ASRCEN        0x8C
#define SUNXI_I2S_ASRC_MANCFG        0x90
#define SUNXI_I2S_ASRC_RATIOSTAT     0x94
#define SUNXI_I2S_ASRC_FIFOSTAT      0x98
#define SUNXI_I2S_ASRC_MBISTCFG      0x9C
#define SUNXI_I2S_ASRC_MBISTSTA      0xA0

/* SUNXI_I2S_CTL:0x00 */
#define RX_SYNC_EN_STA			21
#define RX_SYNC_EN			20
#define	BCLK_OUT			18
#define	LRCK_OUT			17
#define	LRCKR_CTL			16
#define	SDO3_EN				11
#define	SDO2_EN				10
#define	SDO1_EN				9
#define	SDO0_EN				8
#define	MUTE_CTL			6
#define	MODE_SEL			4
#define	LOOP_EN				3
#define	CTL_TXEN			2
#define	CTL_RXEN			1
#define	GLOBAL_EN			0

/* SUNXI_I2S_FMT0:0x04 */
#define	SDI_SYNC_SEL			31
#define	LRCK_WIDTH			30
#define	LRCKR_PERIOD			20
#define	LRCK_POLARITY			19
#define	LRCK_PERIOD			8
#define	BRCK_POLARITY			7
#define	I2S_SAMPLE_RESOLUTION	4
#define	EDGE_TRANSFER			3
#define	SLOT_WIDTH			0

/* SUNXI_I2S_FMT1:0x08 */
#define	RX_MLS				7
#define	TX_MLS				6
#define	SEXT				4
#define	RX_PDM				2
#define	TX_PDM				0

/* SUNXI_I2S_INTSTA:0x0C */
#define	TXU_INT				6
#define	TXO_INT				5
#define	TXE_INT				4
#define	RXU_INT				2
#define RXO_INT				1
#define	RXA_INT				0

/* SUNXI_I2S_FIFOCTL:0x14 */
#define	HUB_EN				31
#define	FIFO_CTL_FTX			25
#define	FIFO_CTL_FRX			24
#define	TXTL				12
#define	RXTL				4
#define	TXIM				2
#define	RXOM				0

/* SUNXI_I2S_FIFOSTA:0x18 */
#define	FIFO_TXE			28
#define	FIFO_TX_CNT			16
#define	FIFO_RXA			8
#define	FIFO_RX_CNT			0

/* SUNXI_I2S_INTCTL:0x1C */
#define	TXDRQEN				7
#define	TXUI_EN				6
#define	TXOI_EN				5
#define	TXEI_EN				4
#define	RXDRQEN				3
#define	RXUIEN				2
#define	RXOIEN				1
#define	RXAIEN				0

/* SUNXI_I2S_CLKDIV:0x24 */
#define	MCLKOUT_EN			8
#define	BCLK_DIV			4
#define	MCLK_DIV			0

/* SUNXI_I2S_CHCFG:0x30 */
#define	TX_SLOT_HIZ			9
#define	TX_STATE			8
#define	RX_SLOT_NUM			4
#define	TX_SLOT_NUM			0

/* SUNXI_I2S_TXnCHSEL:0X34+n*0x04 */
#define	TX_OFFSET			20
#define	TX_CHSEL			16
#define	TX_CHEN				0

/* SUNXI_I2S_RXCHSEL */
#define	RX_OFFSET			20
#define	RX_CHSEL			16

/* sun8iw10 CHMAP default setting */
#define	SUNXI_DEFAULT_CHMAP0		0xFEDCBA98
#define	SUNXI_DEFAULT_CHMAP1		0x76543210

/* RXCHMAP default setting */
#define	SUNXI_DEFAULT_CHMAP		0x76543210

/* Shift & Mask define */

/* SUNXI_I2S_CTL:0x00 */
#define	SUNXI_I2S_MODE_CTL_MASK		3
#define	SUNXI_I2S_MODE_CTL_PCM		0
#define	SUNXI_I2S_MODE_CTL_I2S		1
#define	SUNXI_I2S_MODE_CTL_LEFT		1
#define	SUNXI_I2S_MODE_CTL_RIGHT		2
#define	SUNXI_I2S_MODE_CTL_REVD		3
/* combine LRCK_CLK & BCLK setting */
#define	SUNXI_I2S_LRCK_OUT_MASK		3
#define	SUNXI_I2S_LRCK_OUT_DISABLE		0
#define	SUNXI_I2S_LRCK_OUT_ENABLE		3

/* SUNXI_I2S_FMT0 */
#define	SUNXI_I2S_LRCK_PERIOD_MASK		0x3FF
#define	SUNXI_I2S_SLOT_WIDTH_MASK		7
/* Left Blank */
#define	SUNXI_I2S_SR_MASK			7
#define	SUNXI_I2S_SR_16BIT			3
#define	SUNXI_I2S_SR_24BIT			5
#define	SUNXI_I2S_SR_32BIT			7

#define	SUNXI_I2S_LRCK_POLARITY_NOR		0
#define	SUNXI_I2S_LRCK_POLARITY_INV		1
#define	SUNXI_I2S_BCLK_POLARITY_NOR		0
#define	SUNXI_I2S_BCLK_POLARITY_INV		1

/* SUNXI_I2S_FMT1 */
#define	SUNXI_I2S_FMT1_DEF			0x30

/* SUNXI_I2S_FIFOCTL */
#define	SUNXI_I2S_TXIM_MASK			1
#define	SUNXI_I2S_TXIM_VALID_MSB		0
#define	SUNXI_I2S_TXIM_VALID_LSB		1
/* Left Blank */
#define	SUNXI_I2S_RXOM_MASK			3
/* Expanding 0 at LSB of RX_FIFO */
#define	SUNXI_I2S_RXOM_EXP0			0
/* Expanding sample bit at MSB of RX_FIFO */
#define	SUNXI_I2S_RXOM_EXPH			1
/* Fill RX_FIFO low word be 0 */
#define	SUNXI_I2S_RXOM_TUNL			2
/* Fill RX_FIFO high word be higher sample bit */
#define	SUNXI_I2S_RXOM_TUNH			3

/* SUNXI_I2S_CLKDIV */
#define	SUNXI_I2S_BCLK_DIV_MASK		0xF
#define	SUNXI_I2S_BCLK_DIV_1			1
#define	SUNXI_I2S_BCLK_DIV_2			2
#define	SUNXI_I2S_BCLK_DIV_3			3
#define	SUNXI_I2S_BCLK_DIV_4			4
#define	SUNXI_I2S_BCLK_DIV_5			5
#define	SUNXI_I2S_BCLK_DIV_6			6
#define	SUNXI_I2S_BCLK_DIV_7			7
#define	SUNXI_I2S_BCLK_DIV_8			8
#define	SUNXI_I2S_BCLK_DIV_9			9
#define	SUNXI_I2S_BCLK_DIV_10		10
#define	SUNXI_I2S_BCLK_DIV_11		11
#define	SUNXI_I2S_BCLK_DIV_12		12
#define	SUNXI_I2S_BCLK_DIV_13		13
#define	SUNXI_I2S_BCLK_DIV_14		14
#define	SUNXI_I2S_BCLK_DIV_15		15
/* Left Blank */
#define	SUNXI_I2S_MCLK_DIV_MASK		0xF
#define	SUNXI_I2S_MCLK_DIV_1			1
#define	SUNXI_I2S_MCLK_DIV_2			2
#define	SUNXI_I2S_MCLK_DIV_3			3
#define	SUNXI_I2S_MCLK_DIV_4			4
#define	SUNXI_I2S_MCLK_DIV_5			5
#define	SUNXI_I2S_MCLK_DIV_6			6
#define	SUNXI_I2S_MCLK_DIV_7			7
#define	SUNXI_I2S_MCLK_DIV_8			8
#define	SUNXI_I2S_MCLK_DIV_9			9
#define	SUNXI_I2S_MCLK_DIV_10		10
#define	SUNXI_I2S_MCLK_DIV_11		11
#define	SUNXI_I2S_MCLK_DIV_12		12
#define	SUNXI_I2S_MCLK_DIV_13		13
#define	SUNXI_I2S_MCLK_DIV_14		14
#define	SUNXI_I2S_MCLK_DIV_15		15

/* SUNXI_I2S_CHCFG */
#define	SUNXI_I2S_TX_SLOT_MASK		0XF
#define	SUNXI_I2S_RX_SLOT_MASK		0XF

/* SUNXI_I2S_TX0CHSEL: */
#define	SUNXI_I2S_TX_OFFSET_MASK		3
#define	SUNXI_I2S_TX_OFFSET_0		0
#define	SUNXI_I2S_TX_OFFSET_1		1
/* Left Blank */
#define	SUNXI_I2S_TX_CHEN_MASK		0xFFFF
#define	SUNXI_I2S_TX_CHSEL_MASK		0xF

/* SUNXI_I2S_RXCHSEL */
#define SUNXI_I2S_RX_OFFSET_MASK		3
#define	SUNXI_I2S_RX_CHSEL_MASK		0XF

#define I2S_RXCH_DEF_MAP(x) (x << ((x%4)<<3))
#define I2S_RXCHMAP(x) (0x1f << ((x%4)<<3))

/* SUNXI_I2S_ASRC_MCLKCFG:0x80 */
#define I2S_ASRC_MCLK_GATE           16
#define I2S_ASRC_MCLK_RATIO          0

/* SUNXI_I2S_ASRC_FSOUTCFG:0x84 */
#define I2S_ASRC_FSOUT_GATE          20
#define I2S_ASRC_FSOUT_CLKSRC        16
#define I2S_ASRC_FSOUT_CLKDIV1       4
#define I2S_ASRC_FSOUT_CLKDIV2       0

/* SUNXI_I2S_ASRC_FSIN_EXTCFG:0x88 */
#define I2S_ASRC_FSIN_EXTEN          16
#define I2S_ASRC_FSIN_EXTCYCLE       0

/* SUNXI_I2S_ASRC_ASRCEN:0x8C */
#define I2S_ASRC_ASRCEN              0

/* SUNXI_I2S_ASRC_MANCFG:0x90 */
#define I2S_ASRC_MANRATIOEN          31
#define I2S_ASRC_MAN_RATIO           0

/* SUNXI_I2S_ASRC_RATIOSTAT:0x94 */
/* SUNXI_I2S_ASRC_FIFOSTAT:0x98 */
/* SUNXI_I2S_ASRC_MBISTCFG:0x9C */
/* SUNXI_I2S_ASRC_MBISTSTA:0xA0 */

/* DRQ Type */
#ifndef DRQDST_I2S_0_TX
#define DRQDST_I2S_0_TX	3
#endif
#ifndef DRQDST_I2S_1_TX
#define DRQDST_I2S_1_TX	4
#endif
#ifndef DRQDST_I2S_2_TX
#define DRQDST_I2S_2_TX	4
#endif

#ifndef DRQSRC_I2S_0_RX
#define DRQSRC_I2S_0_RX	3
#endif
#ifndef DRQSRC_I2S_1_RX
#define DRQSRC_I2S_1_RX	4
#endif
#ifndef DRQSRC_I2S_2_RX
#define DRQSRC_I2S_2_RX	4
#endif

#define	SUNXI_I2S_RATES	(SNDRV_PCM_RATE_8000_192000 \
				| SNDRV_PCM_RATE_KNOT)

#define SUNXI_I2S_DRQDST(sunxi_i2s, x)			\
	((sunxi_i2s)->playback_dma_param.dma_drq_type_num =	\
				DRQDST_I2S_##x##_TX)
#define SUNXI_I2S_DRQSRC(sunxi_i2s, x)			\
	((sunxi_i2s)->capture_dma_param.dma_drq_type_num =	\
				DRQSRC_I2S_##x##_RX)

/*to clear FIFO*/
#define SUNXI_I2S_FTX_TIMES			1

/* SUNXI_I2S_HUB_ENABLE: Whether to use the hub mode */
#define SUNXI_I2S_HUB_ENABLE

enum sunxi_i2s_clk_parent {
	SUNXI_MODULE_CLK_PLL_AUDIO,
	SUNXI_MODULE_CLK_PLL_AUDIOX4,
};

enum SUNXI_I2S_DAI_FMT_SEL {
	SUNXI_I2S_DAI_PLL = 0,
	SUNXI_I2S_DAI_MCLK,
	SUNXI_I2S_DAI_FMT,
	SUNXI_I2S_DAI_MASTER,
	SUNXI_I2S_DAI_INVERT,
	SUNXI_I2S_DAI_SLOT_NUM,
	SUNXI_I2S_DAI_SLOT_WIDTH,
};

struct sunxi_i2s_dai_fmt {
	unsigned int pllclk_freq;
	unsigned int moduleclk_freq;
	unsigned int fmt;
	unsigned int slots;
	unsigned int slot_width;
};

struct sunxi_i2s_param {
	uint8_t i2s_index;
	uint8_t tdm_num;
	bool tx_pin[4];
	bool rx_pin[4];
	uint8_t i2s_master;
	uint8_t audio_format;
	uint8_t signal_inversion;
	uint16_t pcm_lrck_period;
	uint8_t msb_lsb_first:1;
	uint8_t sign_extend:2;
	uint8_t tx_data_mode:2;
	uint8_t rx_data_mode:2;
	uint8_t slot_width_select;
	uint8_t frametype;
	uint8_t tdm_config;
	uint16_t mclk_div;
	bool rx_sync_en;
	bool rx_sync_ctl;
	int16_t rx_sync_id;
	rx_sync_domain_t rx_sync_domain;
};

struct i2s_pinctrl {
	gpio_pin_t gpio_pin;
	uint8_t mux;
	uint8_t driv_level;
};

struct pa_config {
	gpio_pin_t pin;
	gpio_data_t level;
	uint16_t msleep;
	bool used;
};

#if defined(CONFIG_ARCH_SUN8IW18P1)
#include "platforms/i2s-sun8iw18.h"
#endif
#if defined(CONFIG_ARCH_SUN8IW19)
#include "platforms/i2s-sun8iw19.h"
#endif
#if defined(CONFIG_ARCH_SUN8IW20) || defined(CONFIG_SOC_SUN20IW1)
#include "platforms/i2s-sun8iw20.h"
#endif
#if defined(CONFIG_ARCH_SUN20IW2)
#include "platforms/i2s-sun20iw2.h"
#endif
#if defined(CONFIG_ARCH_SUN55IW3)
#include "platforms/i2s-sun55iw3.h"
#endif

#endif	/* __SUNXI_I2S_H_ */
