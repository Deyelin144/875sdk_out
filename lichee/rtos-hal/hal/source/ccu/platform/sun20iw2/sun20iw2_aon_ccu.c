/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 *the the People's Republic of China and other countries.
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


#include "aw_ccu.h"
#include "clk_core.h"
#include "ccu_reset.h"

#include "ccu_div.h"
#include "ccu_gate.h"
#include "ccu_mux.h"
#include "ccu_pll.h"

#include "sun20iw2_aon_ccu.h"

static const clk_number_t g_hosc_parents[] =
{
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_26M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_40M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_24M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_32M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_24576K),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_40M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_40M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_40M),
};
static AW_CCU_MUX(g_hosc_fake_mux_clk, "hosc-fake-mux", 0,
	g_hosc_parents, 0x084, 0, 3);

static division_map_t g_dpllx_pre_div_map[] =
{
	{ .field_value = 0, .div_value = 1 },
	{ .field_value = 1, .div_value = 1 },
	{ .field_value = 2, .div_value = 2 },
	{ .field_value = 3, .div_value = 3 },
	{ .field_value = 4, .div_value = 4 },
	{ .field_value = 5, .div_value = 5 },
	{ .field_value = 6, .div_value = 6 },
	{ .field_value = 7, .div_value = 7 },
	{ /* Sentinel */ },
};

static AW_CCU_DIV_WITH_TABLE(g_dpll1_pre_div_clk, "dpll1_pre_div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX), 0x008C, 0, 3, g_dpllx_pre_div_map);

static AW_CCU_PLL_ZERO_BASED_WITH_LOCK_STATE_REG_AND_DEFAULT_STABLE_TIME(
	g_dpll1_clk, "dpll1", 0, MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1_PRE_DIV),
	0x008C, 4, 8, 31,
	PLL_CTRL_BIT_ABSENT_BIT_OFFSET, 0xA4, 30);

static AW_CCU_DIV_WITH_TABLE(g_dpll2_pre_div_clk, "dpll2_pre_div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX), 0x0090, 0, 3, g_dpllx_pre_div_map);

static AW_CCU_PLL_ZERO_BASED_WITH_LOCK_STATE_REG_AND_DEFAULT_STABLE_TIME(
	g_dpll2_clk, "dpll2", 0, MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL2_PRE_DIV),
	0x0090, 4, 8, 31,
	PLL_CTRL_BIT_ABSENT_BIT_OFFSET, 0x0, PLL_STATE_BIT_ABSENT_BIT_OFFSET);

static AW_CCU_DIV_WITH_TABLE(g_dpll3_pre_div_clk, "dpll3_pre_div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX), 0x0094, 0, 3, g_dpllx_pre_div_map);

static AW_CCU_PLL_ZERO_BASED_WITH_LOCK_STATE_REG_AND_DEFAULT_STABLE_TIME(
	g_dpll3_clk, "dpll3", 0, MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL3_PRE_DIV),
	0x0094, 4, 8, 31,
	PLL_CTRL_BIT_ABSENT_BIT_OFFSET, 0xA8, 30);

static CLK_FIXED_FACTOR(g_rfip0_clk, "rfip0-dpll", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1), 1, 12);

static CLK_FIXED_FACTOR(g_rfip1_clk, "rfip1-dpll", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL3), 1, 12);

#if 0
static AW_CCU_LDIV(g_pll_audio_pre_div_clk, "pll_audio_pre_div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX), 0x0098, 0, 5);

static AW_CCU_PLL_WITH_DEFAULT_STABLE_TIME(g_pll_audio_clk, "pll-audio", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_AUDIO_PRE_DIV), 0x0098, 8, 7,
	PLL_CTRL_BIT_ABSENT_BIT_OFFSET,
	31, PLL_CTRL_BIT_ABSENT_BIT_OFFSET,
	29, 28);
#else
#if 0
static AW_CCU_PLL_WITH_DEFAULT_STABLE_TIME(g_pll_audio_clk, "pll-audio", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX), 0x0098, 8, 7,
	PLL_CTRL_BIT_ABSENT_BIT_OFFSET,
	31, PLL_CTRL_BIT_ABSENT_BIT_OFFSET,
	29, 28);
#endif

