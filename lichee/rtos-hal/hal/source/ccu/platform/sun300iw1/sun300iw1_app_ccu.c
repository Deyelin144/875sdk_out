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


#include "ccu_gate.h"
#include "ccu_mux.h"
#include "ccu_div.h"
#include "ccu_multiplier.h"
#include "ccu_pll.h"

//#include "ccu_mp.h"
//#include "ccu_nk.h"
//#include "ccu_nkm.h"
//#include "ccu_nkmp.h"
//#include "ccu_nm.h"

#include "sun300iw1_app_ccu.h"



const clk_number_t g_dram_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_DDR),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_1024M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_768M),
};
AW_CCU_MUX_WITH_UPD_LINK_LDIV_PDIV_LINK_GATE(g_dram_clk, "dram", 0,
	g_dram_parents, 0x0004, 24, 3, 27, 0, 5, 16, 2, 31);

//AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_dram_clk, "dram", 0,
//	g_dram_parents, 0x0004, 24, 3, 0, 5, 16, 2, 31);

const clk_number_t g_e907_a27l2_mt_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_LOSC),
};
AW_CCU_MUX_LINK_GATE(g_e907_ts_clk, "e907-ts", 0,
	g_e907_a27l2_mt_parents, 0x000c, 24, 1, 31);

AW_CCU_MUX_LINK_GATE(g_a27l2_mt_clk, "a27l2-mt", 0,
	g_e907_a27l2_mt_parents, 0x0010, 24, 1, 31);

const clk_number_t g_smhcx_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_192M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_219M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_219M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_DDR),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_2X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_DDR),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_2X),
};
AW_CCU_MUX_LINK_LDIV_LDIV_LINK_GATE(g_smhc0_clk, "smhc0", 0,
	g_smhcx_parents, 0x014, 24, 3, 0, 5, 16, 5, 31);

const clk_number_t g_ss_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_118M),
};
AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_ss_clk, "ss", 0,
	g_ss_parents, 0x018, 24, 1, 0, 5, 31);

const clk_number_t g_spi_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_307M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_236M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_236M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_48M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_2X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_48M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_2X),
};
AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_spi_clk, "spi", 0,
	g_spi_parents, 0x01c, 24, 3, 0, 4, 16, 2, 31);

const clk_number_t g_spif_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_512M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_384M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_307M),
};
AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_spif_clk, "spif", 0,
	g_spif_parents, 0x020, 24, 2, 0, 4, 16, 2, 31);

const clk_number_t g_mcsi_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_236M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_307M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_384M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_384M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_4X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_4X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_4X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_4X),
};
AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_mcsi_clk, "mcsi", 0,
	g_mcsi_parents, 0x024, 24, 3, 0, 5, 31);

const clk_number_t g_csi_master_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_4X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_4X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_4X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_1024M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_24M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_1024M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_24M),
};
AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_csi_master0_clk, "csi-master0", 0,
	g_csi_master_parents, 0x028, 24, 3, 0, 5, 16, 2, 31);

AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_csi_master1_clk, "csi-master1", 0,
	g_csi_master_parents, 0x02C, 24, 3, 0, 5, 16, 2, 31);

AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_spi2_clk, "spi2", 0,
	g_spi_parents, 0x0030, 24, 3, 0, 4, 16, 2, 31);

const clk_number_t g_tconlcd_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_4X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_512M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_4X),
};
AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_tconlcd_clk, "tconlcd", 0,
	g_tconlcd_parents, 0x0034, 24, 2, 0, 4, 16, 2, 31);

const clk_number_t g_de_g2d_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_307M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_1X),
};

AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_de_clk, "de", 0,
	g_de_g2d_parents, 0x038, 24, 1, 0, 5, 31);

AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_g2d_clk, "g2d", 0,
	g_de_g2d_parents, 0x03c, 24, 1, 0, 5, 31);

const clk_number_t g_gpadc_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_24M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_LOSC),
};

AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_gpadc0_24m_clk, "gpadc0_24m", 0,
	g_gpadc_parents, 0x040, 24, 2, 0, 5, 31);

const clk_number_t g_ve_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_219M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_341M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_614M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_768M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_1024M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_2X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_4X),
};

AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_ve_clk, "ve", 0,
	g_ve_parents, 0x044, 24, 3, 0, 3, 31);

const clk_number_t g_audio_dac_adc_i2s_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_APP_PLL_AUDIO_1X),
	MAKE_CLKn(AW_AON_CCU, CLK_APP_PLL_AUDIO_4X),
};

AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_audio_dac_clk, "audio-dac", 0,
	g_audio_dac_adc_i2s_parents, 0x048, 24, 1, 0, 4, 31);


AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_audio_adc_clk, "audio-adc", 0,
	g_audio_dac_adc_i2s_parents, 0x04c, 24, 1, 0, 4, 31);

AW_CCU_MUX_LINK_LDIV_LINK_GATE(g_i2s0_clk, "i2s0", 0,
	g_audio_dac_adc_i2s_parents, 0x054, 24, 1, 0, 4, 31);

AW_CCU_MUX_LINK_LDIV_LDIV_LINK_GATE(g_smhc1_clk, "smhc1", 0,
	g_smhcx_parents, 0x05c, 24, 3, 0, 5, 16, 5, 31);

const clk_number_t g_pll_audio_4x_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_1536M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CPU),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_2X),
};

AW_CCU_MUX_LINK_LDIV(g_pll_audio_4x_clk, "pll-audio-4x", 0,
	g_pll_audio_4x_parents, 0x0060, 26, 2, 5, 5);

const clk_number_t g_pll_audio_1x_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_PERI_CKO_614M),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CPU),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_VIDEO_2X),
};
AW_CCU_MUX_LINK_LDIV(g_pll_audio_1x_clk, "pll-audio-1x", 0,
	g_pll_audio_1x_parents, 0x0060, 24, 2, 0, 5);

AW_CCU_MUX_LINK_LDIV_PDIV_LINK_GATE(g_spi1_clk, "spi1", 0,
	g_spi_parents, 0x064, 24, 3, 0, 4, 16, 2, 31);

static AW_CCU_GATE(g_gmac_fanout_clk, "a27l2-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x007c, 7);

static AW_CCU_GATE(g_a27_msgbox_clk, "a27-msgbox", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x007c, 6);

static AW_CCU_GATE(g_usb_24m_clk, "usb-24m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x007c, 3);

static AW_CCU_GATE(g_usb_12m_clk, "usb-12m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x007c, 2);

static AW_CCU_GATE(g_usb_48m_clk, "usb-48m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x007c, 1);

static AW_CCU_GATE(g_avs_clk, "avs", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x007c, 0);

const clk_number_t g_gmac_25m_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CSI_2X),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_PLL_CPU),
};

AW_CCU_MUX_LINK_LDIV_LDIV_LINK_GATE(g_gmac_25m_clk, "gmac-25m", 0,
	g_gmac_25m_parents, 0x074, 24, 2, 0, 5, 16, 5, 31);

static AW_CCU_GATE(g_dpss_top_bus_clk, "dpss-top", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 31);

static AW_CCU_GATE(g_gpio_bus_clk, "gpio", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 30);

static AW_CCU_GATE(g_mcsi_ahb_clk, "mcsi-ahb", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 28);

static AW_CCU_GATE(g_mcsi_mbus_clk, "mcsi-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 27);

static AW_CCU_GATE(g_vid_out_clk, "vid-out-ahb", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 26);

static AW_CCU_GATE(g_vid_out_mbus_clk, "vid-out-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 25);

static AW_CCU_GATE(g_gmac_hbus_clk, "gmac-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 24);

static AW_CCU_GATE(g_usbohci_clk, "usbohci", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 22);

static AW_CCU_GATE(g_usbehci_clk, "usbehci", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 21);

static AW_CCU_GATE(g_usbotg_clk, "usbotg", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 20);

static AW_CCU_GATE(g_usb_clk, "usb-hclk", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 19);

static AW_CCU_GATE(g_uart3_clk, "uart3", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_SPC), 0x0080, 18);

static AW_CCU_GATE(g_uart2_clk, "uart2", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_SPC), 0x0080, 17);

static AW_CCU_GATE(g_uart1_clk, "uart1", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_SPC), 0x0080, 16);

static AW_CCU_GATE(g_uart0_clk, "uart0", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_SPC), 0x0080, 15);

static AW_CCU_GATE(g_twi0_clk, "twi0", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_SPC), 0x0080, 14);

static AW_CCU_GATE(g_pwm_clk, "pwm", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 13);

static AW_CCU_GATE(g_trng_clk, "trng", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 11);

static AW_CCU_GATE(g_timer_clk, "timer", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 10);

static AW_CCU_GATE(g_sg_dma_clk, "sgdma", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 9);

static AW_CCU_GATE(g_dma_clk, "dma", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 8);

static AW_CCU_GATE(g_syscrtl_clk, "sysctrl", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 7);

