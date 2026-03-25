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
#if 0

#include "ccu_mp.h"
#include "ccu_nm.h"
#endif

#include "sun300iw1_aon_ccu.h"

AW_CCU_PLL_WITH_DEFAULT_STABLE_TIME(g_pll_cpu_clk, "pll-cpu", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0000, 8, 8, 30, 31, 27, 29, 28);

static division_map_t g_pll_peri_pre_div_map[] =
{
	{ .field_value = 0, .div_value = 1 },
	{ .field_value = 1, .div_value = 2 },
	{ .field_value = 2, .div_value = 3 },
	{ .field_value = 4, .div_value = 5 },
	{ /* Sentinel */ },
};

AW_CCU_DIV_WITH_TABLE(g_pll_peri_pre_div_clk, "pll_peri_pre_div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0020, 0, 3, g_pll_peri_pre_div_map);

AW_CCU_PLL_WITH_DEFAULT_STABLE_TIME(g_pll_peri_clk, "pll_peri", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_PRE_DIV), 0x0020, 8, 8, 30, 31, 27, 29, 28);

CLK_FIXED_FACTOR(g_pll_peri_post_mult_clk, "pll_peri_post_mult", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI), 2, 1);

CLK_FIXED_FACTOR(g_pll_peri_cko_1536_clk, "pll-peri-cko-1536m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 2);

CLK_FIXED_FACTOR(g_pll_peri_cko_1024_clk, "pll-peri-cko-1024m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 3);

CLK_FIXED_FACTOR(g_pll_peri_cko_768_clk, "pll-peri-cko-768m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 4);

CLK_FIXED_FACTOR(g_pll_peri_cko_614_clk, "pll-peri-cko-614m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 5);

CLK_FIXED_FACTOR(g_pll_peri_cko_512_clk, "pll-peri-cko-512m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 6);

CLK_FIXED_FACTOR(g_pll_peri_cko_384_clk, "pll-peri-cko-384m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 8);

CLK_FIXED_FACTOR(g_pll_peri_cko_341_clk, "pll-peri-cko-341m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 9);

CLK_FIXED_FACTOR(g_pll_peri_cko_307_clk, "pll-peri-cko-307m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 10);

CLK_FIXED_FACTOR(g_pll_peri_cko_236_clk, "pll-peri-cko-236m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 13);

CLK_FIXED_FACTOR(g_pll_peri_cko_219_clk, "pll-peri-cko-219m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 14);

CLK_FIXED_FACTOR(g_pll_peri_cko_192_clk, "pll-peri-cko-192m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 16);

CLK_FIXED_FACTOR(g_pll_peri_cko_118_clk, "pll-peri-cko-118m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 26);

CLK_FIXED_FACTOR(g_pll_peri_cko_96_clk, "pll-peri-cko-96m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 32);

CLK_FIXED_FACTOR(g_pll_peri_cko_48_clk, "pll-peri-cko-48m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 64);

CLK_FIXED_FACTOR(g_pll_peri_cko_24_clk, "pll-peri-cko-24m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 128);

CLK_FIXED_FACTOR(g_pll_peri_cko_12_clk, "pll-peri-cko-12m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_POST_MULT), 1, 256);


//TODO: div link pll
AW_CCU_PLL_WITH_DEFAULT_STABLE_TIME(g_pll_video_4x_clk, "pll-video-4x", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0040, 8, 8, 30, 31, 27, 29, 28);

CLK_FIXED_FACTOR(g_pll_video_2x_clk, "pll-video-2x", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_4X), 1, 2);

CLK_FIXED_FACTOR(g_pll_video_1x_clk, "pll-video-1x", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_4X), 1, 4);

//TODO: div link pll
AW_CCU_PLL_WITH_DEFAULT_STABLE_TIME(g_pll_csi_4x_clk, "pll-csi-4x", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0048, 8, 8, 30, 31, 27, 29, 28);

CLK_FIXED_FACTOR(g_pll_csi_2x_clk, "pll-csi-2x", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_4X), 1, 2);

CLK_FIXED_FACTOR(g_pll_csi_1x_clk, "pll-csi-1x", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_4X), 1, 4);

//TODO: div link pll link div
AW_CCU_PLL_WITH_DEFAULT_STABLE_TIME(g_pll_ddr_clk, "pll-ddr", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 8, 8, 30, 31, 27, 29, 28);


const clk_number_t g_hosc_parents[] =
{
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_40M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_24M),
};
AW_CCU_MUX(g_aon_hosc_clk, "hosc", 0,
	g_hosc_parents, 0x0404, 31, 1);


const clk_number_t g_ahb_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_768M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF),
};

AW_CCU_MUX_LINK_LDIV(g_ahb_clk, "ahb", 0,
	g_ahb_parents, 0x0500, 24, 2, 0, 5);


const clk_number_t g_apb_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_384M),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF),
};

AW_CCU_MUX_LINK_LDIV(g_apb_clk, "apb", 0,
	g_apb_parents, 0x0504, 24, 2, 0, 5);

const clk_number_t g_rtc_apb_parents[] =
{
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_96M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
};

AW_CCU_MUX_LINK_LDIV(g_rtc_apb_clk, "rtc-apb", 0,
	g_rtc_apb_parents, 0x0508, 24, 2, 0, 5);


static AW_CCU_GATE(g_pwrctrl_bus_clk, "pwrctrl", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0550, 6);

static AW_CCU_GATE(g_rccal_bus_clk, "rccal", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0550, 2);


const clk_number_t g_apb_spc_clk_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_LOSC), //TODO: spec里未说明此值对应哪个时钟!
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_192M),
};

AW_CCU_MUX_LINK_LDIV(g_apb_spc_clk, "apb-spc", 0,
	g_apb_spc_clk_parents, 0x0580, 24, 2, 0, 5);

const clk_number_t g_e907_clk_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_2X),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CPU),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_1024M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_614M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_614M),
};

AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_e907_clk, "e907", 0,
	g_e907_clk_parents, 0x0584, 24, 3, 0, 5, 31);


const clk_number_t g_a27l2_clk_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_2X),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CPU),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_1024M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_768M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_768M),
};

AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_a27l2_clk, "a27l2", 0,
	g_a27l2_clk_parents, 0x0588, 24, 3, 0, 5, 31);

struct clk_hw *g_sun300iw1_aon_clk_hws[] =
{
	CCU_CLK_HW_ELEMENT(CLK_AON_PLL_CPU, g_pll_cpu_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_PLL_DDR, g_pll_ddr_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_PLL_PERI_PRE_DIV, g_pll_peri_pre_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_PLL_PERI, g_pll_peri_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_POST_MULT, g_pll_peri_post_mult_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_1536M, g_pll_peri_cko_1536_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_1024M, g_pll_peri_cko_1024_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_768M, g_pll_peri_cko_768_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_614M, g_pll_peri_cko_614_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_512M, g_pll_peri_cko_512_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_384M, g_pll_peri_cko_384_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_341M, g_pll_peri_cko_341_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_307M, g_pll_peri_cko_307_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_236M, g_pll_peri_cko_236_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_219M, g_pll_peri_cko_219_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_192M, g_pll_peri_cko_192_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_118M, g_pll_peri_cko_118_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_96M, g_pll_peri_cko_96_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_48M, g_pll_peri_cko_48_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_24M, g_pll_peri_cko_24_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_PERI_CKO_12M, g_pll_peri_cko_12_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_PLL_VIDEO_4X, g_pll_video_4x_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_VIDEO_2X, g_pll_video_2x_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_VIDEO_1X, g_pll_video_1x_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_PLL_CSI_4X, g_pll_csi_4x_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_CSI_2X, g_pll_csi_2x_clk),
	CLK_HW_ELEMENT(CLK_AON_PLL_CSI_1X, g_pll_csi_1x_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_HOSC, g_aon_hosc_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_AHB, g_ahb_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_APB0, g_apb_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_RTC_APB, g_rtc_apb_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_PWRCTRL, g_pwrctrl_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_TCCAL, g_rccal_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_APB_SPC, g_apb_spc_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_E907, g_e907_clk),
	CCU_CLK_HW_ELEMENT(CLK_AON_A27L2, g_a27l2_clk),


#if 0
	//CLK_HW_ELEMENT(CLK_PRCM_AHBS_BUS, g_apbs0_bus_clk),
	//CLK_HW_ELEMENT(CLK_PRCM_APBS0_BUS, g_apbs0_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_PRCM_APBS1_BUS, g_apbs1_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_PRCM_APBS1_DIV, g_apbs1_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_PRCM_R_SPI_FUNC, g_r_spi_func_clk),
	CCU_CLK_HW_ELEMENT(CLK_PRCM_R_SPI_BUS, g_r_spi_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_PRCM_R_UART0_BUS, g_r_uart0_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_PRCM_R_UART1_BUS, g_r_uart1_bus_clk),
#endif
};

reset_hw_t g_sun300iw1_aon_reset_hws[] =
{
	RESET_HW_ELEMENT(RST_AON_BUS_WLAN, 0x0518, 0),
};

clk_controller_t g_aon_ccu =
{
	.id = AW_AON_CCU,
	.reg_base = AON_CCU_REG_BASE,

	.clk_num = ARRAY_SIZE(g_sun300iw1_aon_clk_hws),
	.clk_hws = g_sun300iw1_aon_clk_hws,

	.reset_num = ARRAY_SIZE(g_sun300iw1_aon_reset_hws),
	.reset_hws = g_sun300iw1_aon_reset_hws,
};