static AW_CCU_PLL(g_pll_audio_clk, "pll-audio", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX), 0x0098, 8, 7,
	150, PLL_CTRL_BIT_ABSENT_BIT_OFFSET,
	31, PLL_CTRL_BIT_ABSENT_BIT_OFFSET,
	29, 28);

static AW_CCU_LDIV(g_pll_audio_post_div_clk, "pll_audio_post_div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_AUDIO), 0x0098, 0, 5);
#endif

#if 0
static CLK_FIXED_FACTOR(g_pll_audio_post_mult_clk, "pll_audio_post_mult", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_AUDIO), 2, 1);
#endif

//The hw is pll-audio * 2/4, and it is equivalent to division 2
static CLK_FIXED_FACTOR(g_pll_audio_2x_clk, "pll-audio2x", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_AUDIO_POST_DIV), 1, 2);

//The hw is pll-audio * 2/8, and it is equivalent to division 2
static CLK_FIXED_FACTOR(g_pll_audio_1x_clk, "pll-audio1x", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_AUDIO_POST_DIV), 1, 4);


/* DPLL1 Output Configure Register */
static AW_CCU_GATE(g_ck1_usb_clk, "ck1-usb", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00A4, 31);

static division_map_t g_audio_div_map[] =
{
	{ .field_value = 0, .div_value = 85 },
	{ .field_value = 1, .div_value = 39 },
	{ /* Sentinel */ },
};

static AW_CCU_TDIV_LINK_GATE(g_ck1_aud_clk, "ck1-audio", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1), 0xA4, 24, 1, g_audio_div_map, 27);

static division_map_t g_dev_div_map[] =
{
	{ .field_value = 0, .div_value = 7 },
	{ .field_value = 1, .div_value = 6 },
	{ .field_value = 2, .div_value = 5 },
	{ .field_value = 3, .div_value = 4 },
	{ /* Sentinel */ },
};

static AW_CCU_TDIV_LINK_GATE(g_ck1_dev_clk, "ck1-device", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1), 0xA4, 20, 2, g_dev_div_map, 23);

static division_map_t g_lspsrm_m33_div_map[] =
{
	{ .field_value = 0, .div_value = 8 },
	{ .field_value = 1, .div_value = 7 },
	{ .field_value = 2, .div_value = 6 },
	{ .field_value = 3, .div_value = 5 },
	{ .field_value = 4, .div_value = 4 },
	{ .field_value = 5, .div_value = 4 },
	{ .field_value = 6, .div_value = 4 },
	{ .field_value = 7, .div_value = 4 },
	{ /* Sentinel */ },
};

static AW_CCU_TDIV_LINK_GATE(g_ck1_lspsram_clk, "ck1-lspsram", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1), 0xA4, 16, 3, g_lspsrm_m33_div_map, 19);

static CLK_FIXED_FACTOR(g_ck1_hspsram_fake_pre_mult_clk, "ck1-hspsram-fake-pre-mult", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1), 2, 1);

static division_map_t g_hspsram_div_map[] =
{
	{ .field_value = 0, .div_value = 6 },
	{ .field_value = 1, .div_value = 5 },
	{ .field_value = 2, .div_value = 4 },
	{ .field_value = 3, .div_value = 2 },
	{ /* Sentinel */ },
};

static AW_CCU_TDIV_LINK_GATE(g_ck1_hspsram_clk, "ck1-hspsram", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_HSPSRAM_FAKE_PRE_MULT), 0xA4, 12, 2, g_hspsram_div_map, 15);


static division_map_t g_hifi5_div_map[] =
{
	{ .field_value = 0, .div_value = 7 },
	{ .field_value = 1, .div_value = 6 },
	{ .field_value = 2, .div_value = 5 },
	{ .field_value = 3, .div_value = 4 },
	{ .field_value = 4, .div_value = 3 },
	{ .field_value = 5, .div_value = 3 },
	{ .field_value = 6, .div_value = 3 },
	{ .field_value = 7, .div_value = 3 },
	{ /* Sentinel */ },
};

static AW_CCU_TDIV_LINK_GATE(g_ck1_hifi5_clk, "ck1-hifi5", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1), 0xA4, 8, 3, g_hifi5_div_map, 11);

