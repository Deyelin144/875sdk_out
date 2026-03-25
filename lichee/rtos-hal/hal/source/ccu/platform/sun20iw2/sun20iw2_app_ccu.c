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

#include "sun20iw2_app_ccu.h"

/* BUS Clock Gating Control Register0 */
static AW_CCU_GATE(g_bus_usb_ehci_gate_clk, "bus-usb-ehci-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (31));

static AW_CCU_GATE(g_bus_usb_ohci_gate_clk, "bus-usb-ohci-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (30));

static AW_CCU_GATE(g_bus_csi_jpe_gate_clk, "bus-csi-jpe-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (29));

static AW_CCU_GATE(g_bus_ledc_gate_clk, "bus-ledc-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (28));

static AW_CCU_GATE(g_bus_usb_otg_gate_clk, "bus-usb-otg-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (27));

static AW_CCU_GATE(g_bus_smcard_gate_clk, "bus-smcard-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (26));

static AW_CCU_GATE(g_bus_hspsram_ctrl_gate_clk, "bus-hspsram-ctrl-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (21));

static AW_CCU_GATE(g_bus_ir_rx_gate_clk, "bus-ir-rx-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (16));

static AW_CCU_GATE(g_bus_ir_tx_gate_clk, "bus-ir-tx-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x004, (15));

static AW_CCU_GATE(g_bus_pwm_gate_clk, "bus-pwm-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x004, (14));

static AW_CCU_GATE(g_bus_twi1_gate_clk, "bus-twi1-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x004, (11));

static AW_CCU_GATE(g_bus_twi0_gate_clk, "bus-twi0-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x004, (10));

static AW_CCU_GATE(g_bus_uart2_gate_clk, "bus-uart2-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x004, (8));

static AW_CCU_GATE(g_bus_uart1_gate_clk, "bus-uart1-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x004, (7));

static AW_CCU_GATE(g_bus_uart0_gate_clk, "bus-uart0-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x004, (6));

static AW_CCU_GATE(g_bus_sdc0_gate_clk, "bus-sdc0-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x004, (4));

static AW_CCU_GATE(g_bus_spi1_gate_clk, "bus-spi1-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x004, (1));

static AW_CCU_GATE(g_bus_spi0_gate_clk, "bus-spi0-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x004, (0));

/* BUS Clock Gating Control Register1 */
static AW_CCU_GATE(g_bus_ahb_monitor_gate_clk, "bus-ahb-monitor-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (28));
static AW_CCU_GATE(g_bus_g2d_gate_clk, "bus-g2d-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (27));
static AW_CCU_GATE(g_bus_de_gate_clk, "bus-de-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (26));
static AW_CCU_GATE(g_bus_display_gate_clk, "bus-display-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (25));
static AW_CCU_GATE(g_bus_lcd_gate_clk, "bus-lcd-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (24));

static AW_CCU_GATE(g_bus_bt_core_gate_clk, "bus-bt-core-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (21));
static AW_CCU_GATE(g_bus_wlan_ctrl_gate_clk, "bus-wlan-ctrl-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (20));

static AW_CCU_GATE(g_bus_trng_gate_clk, "bus-trng-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (14));
static AW_CCU_GATE(g_bus_spc_gate_clk, "bus-spc-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (13));
static AW_CCU_GATE(g_bus_ss_gate_clk, "bus-ss-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (12));
static AW_CCU_GATE(g_bus_timer_gate_clk, "bus-timer-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_AHB_MUX), 0x008, (11));
static AW_CCU_GATE(g_bus_spinlock_gate_clk, "bus-spinlock-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x008, (10));

static AW_CCU_GATE(g_bus_dma1_gate_clk, "bus-dma1-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x008, (7));
static AW_CCU_GATE(g_bus_dma0_gate_clk, "bus-dma0-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x008, (6));

static AW_CCU_GATE(g_bus_spdif_gate_clk, "bus-spdif-gate", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_APB_MUX), 0x008, (2));
static AW_CCU_GATE(g_bus_i2s_gate_clk, "bus-i2s-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x008, (1));

/* CPU_DSP_RV Systems Clock Gating Control Register */
static AW_CCU_GATE(g_c906_cfg_gate_clk, "c906-cfg-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x014, (19));
static AW_CCU_GATE(g_c906_msgbox_gate_clk, "c906-msgbox-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x014, (18));

static AW_CCU_GATE(g_hifi5_cfg_gate_clk, "hifi5-cfg-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x014, (11));
static AW_CCU_GATE(g_hifi5_msgbox_gate_clk, "hifi5-msgbox-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x014, (10));
static AW_CCU_GATE(g_m33_msgbox_gate_clk, "m33-msgbox-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x014, (1));

/* MBUS Clock Gating Control Register */
static AW_CCU_GATE(g_mbus_de_gate_clk, "mbus-de-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (9));
static AW_CCU_GATE(g_mbus_g2d_gate_clk, "mbus-g2d-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (8));
static AW_CCU_GATE(g_mbus_csi_gate_clk, "mbus-csi-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (7));
static AW_CCU_GATE(g_mbus_dma1_gate_clk, "mbus-dma1-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (6));
static AW_CCU_GATE(g_mbus_dma0_gate_clk, "mbus-dma0-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (5));
static AW_CCU_GATE(g_mbus_usb_gate_clk, "mbus-usb-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (4));
static AW_CCU_GATE(g_mbus_ce_gate_clk, "mbus-ce-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (3));
static AW_CCU_GATE(g_mbus_hifi5_gate_clk, "mbus-hifi5-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (2));
static AW_CCU_GATE(g_mbus_c906_gate_clk, "mbus-c906-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (1));
static AW_CCU_GATE(g_mbus_m33_gate_clk, "mbus-m33-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x01C, (0));

static const clk_number_t g_module_common1_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DEVICE_DIV),
};

/* SPI0 Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_spi0_clk, "spi0", 0,
	g_module_common1_parents, 0x020, 24, 2, 16, 2, 0, 4, 31);

/* SPI1 Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_spi1_clk, "spi1", 0,
	g_module_common1_parents, 0x024, 24, 2, 16, 2, 0, 4, 31);

/* SDC Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_sdc0_clk, "sdc0", 0,
	g_module_common1_parents, 0x028, 24, 2, 16, 2, 0, 4, 31);

/* SS Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_ss_clk, "ss", 0,
	g_module_common1_parents, 0x02c, 24, 2, 16, 2, 0, 4, 31);

/* CSI_JPE Device CLK Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_csi_jpe_clk, "csi-jpe", 0,
	g_module_common1_parents, 0x030, 24, 2, 16, 2, 0, 4, 31);

/* LEDC Clocok Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_ledc_clk, "ledc", 0,
	g_module_common1_parents, 0x034, 24, 2, 16, 2, 0, 4, 31);

/* IRRX Clock Control Register */
static const clk_number_t g_module_common2_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_SYS_32K_MUX),
};
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_ir_rx_clk, "ir-rx", 0,
	g_module_common2_parents, 0x038, 24, 2, 16, 2, 0, 4, 31);

/* IRTX Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_ir_tx_clk, "ir-tx", 0,
	g_module_common2_parents, 0x03c, 24, 2, 16, 2, 0, 4, 31);

/* System Tick Reference Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_systick_ref_clk, "systick-ref", 0,
	g_module_common2_parents, 0x040, 24, 2, 16, 2, 0, 4, 31);

/* System Tick Clock Calibration Register */
static AW_CCU_GATE(g_systick_noref_gate_clk, "systick_noref-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x044,(25));
static AW_CCU_GATE(g_systick_skew_gate_clk, "systick-skew-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_UNKNOWN), 0x044, (24));

/* CSI Output MCLK Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_csi_clk, "csi", 0,
	g_module_common1_parents, 0x050, 24, 2, 16, 2, 0, 4, 31);

/* Flash Controller SPI Clock Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_flash_controller_clk, "flash-controller", 0,
	g_module_common1_parents, 0x054, 24, 2, 16, 2, 0, 4, 31);

/* APB_SPC Clock Control Register */
static const clk_number_t g_apb_spc_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_DEVICE_DIV),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_SYS_32K_MUX),
};
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_apb_spc_clk, "apb-spc", 0,
	g_apb_spc_parents, 0x05c, 24, 2, 16, 2, 0, 4, 31);

//TODO：spec里bit0为保留字段!
/* USB Clock Control Register */
static AW_CCU_MUX(g_usb_mux_clk, "usb-mux", 0,
	g_module_common1_parents, 0x060, 0, 1);

/* RISCV Clock Control Register */
static const clk_number_t g_c906_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_SYS_32K_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_C906_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_C906_MUX),
};

static division_map_t g_c906_axi_div_map[] =
{
	//{ .field_value = 0, .div_value = 1 },
	{ .field_value = 1, .div_value = 2 },
	{ .field_value = 2, .div_value = 3 },
	{ .field_value = 3, .div_value = 4 },
	{ /* Sentinel */ },
};

/* C906 related clock hw can be composited, but we can't composite it 
for compatibility with legacy code */
#if 0
static AW_CCU_MUX_LINK_PDIV_LINK_GATE(g_c906_clk, "c906", 0,
	g_c906_parents, 0x064, 4, 2, 0, 2, 31);

static AW_CCU_TDIV(g_c906_axi_div_clk, "c906-axi-div", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_C906), 0x064, 8, 2, g_c906_axi_div_map);
#else
static AW_CCU_MUX(g_c906_mux_clk, "c906-mux", 0,
	g_c906_parents, 0x064, 4, 2);

static AW_CCU_GATE(g_c906_gate_clk, "c906-gate", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_C906_MUX), 0x064, (31));

static AW_CCU_PDIV(g_c906_div_clk, "c906-div", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_C906_GATE), 0x064, 0, 2);

static AW_CCU_TDIV(g_c906_axi_div_clk, "c906-axi-div", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_C906_DIV), 0x064, 8, 2, g_c906_axi_div_map);
#endif


/* DSP Clock Control Register */
static const clk_number_t g_hifi5_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_SYS_32K_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_HIFI5_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_HIFI5_MUX),
};
/* DSP related clock hw can be composited, but we can't composite it 
for compatibility with legacy code */
#if 0
static AW_CCU_MUX_LINK_PDIV_LINK_GATE(g_c906_clk, "c906", 0,
	g_c906_parents, 0x064, 4, 2, 0, 2, 31);

static AW_CCU_LDIV(g_c906_axi_div_clk, "c906-axi-div", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_C906), 0x064, 8, 2);
#else
static AW_CCU_MUX(g_hifi5_mux_clk, "hifi5-mux", 0,
		g_hifi5_parents, 0x068, 4, 2);
static AW_CCU_GATE(g_hifi5_gate_clk, "hifi5-gate", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_HIFI5_MUX), 0x068, (31));

static AW_CCU_PDIV(g_hifi5_div_clk, "hifi5-div", 0,
		MAKE_CLKn(AW_APP_CCU, CLK_APP_HIFI5_GATE), 0x068, 0, 2);
#endif

/* HSPSRAM Clock Control Register */
static CLK_FIXED_FACTOR(g_hspsram_fixed_div_clk, "hspsram-fixed-div", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_HSPSRAM_MUX), 1, 8);

static AW_CCU_GATE(g_hspsram_gate_clk, "hspsram-gate", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_HSPSRAM_FIXED_DIV), 0x06c, (31));

/* LSPSRAM Clock Control Register */
static const clk_number_t g_lspsram_parents[] =
{
	MAKE_CLKn(AW_AON_CCU, CLK_AON_HOSC_FAKE_MUX),
	MAKE_CLKn(AW_AON_CCU, CLK_AON_CK_LSPSRAM_MUX),
};

#if 1
static AW_CCU_MUX_LINK_PDIV_LINK_GATE(g_lspsram_clk, "lspsram", 0,
	g_lspsram_parents, 0x70, 4, 1, 0, 2, 31);

static CLK_FIXED_FACTOR(g_lspsram_fixed_div_clk, "lspsram-fixed-div", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_LSPSRAM), 1, 2);

#else
static AW_CCU_MUX(g_lspsram_mux_clk, "lspsram-mux", 0,
	g_lspsram_parents, 0x070, 4, 1, 0);

static AW_CCU_GATE(g_lspsram_gate_clk, "lspsram-gate", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_LSPSRAM_MUX), 0x070, (31));

static AW_CCU_PDIV(g_lspsram_div_clk, "lspsram-div", 0,
		MAKE_CLKn(AW_APP_CCU, CLK_APP_LSPSRAM_GATE), 0x070, 0, 2);

static CLK_FIXED_FACTOR(g_lspsram_fixed_div_clk, "lspsram-fixed-div", 0,
	MAKE_CLKn(AW_APP_CCU, CLK_APP_LSPSRAM_DIV), 1, 2);
#endif


/* G2D Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_g2d_clk, "g2d", 0,
	g_module_common1_parents, 0x074, 24, 2, 16, 2, 0, 4, 31);

/* DE Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_de_clk, "de", 0,
	g_module_common1_parents, 0x078, 24, 2, 16, 2, 0, 4, 31);

/* LCD Clock Control Register */
static AW_CCU_MUX_LINK_PDIV_LDIV_LINK_GATE(g_lcd_clk, "lcd", 0,
	g_module_common1_parents, 0x07c, 24, 2, 16, 2, 0, 4, 31);


struct clk_hw *g_sun20iw2_clk_hws[] =
{
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_USB_EHCI_GATE, g_bus_usb_ehci_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_USB_OHCI_GATE, g_bus_usb_ohci_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_CSI_JPE_GATE, g_bus_csi_jpe_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_LEDC_GATE, g_bus_ledc_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_USB_OTG_GATE, g_bus_usb_otg_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SMCARD_GATE, g_bus_smcard_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_HSPSRAM_CTRL_GATE, g_bus_hspsram_ctrl_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_IR_RX_GATE, g_bus_ir_rx_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_IR_TX_GATE, g_bus_ir_tx_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_PWM_GATE, g_bus_pwm_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_TWI0_GATE, g_bus_twi0_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_TWI1_GATE, g_bus_twi1_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_UART0_GATE, g_bus_uart0_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_UART1_GATE, g_bus_uart1_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_UART2_GATE, g_bus_uart2_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SDC0_GATE, g_bus_sdc0_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPI0_GATE, g_bus_spi0_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPI1_GATE, g_bus_spi1_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_AHB_MONITOR_GATE, g_bus_ahb_monitor_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_G2D_GATE, g_bus_g2d_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_DE_GATE, g_bus_de_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_DISPLAY_GATE, g_bus_display_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_LCD_GATE, g_bus_lcd_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_BT_CORE_GATE, g_bus_bt_core_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_WLAN_CTRL_GATE, g_bus_wlan_ctrl_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_TRNG_GATE, g_bus_trng_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPC_GATE, g_bus_spc_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SS_GATE, g_bus_ss_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_TIMER_GATE, g_bus_timer_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPINLOCK_GATE, g_bus_spinlock_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_DMA0_GATE, g_bus_dma0_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_DMA1_GATE, g_bus_dma1_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_SPDIF_GATE, g_bus_spdif_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_BUS_I2S_GATE, g_bus_i2s_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_C906_CFG_GATE, g_c906_cfg_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_C906_MSGBOX_GATE, g_c906_msgbox_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HIFI5_CFG_GATE, g_hifi5_cfg_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HIFI5_MSGBOX_GATE, g_hifi5_msgbox_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_M33_MSGBOX_GATE, g_m33_msgbox_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_DE_GATE, g_mbus_de_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_G2D_GATE, g_mbus_g2d_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_CSI_GATE, g_mbus_csi_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_DMA0_GATE, g_mbus_dma0_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_DMA1_GATE, g_mbus_dma1_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_USB_GATE, g_mbus_usb_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_CE_GATE, g_mbus_ce_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_HIFI5_GATE, g_mbus_hifi5_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_C906_GATE, g_mbus_c906_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_MBUS_M33_GATE, g_mbus_m33_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SPI0, g_spi0_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SPI1, g_spi1_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SDC0, g_sdc0_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SS, g_ss_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_CSI_JPE, g_csi_jpe_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_LEDC, g_ledc_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_IR_RX, g_ir_rx_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_IR_TX, g_ir_tx_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SYSTICK_REF, g_systick_ref_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SYSTICK_NOREF_GATE, g_systick_noref_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_SYSTICK_SKEW_GATE, g_systick_skew_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_CSI, g_csi_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_FLASH_CONTROLLER, g_flash_controller_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_APB_SPC, g_apb_spc_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_USB_MUX, g_usb_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_C906_MUX, g_c906_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_C906_GATE, g_c906_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_C906_DIV, g_c906_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_C906_AXI_DIV, g_c906_axi_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HIFI5_MUX, g_hifi5_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HIFI5_GATE, g_hifi5_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HIFI5_DIV, g_hifi5_div_clk),
	CLK_HW_ELEMENT(CLK_APP_HSPSRAM_FIXED_DIV, g_hspsram_fixed_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_HSPSRAM_GATE, g_hspsram_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_LSPSRAM, g_lspsram_clk),
	CLK_HW_ELEMENT(CLK_APP_LSPSRAM_FIXED_DIV, g_lspsram_fixed_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_G2D, g_g2d_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_DE, g_de_clk),
	CCU_CLK_HW_ELEMENT(CLK_APP_LCD, g_lcd_clk),
};


reset_hw_t g_sun20iw2_reset_hws[] =
{
	RESET_HW_ELEMENT(RST_APP_USB_EHCI, 0xC, 31),
	RESET_HW_ELEMENT(RST_APP_USB_OHCI, 0xC, 30),
	RESET_HW_ELEMENT(RST_APP_CSI_JPE, 0xC, 29),
	RESET_HW_ELEMENT(RST_APP_LEDC, 0xC, 28),
	RESET_HW_ELEMENT(RST_APP_USB_OTG, 0xC, 27),
	RESET_HW_ELEMENT(RST_APP_SMCARD, 0xC, 26),
	RESET_HW_ELEMENT(RST_APP_USB_PHY, 0xC, 25),
	RESET_HW_ELEMENT(RST_APP_FLASH_ENC, 0xC, 23),
	RESET_HW_ELEMENT(RST_APP_FLASH_CTRL, 0xC, 22),
	RESET_HW_ELEMENT(RST_APP_HSPSRAM_CTRL, 0xC, 21),
	RESET_HW_ELEMENT(RST_APP_LSPSRAM_CTRL, 0xC, 20),
	RESET_HW_ELEMENT(RST_APP_IRRX, 0xC, 16),
	RESET_HW_ELEMENT(RST_APP_IRTX, 0xC, 15),
	RESET_HW_ELEMENT(RST_APP_PWM, 0xC, 14),
	RESET_HW_ELEMENT(RST_APP_TWI1, 0xC, 11),
	RESET_HW_ELEMENT(RST_APP_TWI0, 0xC, 10),
	RESET_HW_ELEMENT(RST_APP_UART2, 0xC, 8),
	RESET_HW_ELEMENT(RST_APP_UART1, 0xC, 7),
	RESET_HW_ELEMENT(RST_APP_UART0, 0xC, 6),
	RESET_HW_ELEMENT(RST_APP_SDC0, 0xC, 4),
	RESET_HW_ELEMENT(RST_APP_SPI1, 0xC, 1),
	RESET_HW_ELEMENT(RST_APP_SPI0, 0xC, 0),

	RESET_HW_ELEMENT(RST_APP_G2D, 0x10, 27),
	RESET_HW_ELEMENT(RST_APP_DE, 0x10, 26),
	RESET_HW_ELEMENT(RST_APP_DISPLAY, 0x10, 25),
	RESET_HW_ELEMENT(RST_APP_LCD, 0x10, 24),
	RESET_HW_ELEMENT(RST_APP_BT_CORE, 0x10, 21),
	RESET_HW_ELEMENT(RST_APP_WLAN_CTRL, 0x10, 20),
	RESET_HW_ELEMENT(RST_APP_TRNG, 0x10, 14),
	RESET_HW_ELEMENT(RST_APP_SPC, 0x10, 13),
	RESET_HW_ELEMENT(RST_APP_SS, 0x10, 12),
	RESET_HW_ELEMENT(RST_APP_TIMER, 0x10, 11),
	RESET_HW_ELEMENT(RST_APP_SPINLOCK, 0x10, 10),
	RESET_HW_ELEMENT(RST_APP_DMA1, 0x10, 7),
	RESET_HW_ELEMENT(RST_APP_DMA0, 0x10, 6),
	RESET_HW_ELEMENT(RST_APP_SPDIF, 0x10, 2),
	RESET_HW_ELEMENT(RST_APP_I2S, 0x10, 1),


	RESET_HW_ELEMENT(RST_APP_C906_SYS_APB_SOFT, 0x18, 21),
	RESET_HW_ELEMENT(RST_APP_C906_TIMESTAMP, 0x18, 20),
	RESET_HW_ELEMENT(RST_APP_C906_CFG, 0x18, 19),
	RESET_HW_ELEMENT(RST_APP_C906_MSGBOX, 0x18, 18),
	RESET_HW_ELEMENT(RST_APP_C906_WDG, 0x18, 17),
	RESET_HW_ELEMENT(RST_APP_C906_CORE, 0x18, 16),
	RESET_HW_ELEMENT(RST_APP_HIFI5_DEBUG, 0x18, 14),
	RESET_HW_ELEMENT(RST_APP_HIFI5_INTC, 0x18, 13),
	RESET_HW_ELEMENT(RST_APP_HIFI5_TZMA, 0x18, 12),
	RESET_HW_ELEMENT(RST_APP_HIFI5_CFG, 0x18, 11),
	RESET_HW_ELEMENT(RST_APP_HIFI5_MSGBOX, 0x18, 10),
	RESET_HW_ELEMENT(RST_APP_HIFI5_WDG, 0x18, 9),
	RESET_HW_ELEMENT(RST_APP_HIFI5_CORE, 0x18, 8),
	RESET_HW_ELEMENT(RST_APP_M33_CFG, 0x18, 2),
	RESET_HW_ELEMENT(RST_APP_M33_MSGBOX, 0x18, 1),
	RESET_HW_ELEMENT(RST_APP_M33_WDG, 0x18, 0),
};

clk_controller_t g_app_ccu =
{
	.id = AW_APP_CCU,
	.reg_base = APP_CCU_REG_BASE,

	.clk_num = ARRAY_SIZE(g_sun20iw2_clk_hws),
	.clk_hws = g_sun20iw2_clk_hws,

	.reset_num = ARRAY_SIZE(g_sun20iw2_reset_hws),
	.reset_hws = g_sun20iw2_reset_hws,
};

