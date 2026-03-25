/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
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

#include "plat_fc_common.h"
#include "../flash_controller.h"

#include <hal_time.h>
#include <hal_clk.h>
#include <hal_reset.h>

static flash_controller_t s_plat_fc_arr[PLAT_FC_MAX_NUM];

int plat_is_integrated_spi_nor(void);

int plat_fc_init(flash_controller_t *fc)
{
	fc->reg_base_addr = PLAT_FC_REG_BASE_ADDR;
	fc->start_addr = PLAT_FC_MEM_MAP_START_ADDR;
	fc->end_addr = PLAT_FC_MEM_MAP_END_ADDR;

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
	fc->input_clk_type = PLAT_FC_INPUT_CLK_TYPE;
	fc->input_clk_id = PLAT_FC_INPUT_CLK_ID;

	if (plat_is_integrated_spi_nor())
	{
		fc->pinmux_arr = g_integrated_nor_pinmux;
		fc->pin_num = ARRAY_SIZE(g_integrated_nor_pinmux);
		fc_info("Intergated SPI NOR flash");
	}
	else
	{
		fc->pinmux_arr = g_external_nor_pinmux;
		fc->pin_num = ARRAY_SIZE(g_external_nor_pinmux);
		fc_info("External SPI NOR flash");
	}
#endif
	return 0;
}

flash_controller_t *plat_get_fc_object(unsigned int id)
{
	if (id >= PLAT_FC_MAX_NUM)
		return NULL;

	return &s_plat_fc_arr[id];
}

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
/*
 * if the specific platform need to setup more parent clock, you can override the weak function
 * plat_fc_init_module_clk on plat_fc_sunxxx.c.
 */
__attribute__((weak)) int plat_fc_init_module_clk(const flash_controller_t *fc,
		uint32_t clk_freq, uint32_t *actual_freq)
{
	int ret = 0;
	hal_clk_status_t clk_ret;
	hal_clk_type_t mclk_type, pclk_type;
	hal_clk_id_t mclk_id, pclk_id;
	hal_clk_t mclk, pclk;
	uint32_t freq;

	pclk_type = PLAT_FC_PARENT_CLK_TYPE;
	pclk_id = PLAT_FC_PARENT_CLK_ID;
	pclk = hal_clock_get(pclk_type, pclk_id);
	if (!pclk)
	{
		fc_err("get clk handle failed, type: %d, id: %d", pclk_type, pclk_id);
		return -FC_RET_CODE_GET_CLK_FAILED;
	}

	mclk_type = fc->input_clk_type;
	mclk_id = fc->input_clk_id;
	mclk = hal_clock_get(mclk_type, mclk_id);
	if (!mclk)
	{
		fc_err("get clk handle failed, type: %d, id: %d", mclk_type, mclk_id);
		ret = -FC_RET_CODE_GET_CLK_FAILED;
		goto exit_with_put_pclk;
	}

	clk_ret = hal_clk_set_parent(mclk, pclk);
	if (clk_ret)
	{
		fc_err("hal_clk_set_parent failed, ret: %d", clk_ret);
		ret = -FC_RET_CODE_SET_CLK_PARENT_FAILED;
		goto exit_with_put_mclk;
	}

	clk_ret = hal_clk_set_rate(mclk, clk_freq);
	if (clk_ret)
	{
		fc_err("hal_clk_set_rate failed, ret: %d", clk_ret);
		ret = -FC_RET_CODE_SET_CLK_FREQ_FAILED;
		goto exit_with_put_mclk;
	}

	clk_ret = hal_clock_enable(mclk);
	if (clk_ret)
	{
		fc_err("hal_clock_enable failed, ret: %d", clk_ret);
		ret = -FC_RET_CODE_ENABLE_CLK_FAILED;
		goto exit_with_put_mclk;
	}

	freq = hal_clk_get_rate(mclk);

	if (actual_freq)
		*actual_freq = freq;

	fc_info("FC module clk init success! freq: %uHz, target: %uHz", freq, clk_freq);

exit_with_put_mclk:
	hal_clock_put(mclk);

exit_with_put_pclk:
	hal_clock_put(pclk);

	return ret;
}

__attribute__((weak)) int plat_fc_reset_module(const flash_controller_t *fc)
{
	hal_reset_type_t reset_type = PLAT_FC_RESET_TYPE;
	u32 reset_id = PLAT_FC_RESET_ID;
	struct reset_control *reset;

	reset = hal_reset_control_get(reset_type, reset_id);
	hal_reset_control_assert(reset);
	hal_udelay(100);
	hal_reset_control_deassert(reset);
	hal_reset_control_put(reset);

	reset_type = PLAT_FC_ENC_RESET_TYPE;
	reset_id = PLAT_FC_ENC_RESET_ID;

	reset = hal_reset_control_get(reset_type, reset_id);
	hal_reset_control_assert(reset);
	hal_udelay(100);
	hal_reset_control_deassert(reset);
	hal_reset_control_put(reset);
	return 0;
}

__attribute__((weak)) int plat_is_integrated_spi_nor(void)
{
	return 0;
}

__attribute__((weak)) int plat_fc_init_pinmux(const flash_controller_t *fc)
{
	const fc_pinmux_info_t *pinmux_info;
	unsigned int i, pin_num;

	pinmux_info = fc->pinmux_arr;
	pin_num = fc->pin_num;

	for (i = 0; i < pin_num; i++)
	{
		hal_gpio_pinmux_set_function(pinmux_info[i].pin, GPIO_MUXSEL_DISABLED);
	}
	for (i = 0; i < pin_num; i++)
	{
		hal_gpio_set_driving_level(pinmux_info[i].pin, pinmux_info[i].drv_level);
		hal_gpio_set_pull(pinmux_info[i].pin, pinmux_info[i].pull_cfg);
		hal_gpio_pinmux_set_function(pinmux_info[i].pin, pinmux_info[i].func);
	}
	return 0;
}

#endif