static CLK_FIXED_FACTOR(g_ck1_c906_fake_pre_mult_clk, "ck1-c906-fake-pre-mult", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1), 2, 1);

static division_map_t g_c906_div_map[] =
{
	{ .field_value = 0, .div_value = 14 },
	{ .field_value = 1, .div_value = 8 },
	{ .field_value = 2, .div_value = 6 },
	{ .field_value = 3, .div_value = 5 },
	{ .field_value = 4, .div_value = 4 },
	{ .field_value = 5, .div_value = 4 },
	{ .field_value = 6, .div_value = 4 },
	{ .field_value = 7, .div_value = 4 },
	{ /* Sentinel */ },
};

static AW_CCU_TDIV_LINK_GATE(g_ck1_c906_clk, "ck1-c906", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_C906_FAKE_PRE_MULT), 0xA4, 4, 3, g_c906_div_map, 7);

static AW_CCU_TDIV_LINK_GATE(g_ck1_m33_clk, "ck1-m33", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL1), 0xA4, 0, 3, g_lspsrm_m33_div_map, 3);



/* DPLL3 Output Configure Register */
static AW_CCU_TDIV_LINK_GATE(g_ck3_dev_clk, "ck3-device", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL3), 0xA8, 20, 2, g_dev_div_map, 23);

static AW_CCU_TDIV_LINK_GATE(g_ck3_lspsram_clk, "ck3-lspsram", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL3), 0xA4, 16, 3, g_lspsrm_m33_div_map, 19);

static CLK_FIXED_FACTOR(g_ck3_hspsram_fake_pre_mult_clk, "ck3-hspsram-fake-pre-mult", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL3), 2, 1);

static AW_CCU_TDIV_LINK_GATE(g_ck3_hspsram_clk, "ck3-hspsram", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_HSPSRAM_FAKE_PRE_MULT), 0xA8, 12, 2, g_hspsram_div_map, 15);

static AW_CCU_TDIV_LINK_GATE(g_ck3_hifi5_clk, "ck3-hifi5", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL3), 0xA8, 8, 3, g_hifi5_div_map, 11);

static CLK_FIXED_FACTOR(g_ck3_c906_fake_pre_mult_clk, "ck3-c906-fake-pre-mult", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL3), 2, 1);

static AW_CCU_TDIV_LINK_GATE(g_ck3_c906_clk, "ck3-c906", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_C906_FAKE_PRE_MULT), 0xA8, 4, 3, g_c906_div_map, 7);

static AW_CCU_TDIV_LINK_GATE(g_ck3_m33_clk, "ck3-m33", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DPLL3), 0xA8, 0, 3, g_lspsrm_m33_div_map, 3);


/* AUDPLL and DPLL3 LDO Control Register */
static AW_CCU_GATE(g_ldo1_bypass_clk, "ldo1-bypass", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_USELESS), 0x00AC, 1);

static AW_CCU_GATE(g_ldo2_en_clk, "ldo2-en", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_USELESS), 0x00AC, 16);

static AW_CCU_GATE(g_ldo1_en_clk, "ldo1-en", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_USELESS), 0x00AC, 0);

#if 0
/* Wlan BT PFTP Contrcl Rigister */
static AW_CCU_GATE(g_wlan_bt_debug_sel0_clk, "wlan-bt-debug0-sel", "hosc", 0x0c4, BIT(4), 0);
static AW_CCU_GATE(g_wlan_bt_debug_sel1_clk, "wlan-bt-debug1-sel", "hosc", 0x0c4, BIT(5), 0);
static AW_CCU_GATE(g_wlan_bt_debug_sel2_clk, "wlan-bt-debug2-sel", "hosc", 0x0c4, BIT(6), 0);
static AW_CCU_GATE(g_wlan_bt_debug_sel3_clk, "wlan-bt-debug3-sel", "hosc", 0x0c4, BIT(7), 0);
static AW_CCU_GATE(g_wlan_bt_debug_sel4_clk, "wlan-bt-debug4-sel", "hosc", 0x0c4, BIT(8), 0);
static AW_CCU_GATE(g_wlan_bt_debug_sel5_clk, "wlan-bt-debug5-sel", "hosc", 0x0c4, BIT(9), 0);
static AW_CCU_GATE(g_wlan_bt_debug_sel6_clk, "wlan-bt-debug6-sel", "hosc", 0x0c4, BIT(10), 0);
static AW_CCU_GATE(g_wlan_bt_debug_sel7_clk, "wlan-bt-debug7-sel", "hosc", 0x0c4, BIT(11), 0);
#endif

