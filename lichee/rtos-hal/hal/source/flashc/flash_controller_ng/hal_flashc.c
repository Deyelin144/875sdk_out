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

#include <sunxi_hal_flashctrl.h>

#include "flash_controller.h"

#include <stdio.h>
#include <string.h>

#include "platform_flashc.h"

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
static fc_spi_timing_cfg_t s_default_spi_timing_cfg =
{
	.clk_polarity = 0,
	.clk_phase = 0,
	.cs_active_low = 1,
	.rx_lsb_first = 0,
};
#endif

int hal_flash_ctrl_init(unsigned int id, unsigned int clk_freq)
{
	int ret = 0;
	flash_controller_t *fc = plat_get_fc_object(id);
	if (!fc)
		return -1;

	ret = plat_fc_init(fc);
	if (ret)
	{
		return -2;
	}

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
	fc->spi_timing_cfg = s_default_spi_timing_cfg;
#endif
	return flash_controller_init(fc, clk_freq);
}

int hal_flash_ctrl_exec_operation(unsigned int id, const fc_operation_t *opr_info)
{
	flash_controller_t *fc = plat_get_fc_object(id);
	if (!fc)
		return -1;

	return flash_controller_exec_operation(fc, opr_info);
}

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
int hal_flash_ctrl_set_output_clk_freq(unsigned int id, unsigned int clk_freq)
{
	flash_controller_t *fc = plat_get_fc_object(id);
	if (!fc)
		return -1;


	return flash_controller_set_output_clk_freq(fc, clk_freq);
}

int hal_flash_ctrl_setup_mem_map_opr(unsigned int id, const fc_mem_map_operation_t *opr)
{
	flash_controller_t *fc = plat_get_fc_object(id);
	if (!fc)
		return -1;


	return flash_controller_setup_mem_map_opr(fc, opr);
}

int hal_flash_ctrl_add_mem_map(unsigned int id, const fc_mem_map_info_t *mmap)
{
	flash_controller_t *fc = plat_get_fc_object(id);
	if (!fc)
		return -1;


	return flash_controller_add_mem_map(fc, mmap);
}

int hal_flash_ctrl_del_mem_map(unsigned int id, const fc_mem_map_info_t *mmap)
{
	flash_controller_t *fc = plat_get_fc_object(id);
	if (!fc)
		return -1;

	return -2;
}
#else
int hal_flash_ctrl_set_output_clk_freq(unsigned int id, unsigned int clk_freq)
{
	return -1;
}
#endif