static AW_CCU_GATE(g_ce_clk, "ce", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 6);

static AW_CCU_GATE(g_hstimer_clk, "hstimer", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 5);

static AW_CCU_GATE(g_splock_clk, "splock", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 4);

static AW_CCU_GATE(g_dram_bus_clk, "dram-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 3);

static AW_CCU_GATE(g_rv_msgbox_clk, "rv-msgbox-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 2);

static AW_CCU_GATE(g_riscv_clk, "riscv", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0080, 0);

static AW_CCU_GATE(g_g2d_mbus_clk, "g2d-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 31);

static AW_CCU_GATE(g_g2d_bus_clk, "g2d-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 30);

static AW_CCU_GATE(g_g2d_hb_clk, "g2d-hb", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 29);

static AW_CCU_GATE(g_twi2_clk, "twi2", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_SPC), 0x0084, 25);

static AW_CCU_GATE(g_twi1_clk, "twi1", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_SPC), 0x0084, 24);

static AW_CCU_GATE(g_spi2_bus_clk, "spi2-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 23);

static AW_CCU_GATE(g_gmac_bus_clk, "gmac-hbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 22);

static AW_CCU_GATE(g_smhc1_bus_clk, "smhc-bus1", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 21);

static AW_CCU_GATE(g_smhc0_bus_clk, "smhc-bus0", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 20);

static AW_CCU_GATE(g_spi1_bus_clk, "spi1-hbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 19);

static AW_CCU_GATE(g_gbgsys_clk, "dbgsys", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 18);

static AW_CCU_GATE(g_gmac_mbus_ahb_clk, "gmac-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 17);

static AW_CCU_GATE(g_smhc1_mbus_ahb_clk, "smhc1-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 16);

static AW_CCU_GATE(g_smhc0_mbus_ahb_clk, "smhc0-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 15);

static AW_CCU_GATE(g_usb_mbus_ahb_clk, "usb-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 14);

static AW_CCU_GATE(g_dma_mbus_clk, "dma-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 13);

static AW_CCU_GATE(g_i2s0_bus_clk, "i2s0-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 8);

static AW_CCU_GATE(g_adda_bus_clk, "adda-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 6);

static AW_CCU_GATE(g_spif_bus_clk, "spif-hbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 5);

static AW_CCU_GATE(g_spi_bus_clk, "spi-hbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 4);

static AW_CCU_GATE(g_ve_ahb_clk, "ve-ahb", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 3);

static AW_CCU_GATE(g_ve_mbus_clk, "ve-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 2);

static AW_CCU_GATE(g_ths_bus_clk, "ths-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 1);

static AW_CCU_GATE(g_gpa_clk, "gpa-bus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0084, 0);

static AW_CCU_GATE(g_mcsi_hbus_clk, "mcsi-hbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 28);

static AW_CCU_GATE(g_mcsi_sbus_clk, "mcsi-sclk", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 27);

static AW_CCU_GATE(g_misp_bus_clk, "misp-sclk", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 26);

static AW_CCU_GATE(g_res_dcap_24m_clk, "res_dcap_24m", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 10);

static AW_CCU_GATE(g_sd_monitor_clk, "sd-monitor", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 9);

static AW_CCU_GATE(g_ahb_monitor_clk, "ahb-monitor", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 8);

static AW_CCU_GATE(g_ve_sbus_clk, "ve-sclk", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 5);

static AW_CCU_GATE(g_ve_hbus_clk, "ve-hclk", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 4);

static AW_CCU_GATE(g_tcon_bus_clk, "tcon-hclk", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 3);

static AW_CCU_GATE(g_sgdma_clk, "sgdma-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 2);

static AW_CCU_GATE(g_de_bus_clk, "de-mbus", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 1);

static AW_CCU_GATE(g_de_hb_clk, "de-hb", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC), 0x0088, 0);



struct clk_hw *g_sun300iw1_clk_hws[] = {
	CCU_CLK_HW_ELEMENT(CLK_APP_DRAM, g_dram_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_E907_TS, g_e907_ts_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_AL27_MT, g_a27l2_mt_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SMHC0, g_smhc0_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SS, g_ss_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SPI, g_spi_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SPIF, g_spif_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MCSI, g_mcsi_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_CSI_MASTER0, g_csi_master0_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_CSI_MASTER1, g_csi_master1_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SPI2, g_spi2_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_TCONLCD, g_tconlcd_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_DE, g_de_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_G2D, g_g2d_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_GPADC0_24M, g_gpadc0_24m_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_VE, g_ve_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_AUDIO_DAC, g_audio_dac_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_AUDIO_ADC, g_audio_adc_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_I2S0, g_i2s0_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SMHC1, g_smhc1_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_PLL_AUDIO_4X, g_pll_audio_4x_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_PLL_AUDIO_1X, g_pll_audio_1x_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SPI1, g_spi1_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_GMAC_FANOUT, g_gmac_fanout_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_A27_MSGBOX, g_a27_msgbox_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_USB_24M, g_usb_24m_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_USB_12M, g_usb_12m_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_USB_48M, g_usb_48m_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_AVS, g_avs_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_GMAC_25M, g_gmac_25m_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_DPSS_TOP, g_dpss_top_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_GPIO, g_gpio_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MCSI_AHB, g_mcsi_ahb_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_MCSI, g_mcsi_mbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_VID_OUT, g_vid_out_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_VID_OUT, g_vid_out_mbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HBUS_GMAC, g_gmac_hbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_USBOHCI, g_usbohci_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_USBEHCI, g_usbehci_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_USBOTG, g_usbotg_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_USB, g_usb_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_UART3, g_uart3_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_UART2, g_uart2_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_UART1, g_uart1_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_UART0, g_uart0_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_TWI0, g_twi0_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_PWM, g_pwm_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_TRNG, g_trng_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_TIMER, g_timer_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SG_DMA, g_sg_dma_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_DMA, g_dma_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SYSCRTL, g_syscrtl_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_CE, g_ce_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HSTIMER, g_hstimer_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SPLOCK, g_splock_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_DRAM, g_dram_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_RV_MSGBOX, g_rv_msgbox_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_RISCV, g_riscv_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_G2D, g_g2d_mbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_G2D, g_g2d_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_G2D_HB, g_g2d_hb_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_TWI2, g_twi2_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_TWI1, g_twi1_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPI2, g_spi2_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_GMAC, g_gmac_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SMHC1, g_smhc1_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SMHC0, g_smhc0_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPI1, g_spi1_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_GBGSYS, g_gbgsys_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_GMAC, g_gmac_mbus_ahb_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_SMHC1, g_smhc1_mbus_ahb_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_SMHC0, g_smhc0_mbus_ahb_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_USB, g_usb_mbus_ahb_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_DMA, g_dma_mbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_I2S0, g_i2s0_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_ADDA, g_adda_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPIF, g_spif_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPI, g_spi_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_VE_AHB, g_ve_ahb_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_VE, g_ve_mbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_THS, g_ths_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_GPA, g_gpa_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HBUS_MCSI, g_mcsi_hbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SBUS_MCSI, g_mcsi_sbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_MISP, g_misp_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_24M_DCAP_RES, g_res_dcap_24m_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SD_MONITOR, g_sd_monitor_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_AHB_MONITOR, g_ahb_monitor_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SBUS_VE, g_ve_sbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HBUS_VE, g_ve_hbus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_TCON, g_tcon_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SGDMA, g_sgdma_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_DE, g_de_bus_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_DE_HB, g_de_hb_clk),
};


reset_hw_t g_sun300iw1_reset_hws[] =
{
	RESET_HW_ELEMENT(RST_APP_BUS_DPSSTOP, 0x0090, 31),
	RESET_HW_ELEMENT(RST_APP_BUS_MCSI, 0x0090, 28),
	RESET_HW_ELEMENT(RST_APP_BUS_HRESETN_MCSI, 0x0090, 27),
	RESET_HW_ELEMENT(RST_APP_BUS_G2D, 0x0090, 26),
	RESET_HW_ELEMENT(RST_APP_BUS_DE, 0x0090, 25),
	RESET_HW_ELEMENT(RST_APP_BUS_GMAC, 0x0090, 24),
	RESET_HW_ELEMENT(RST_APP_BUS_USBPHY, 0x0090, 23),
	RESET_HW_ELEMENT(RST_APP_BUS_USBOHCI, 0x0090, 22),
	RESET_HW_ELEMENT(RST_APP_BUS_USBEHCI, 0x0090, 21),
	RESET_HW_ELEMENT(RST_APP_BUS_USBOTG, 0x0090, 20),
	RESET_HW_ELEMENT(RST_APP_BUS_USB, 0x0090, 19),
	RESET_HW_ELEMENT(RST_APP_BUS_UART3, 0x0090, 18),
	RESET_HW_ELEMENT(RST_APP_BUS_UART2, 0x0090, 17),
	RESET_HW_ELEMENT(RST_APP_BUS_UART1, 0x0090, 16),
	RESET_HW_ELEMENT(RST_APP_BUS_UART0, 0x0090, 15),
	RESET_HW_ELEMENT(RST_APP_BUS_TWI0, 0x0090, 14),
	RESET_HW_ELEMENT(RST_APP_BUS_PWM, 0x0090, 13),
	RESET_HW_ELEMENT(RST_APP_BUS_TRNG, 0x0090, 11),
	RESET_HW_ELEMENT(RST_APP_BUS_TIMER, 0x0090, 10),
	RESET_HW_ELEMENT(RST_APP_BUS_SGDMA, 0x0090, 9),
	RESET_HW_ELEMENT(RST_APP_BUS_DMA, 0x0090, 8),
	RESET_HW_ELEMENT(RST_APP_BUS_SYSCTRL, 0x0090, 7),
	RESET_HW_ELEMENT(RST_APP_BUS_CE, 0x0090, 6),
	RESET_HW_ELEMENT(RST_APP_BUS_HSTIMER, 0x0090, 5),
	RESET_HW_ELEMENT(RST_APP_BUS_SPLOCK, 0x0090, 4),
	RESET_HW_ELEMENT(RST_APP_BUS_DRAM, 0x0090, 3),
	RESET_HW_ELEMENT(RST_APP_BUS_RV_MSGBOX, 0x0090, 2),
	RESET_HW_ELEMENT(RST_APP_BUS_RV_SYS_APB, 0x0090, 1),
	RESET_HW_ELEMENT(RST_APP_BUS_RV, 0x0090, 0),
	RESET_HW_ELEMENT(RST_APP_BUS_A27_CFG, 0x0094, 28),
	RESET_HW_ELEMENT(RST_APP_BUS_A27_MSGBOX, 0x0094, 27),
	RESET_HW_ELEMENT(RST_APP_BUS_A27, 0x0094, 26),
	RESET_HW_ELEMENT(RST_APP_BUS_TWI2, 0x0094, 25),
	RESET_HW_ELEMENT(RST_APP_BUS_TWI1, 0x0094, 24),
	RESET_HW_ELEMENT(RST_APP_BUS_SPI2, 0x0094, 23),
	RESET_HW_ELEMENT(RST_APP_BUS_SMHC1, 0x0094, 21),
	RESET_HW_ELEMENT(RST_APP_BUS_SMHC0, 0x0094, 20),
	RESET_HW_ELEMENT(RST_APP_BUS_SPI1, 0x0094, 19),
	RESET_HW_ELEMENT(RST_APP_BUS_DGBSYS, 0x0094, 18),
	RESET_HW_ELEMENT(RST_APP_BUS_MBUS, 0x0094, 12),
	RESET_HW_ELEMENT(RST_APP_BUS_TCONLCD, 0x0094, 11),
	RESET_HW_ELEMENT(RST_APP_BUS_TCON, 0x0094, 10),
	RESET_HW_ELEMENT(RST_APP_BUS_I2S0, 0x0094, 8),
	RESET_HW_ELEMENT(RST_APP_BUS_AUDIO, 0x0094, 6),
	RESET_HW_ELEMENT(RST_APP_BUS_SPIF, 0x0094, 5),
	RESET_HW_ELEMENT(RST_APP_BUS_SPI, 0x0094, 4),
	RESET_HW_ELEMENT(RST_APP_BUS_VE, 0x0094, 3),
	RESET_HW_ELEMENT(RST_APP_BUS_THS, 0x0094, 1),
	RESET_HW_ELEMENT(RST_APP_BUS_GPADC, 0x0094, 0),
	RESET_HW_ELEMENT(RST_APP_BUS_A27WFG, 0x0098, 2),
	RESET_HW_ELEMENT(RST_APP_BUS_GPIOWDG, 0x0098, 1),
	RESET_HW_ELEMENT(RST_APP_BUS_RV_WDG, 0x0098, 0),
	RESET_HW_ELEMENT(RST_APP_BUS_E907, 0x009C, 0),
};

clk_controller_t g_app_ccu =
{
	.id = AW_APP_CCU,
	.reg_base = APP_CCU_REG_BASE,

	.clk_num = ARRAY_SIZE(g_sun300iw1_clk_hws),
	.clk_hws = g_sun300iw1_clk_hws,

	.reset_num = ARRAY_SIZE(g_sun300iw1_reset_hws),
	.reset_hws = g_sun300iw1_reset_hws,
};