static const clk_number_t g_wlan_bt_parents[] =
{
	MAKE_CLKn(AW_SRC_CCU, CLK_AON_RFIP0_DPLL),
	MAKE_CLKn(AW_SRC_CCU, CLK_AON_RFIP1_DPLL),
};
static AW_CCU_MUX(g_wlan_mux_clk, "wlan-mux", 0,
	g_wlan_bt_parents, 0x00C4, 2, 1);
static AW_CCU_MUX(g_bt_mux_clk, "bt-mux", 0,
	g_wlan_bt_parents, 0x00C4, 1, 1);

static AW_CCU_GATE(g_rfip2_en, "rfip2-en", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_USELESS), 0x00C4, 0);

/* Module Clock Enable Register */
static AW_CCU_GATE(g_ble_32m_gate_clk, "ble-32m-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, 17);

static AW_CCU_GATE(g_ble_48m_gate_clk, "ble-48m-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (16));

static AW_CCU_GATE(g_bus_mad_ahb_gate_clk, "bus-mad-ahb-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x00CC, (15));

static AW_CCU_GATE(g_bus_gpio_gate_clk, "bus-gpio-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (11));

static AW_CCU_GATE(g_bus_codec_dac_gate_clk, "bus-codec-dac-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (10));

static AW_CCU_GATE(g_bus_rccal_gate_clk, "bus-rccal-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (8));

static AW_CCU_GATE(g_bus_codec_adc_gate_clk, "bus-codec-adc-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (5));

static AW_CCU_GATE(g_bus_mad_apb_gate_clk, "bus-mad-apb-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x00CC, (4));

static AW_CCU_GATE(g_bus_dmic_gate_clk, "bus-dmic-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (3));

static AW_CCU_GATE(g_bus_gpadc_gate_clk, "bus-gpadc-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (2));

static AW_CCU_GATE(g_bus_lpuart1_wkup_gate_clk, "bus-lpuart1-wkup-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (1));

static AW_CCU_GATE(g_bus_lpuart0_wkup_gate_clk, "bus-lpuart0-wkup-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x00CC, (0));

/* LPUART0 Control Register */
static const clk_number_t g_lpuartx_parents[] =
{
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_SYS_32K_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
};

static AW_CCU_MUX_LINK_GATE(g_lpuart0_clk, "lpuart0", 0,
	g_lpuartx_parents, 0x00D0, 0, 1, 31);

/* LPUART1 Control Register */
static AW_CCU_MUX_LINK_GATE(g_lpuart1_clk, "lpuart1", 0,
	g_lpuartx_parents, 0x00D4, 0, 1, 31);

/* GPADC Clock Control Register */
static const clk_number_t g_gpadc_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_SYS_32K_MUX),
};
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_gpadc_clk, "gpadc", 0,
	g_gpadc_parents, 0x00D8, 24, 2, 16, 2, 0, 4, 31);

/* Audio Clock Control Register */
static AW_CCU_GATE(g_dmic_gate_clk, "dmic-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CODEC_ADC_MUX), 0x0DC, (28));

static const clk_number_t g_audio_common_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AUDPLL_HOSC_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_AUDIO_DIV),
};

static AW_CCU_MUX_LINK_GATE(g_spdif_tx_clk, "spdif-tx", 0,
	g_audio_common_parents, 0x0DC, 21, 1, 27);

static const clk_number_t g_audpll_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_AUDIO1X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_AUDIO2X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN),
};

static AW_CCU_MUX(g_audpll_hosc_mux_clk, "audpll-hosc-mux", 0,
	g_audpll_parents, 0x0DC, 15, 2);

static AW_CCU_MUX_LINK_GATE(g_i2s_clk, "i2s", 0,
	g_audio_common_parents, 0x0DC, 20, 1, (26));

static AW_CCU_MUX_LINK_GATE(g_codec_dac_clk, "codec-dac", 0,
	g_audio_common_parents, 0x0dc, 19, 1, (25));

static const clk_number_t g_codec_adc_mux_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CODEC_ADC_PRE),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_RC_HF_DIV),
};
static AW_CCU_MUX(g_codec_adc_mux_clk, "codec-adc-mux", 0,
	g_codec_adc_mux_parents, 0x0DC, 18, 1);

static AW_CCU_GATE(g_codec_adc_gate_clk, "codec-adc-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CODEC_ADC_MUX), 0x0DC, (24));

static const clk_number_t g_codec_adc_pre_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AUDPLL_HOSC_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_AUDIO_DIV),
};
static AW_CCU_MUX_LINK_LDIV(g_codec_adc_pre_clk, "codec-adc-pre", 0,
	g_codec_adc_pre_parents, 0x0DC, 17, 1, 8, 4);

static AW_CCU_LDIV(g_ck1_audio_div_clk, "ck1-audio-div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_AUDIO), 0x0DC, 4, 4);

//AUD_RCO_DIV_N, rco_hf_div
static AW_CCU_LDIV(g_rc_hf_div_clk, "rc-hf-div", 0,
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_RC_HF_GATE), 0x0DC, 0, 3);

/* System Clock Control Register */
static const clk_number_t g_ck_hspsram_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_HSPSRAM),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_HSPSRAM),
};
static AW_CCU_MUX(g_ck_hspsram_mux_clk, "ck-hspsram-mux", 0,
	g_ck_hspsram_parents, 0xE0, 21, 1);

