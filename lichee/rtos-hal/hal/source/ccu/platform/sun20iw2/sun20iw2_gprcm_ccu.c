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

#include "sun20iw2_gprcm_ccu.h"


static CLK_FIXED_FACTOR(g_hosc_div_clk, "gprcm-hosc-div", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_DIV_32K), 1, 1);

static CLK_FIXED_FACTOR(g_ble_32m_div_clk, "gprcm-ble-32m-div", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_BLE_32M_DIV_32K), 1, 1);

//FIXME: the frequency of this clk is fixed regardless of RCCAL config
static CLK_FIXED_FACTOR(g_rccal_out_clk, "rccal-out", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RCCAL_OUT_32K), 1, 1);

/* System LFCLK Control Register */
static AW_CCU_GATE(g_losc_gate_clk, "losc-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_LOSC), 0x80, 31);

static AW_CCU_GATE(g_rc_lf_gate_clk, "rc-lf-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_LF), 0x80, 30);

static CLK_FIXED_FACTOR(g_rc_lf_fixed_div_clk, "rc-lf-fixed-div", 0,
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_RC_LF_GATE), 1, 32);

static AW_CCU_GATE(g_rc_hf_gate_clk, "rc-hf-gate", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_RC_HF), 0x80, 29);

static const clk_number_t g_sys32k_ble32k_parents[] =
{
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_LFCLK_MUX),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_RCCAL_OUT),
};
static AW_CCU_MUX(g_sys_32k_mux_clk, "sys-32k-mux", 0,
	g_sys32k_ble32k_parents, 0x80, 28, 1);

static AW_CCU_MUX(g_ble_32k_mux_clk, "ble-32k-mux", 0,
	g_sys32k_ble32k_parents, 0x80, 27, 1);

static const clk_number_t g_sysrtc_32k_parents[] =
{
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_LFCLK_MUX),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_RCCAL_OUT),
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_HOSC_DIV_32K),
};
static AW_CCU_MUX(g_sysrtc_32k_mux_clk, "sysrtc-32k-mux", 0,
	g_sysrtc_32k_parents, 0x80, 25, 2);

static const clk_number_t g_lfclk_parents[] =
{
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_RC_LF_FIXED_DIV),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_LOSC_GATE),
};
static AW_CCU_MUX(g_lfclk_mux_clk, "lfclk-mux", 0,
	g_lfclk_parents, 0x80, 24, 1);


/* BLE RCOSC Calibration Control Register0 */
/* Since there is no rccal clk hw type. So we provide a fake clk to be compatible with
   legacy code which is used in rccal driver
*/
static AW_CCU_GATE(g_rccal_wup_en, "rccal-wup-en", 0,
	MAKE_CLKn(AW_SRC_CCU, CLK_SRC_USELESS), 0x144, 16);

/* BLE CLK32 AUTO Switch Register */
static const clk_number_t g_ble_32k_2_parents[] =
{
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_HOSC_DIV),
	MAKE_CLKn(AW_GPRCM_CCU, CLK_GPRCM_BLE_32M_DIV),
};
static AW_CCU_MUX_LINK_GATE(g_ble_32k_2_clk, "blk-32k-2", 0,
	g_ble_32k_2_parents, 0x14C, 1, 1, 4);

static AW_CCU_GATE(g_ble_32k_auto_switch_en, "ble-32k-auto-switch-en", 0,
	MAKE_CLKn(AW_AON_CCU, CLK_GPRCM_BLE_32K_MUX), 0x14C, 0);

static struct clk_hw *g_sun20iw2_gprcm_clk_hws[] =
{
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_SYSRTC_32K_MUX, g_sysrtc_32k_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_SYS_32K_MUX, g_sys_32k_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_BLE_32K_MUX, g_ble_32k_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_LFCLK_MUX, g_lfclk_mux_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_LOSC_GATE, g_losc_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_RC_LF_GATE, g_rc_lf_gate_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_RC_HF_GATE, g_rc_hf_gate_clk),
	CLK_HW_ELEMENT(CLK_GPRCM_RC_LF_FIXED_DIV, g_rc_lf_fixed_div_clk),
	CLK_HW_ELEMENT(CLK_GPRCM_RCCAL_OUT, g_rccal_out_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_RCCAL_WUP_EN, g_rccal_wup_en),
	CLK_HW_ELEMENT(CLK_GPRCM_HOSC_DIV, g_hosc_div_clk),
	CLK_HW_ELEMENT(CLK_GPRCM_BLE_32M_DIV, g_ble_32m_div_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_BLE_32K_2, g_ble_32k_2_clk),
	CCU_CLK_HW_ELEMENT(CLK_GPRCM_BLE_32K_AUTO_SWITCH_EN, g_ble_32k_auto_switch_en),
};

clk_controller_t g_gprcm_ccu =
{
	.id = AW_GPRCM_CCU,
	.reg_base = GPRCM_CCU_REG_BASE,

	.clk_num = ARRAY_SIZE(g_sun20iw2_gprcm_clk_hws),
	.clk_hws = g_sun20iw2_gprcm_clk_hws,

	.reset_num = 0,
	.reset_hws = NULL,
};