static const clk_number_t g_ck_lspsram_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_LSPSRAM),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_LSPSRAM),
};
static AW_CCU_MUX(g_ck_lspsram_mux_clk, "ck-lspsram-mux", 0,
	g_ck_lspsram_parents, 0xE0, 20, 1);

static const clk_number_t g_m33_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_M33),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_M33),
};
static AW_CCU_MUX(g_m33_mux_clk, "m33-mux", 0,
	g_m33_parents, 0xE0, 19, 1);

static const clk_number_t g_hifi5_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_HIFI5),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_HIFI5),
};
static AW_CCU_MUX(g_hifi5_mux_clk, "hifi5-mux", 0,
	g_hifi5_parents, 0xE0, 18, 1);

static const clk_number_t g_ck_c906_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_C906),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_C906),
};
static AW_CCU_MUX(g_ck_c906_mux_clk, "ck-c906-mux", 0,
	g_ck_c906_parents, 0xE0, 17, 1);

static const clk_number_t g_device_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_DEV),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_DEV),
};

#if 0
static AW_CCU_MUX_LINK_LDIV(g_device_clk, "device", CLK_IS_CRITICAL,
	g_device_parents, 0xE0, 16, 1, 0, 4);
#else
static AW_CCU_MUX(g_ck_dev_mux_clk, "ck-dev-mux", 0,
	g_device_parents, 0xE0, 16, 1);

static AW_CCU_LDIV(g_device_div_clk, "device-div", CLK_IS_CRITICAL,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_DEV_MUX), 0xE0, 0, 4);
#endif

//sysclk
static AW_CCU_LDIV(g_ck_m33_div_clk, "ck-m33-div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_M33_MUX), 0xE0, 8, 4);

static const clk_number_t g_ahb_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_SYS_32K_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_M33_DIV),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_RC_HF_DIV),
};
static AW_CCU_MUX(g_ahb_mux_clk, "ahb-mux", 0,
	g_ahb_parents, 0xE0, 12, 2);

static AW_CCU_PDIV(g_hosc_to_apb_div_clk, "hosc-to-apb-div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX), 0xE0, 4, 2);

static AW_CCU_PDIV(g_sys_32k_to_apb_div_clk, "sys-32k-to-apb-div", 0,
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_SYS_32K_MUX), 0xE0, 4, 2);

static AW_CCU_PDIV(g_ahb_to_apb_div_clk, "ahb-to-apb-div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0xE0, 4, 2);

static const clk_number_t g_apb_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_TO_APB_DIV),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_SYS_32K_TO_APB_DIV),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_TO_APB_DIV),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_RC_HF_DIV),
};
static AW_CCU_MUX(g_apb_mux_clk, "apb-mux", CLK_IS_CRITICAL,
	g_apb_parents, 0xE0, 6, 2);

/* MAD_lpsd_clk control register */
static AW_CCU_PDIV(g_mad_lpsr_pre_div_clk, "mad-lpsd-pre-div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CODEC_ADC_MUX), 0xE4, 16, 2);

static AW_CCU_LDIV_LINK_GATE(g_mad_lpsd_clk, "mad-lpsd", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_MAD_LPSD_PRE_DIV), 0x0E4, 0, 4, 31);

/* SPDIF_RX_clock control register */
static const clk_number_t g_spdif_rx_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK1_M33),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_HIFI5_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_C906_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_M33),
};
static AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_spdif_rx_clk, "spdif-rx", 0,
	g_spdif_rx_parents, 0x0E8, 24, 2, 0, 4, 16, 2, 31);

/* I2S_ASRC_clk control register */
static const clk_number_t g_i2s_asrc_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_M33_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_HIFI5_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_C906_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK3_HIFI5),
};
static AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_i2s_asrc_clk, "i2s-asrc", 0,
	g_i2s_asrc_parents, 0x0EC, 24, 2, 0, 4, 16, 2, 31);


struct clk_hw *g_sun20iw2_aon_clk_hws[] =
{
	CCU_CLK_HW_ELEMENT(CLK_AON_HOSC_FAKE_MUX, g_hosc_fake_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_DPLL1_PRE_DIV, g_dpll1_pre_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_DPLL1, g_dpll1_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_DPLL2_PRE_DIV, g_dpll2_pre_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_DPLL2, g_dpll2_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_DPLL3_PRE_DIV, g_dpll3_pre_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_DPLL3, g_dpll3_clk),
	CLK_HW_ELEMENT(CLK_AON_RFIP0_DPLL, g_rfip0_clk),
	CLK_HW_ELEMENT(CLK_AON_RFIP1_DPLL, g_rfip1_clk),
	//CCU_CLK_HW_ELEMENT(CLK_AON_PLL_AUDIO_PRE_DIV, g_pll_audio_pre_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_PLL_AUDIO, g_pll_audio_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_PLL_AUDIO_POST_DIV, g_pll_audio_post_div_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_AUDIO1X, g_pll_audio_1x_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_AUDIO2X, g_pll_audio_2x_clk),

	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_USB, g_ck1_usb_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_AUDIO, g_ck1_aud_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_DEV, g_ck1_dev_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_LSPSRAM, g_ck1_lspsram_clk),
	CLK_HW_ELEMENT(CLK_AON_CK1_HSPSRAM_FAKE_PRE_MULT, g_ck1_hspsram_fake_pre_mult_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_HSPSRAM, g_ck1_hspsram_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_HIFI5, g_ck1_hifi5_clk),
	CLK_HW_ELEMENT(CLK_AON_CK1_C906_FAKE_PRE_MULT, g_ck1_c906_fake_pre_mult_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_C906, g_ck1_c906_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_M33, g_ck1_m33_clk),

	CCU_CLK_HW_ELEMENT(CLK_AON_CK3_DEV, g_ck3_dev_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK3_LSPSRAM, g_ck3_lspsram_clk),
	CLK_HW_ELEMENT(CLK_AON_CK3_HSPSRAM_FAKE_PRE_MULT, g_ck3_hspsram_fake_pre_mult_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK3_HSPSRAM, g_ck3_hspsram_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK3_HIFI5, g_ck3_hifi5_clk),
	CLK_HW_ELEMENT(CLK_AON_CK3_C906_FAKE_PRE_MULT, g_ck3_c906_fake_pre_mult_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK3_C906, g_ck3_c906_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK3_M33, g_ck3_m33_clk),

	CCU_CLK_HW_ELEMENT(CLK_AON_LDO_BYPASS, g_ldo1_bypass_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_LDO2_EN, g_ldo2_en_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_LDO1_EN, g_ldo1_en_clk),
#if 0
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_BT_DEBUG_SEL0, g_xxx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_BT_DEBUG_SEL1, g_xxx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_BT_DEBUG_SEL2, g_xxx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_BT_DEBUG_SEL3, g_xxx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_BT_DEBUG_SEL4, g_xxx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_BT_DEBUG_SEL5, g_xxx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_BT_DEBUG_SEL6, g_xxx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_BT_DEBUG_SEL7, g_xxx_clk),
#endif
	CCU_CLK_HW_ELEMENT(CLK_AON_WLAN_MUX, g_wlan_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BT_MUX, g_bt_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_RFIP2_EN, g_rfip2_en),

	CCU_CLK_HW_ELEMENT(CLK_AON_BLE_32M_GATE, g_ble_32m_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BLE_48M_GATE, g_ble_48m_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_MAD_AHB_GATE, g_bus_mad_ahb_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_GPIO_GATE, g_bus_gpio_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_CODEC_DAC_GATE, g_bus_codec_dac_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_RCCAL_GATE, g_bus_rccal_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_CODEC_ADC_GATE, g_bus_codec_adc_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_MAD_APB_GATE, g_bus_mad_apb_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_DMIC_GATE, g_bus_dmic_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_GPADC_GATE, g_bus_gpadc_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_LPUART1_WKUP_GATE, g_bus_lpuart1_wkup_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_BUS_LPUART0_WKUP_GATE, g_bus_lpuart0_wkup_gate_clk),

	CCU_CLK_HW_ELEMENT(CLK_AON_LPUART0, g_lpuart0_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_LPUART1, g_lpuart1_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_GPADC, g_gpadc_clk),

	CCU_CLK_HW_ELEMENT(CLK_AON_DMIC_GATE, g_dmic_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_SPDIF_TX, g_spdif_tx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_I2S, g_i2s_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CODEC_DAC, g_codec_dac_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CODEC_ADC_MUX, g_codec_adc_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CODEC_ADC_GATE, g_codec_adc_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_AUDPLL_HOSC_MUX, g_audpll_hosc_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CODEC_ADC_PRE, g_codec_adc_pre_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK1_AUDIO_DIV, g_ck1_audio_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_RC_HF_DIV, g_rc_hf_div_clk),

	CCU_CLK_HW_ELEMENT(CLK_AON_CK_HSPSRAM_MUX, g_ck_hspsram_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK_LSPSRAM_MUX, g_ck_lspsram_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK_M33_MUX, g_m33_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK_HIFI5_MUX, g_hifi5_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK_C906_MUX, g_ck_c906_mux_clk),

	CCU_CLK_HW_ELEMENT(CLK_AON_CK_DEV_MUX, g_ck_dev_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_DEVICE_DIV, g_device_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_CK_M33_DIV, g_ck_m33_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_AHB_MUX, g_ahb_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_HOSC_TO_APB_DIV, g_hosc_to_apb_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_SYS_32K_TO_APB_DIV, g_sys_32k_to_apb_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_AHB_TO_APB_DIV, g_ahb_to_apb_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_APB_MUX, g_apb_mux_clk),

	CCU_CLK_HW_ELEMENT(CLK_AON_MAD_LPSD_PRE_DIV, g_mad_lpsr_pre_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_MAD_LPSD, g_mad_lpsd_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_SPDIF_RX, g_spdif_rx_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_I2S_ASRC, g_i2s_asrc_clk),
};

reset_hw_t g_sun20iw2_aon_reset_hws[] =
{
	RESET_HW_ELEMENT(RST_AON_BLE_RTC, 0x00C8, 16),
	RESET_HW_ELEMENT(RST_AON_MADCFG, 0x00C8, 15),
	RESET_HW_ELEMENT(RST_AON_WLAN_CONN, 0x00C8, 13),
	RESET_HW_ELEMENT(RST_AON_WLAN, 0x00C8, 12),
	RESET_HW_ELEMENT(RST_AON_CODEC_DAC, 0x00C8, 10),
	RESET_HW_ELEMENT(RST_AON_RFAS, 0x00C8, 9),
	RESET_HW_ELEMENT(RST_AON_RCCAL, 0x00C8, 8),
	RESET_HW_ELEMENT(RST_AON_LPSD, 0x00C8, 7),
	RESET_HW_ELEMENT(RST_AON_AON_TIMER, 0x00C8, 6),
	RESET_HW_ELEMENT(RST_AON_CODEC_ADC, 0x00C8, 5),
	RESET_HW_ELEMENT(RST_AON_MAD, 0x00C8, 4),
	RESET_HW_ELEMENT(RST_AON_DMIC, 0x00C8, 3),
	RESET_HW_ELEMENT(RST_AON_GPADC, 0x00C8, 2),
	RESET_HW_ELEMENT(RST_AON_LPUART1, 0x00C8, 1),
	RESET_HW_ELEMENT(RST_AON_LPUART0, 0x00C8, 0),
	//RESET_HW_ELEMENT(RST_AON_BLE_32M, 0x00CC, 17),
	//RESET_HW_ELEMENT(RST_AON_BLE_48M, 0x00CC, 16),
};

clk_controller_t g_aon_ccu =
{
	.id = AW_AON_CCU,
	.reg_base = AON_CCU_REG_BASE,

	.clk_num = ARRAY_SIZE(g_sun20iw2_aon_clk_hws),
	.clk_hws = g_sun20iw2_aon_clk_hws,

	.reset_num = ARRAY_SIZE(g_sun20iw2_aon_reset_hws),
	.reset_hws = g_sun20iw2_aon_reset_hws,
};


#ifndef CONFIG_ARCH_DSP /* freq trim do not use in dsp */
#define CCMU_AON_DXCO_CTRL              (AON_CCU_REG_BASE + 0x88)
#define CCMU_AON_DXCO_TRIM_START        (20)
#define CCMU_AON_DXCO_TRIM_LEN          (7) //the analog only use 7 bits, max trim is 127
#define CCMU_AON_DXCO_TRIM_MASK         (0x7f << CCMU_AON_DXCO_TRIM_START)

#include <io.h>
hal_clk_status_t hal_clk_ccu_aon_set_freq_trim(uint32_t value)
{
	sr32(CCMU_AON_DXCO_CTRL, CCMU_AON_DXCO_TRIM_START, CCMU_AON_DXCO_TRIM_LEN, value);
	return 0;
}

uint32_t hal_clk_ccu_aon_get_freq_trim(void)
{
	uint32_t reg_data = hal_readl(CCMU_AON_DXCO_CTRL);
	return (reg_data & CCMU_AON_DXCO_TRIM_MASK) >> CCMU_AON_DXCO_TRIM_START;
}

#if DCXO_ICTRL_USE_BIT12
#define CCMU_AON_DXCO_ICTRL_START        (12)
#define CCMU_AON_DXCO_ICTRL_LEN          (5)
#define CCMU_AON_DXCO_ICTRL_MASK         (0x1f)
#else
#define CCMU_AON_DXCO_ICTRL_START        (13)
#define CCMU_AON_DXCO_ICTRL_LEN          (4)
#define CCMU_AON_DXCO_ICTRL_MASK         (0xf)
#endif

hal_clk_status_t hal_clk_ccu_aon_set_dcxo_ictrl(uint32_t value)
{
	sr32(CCMU_AON_DXCO_CTRL, CCMU_AON_DXCO_ICTRL_START,
	     CCMU_AON_DXCO_ICTRL_LEN, value);
	return 0;
}

uint32_t hal_clk_ccu_aon_get_dcxo_ictrl(void)
{
	uint32_t reg_data = hal_readl(CCMU_AON_DXCO_CTRL);
	return (reg_data &
	        (CCMU_AON_DXCO_ICTRL_MASK << CCMU_AON_DXCO_ICTRL_START)) >>
	        CCMU_AON_DXCO_ICTRL_START;
}
#endif


#define HOSC_CLOCK_24M      (24U * 1000U * 1000U)
#define HOSC_CLOCK_26M      (26U * 1000U * 1000U)
#define HOSC_CLOCK_32M      (32U * 1000U * 1000U)
#define HOSC_CLOCK_40M      (40U * 1000U * 1000U)
#define HOSC_CLOCK_24_576M      (24576U * 1000U)

uint32_t HAL_GetHFClock(void)
{
    uint32_t PRCM_HOSCClock[] = {
                    HOSC_CLOCK_26M,
                    HOSC_CLOCK_40M,
                    HOSC_CLOCK_24M,
                    HOSC_CLOCK_32M,
                    HOSC_CLOCK_24_576M};
    uint32_t val = 0;

    val = *(volatile uint32_t *)(AON_CCU_REG_BASE + 0x84);

    return PRCM_HOSCClock[val];
}

