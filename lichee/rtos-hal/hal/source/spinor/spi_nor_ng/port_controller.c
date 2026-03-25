/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
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

#include <stdio.h>
#include <string.h>

#include <sunxi_hal_flashctrl.h>

#include "spi_nor.h"


#define DEFAULT_SPI_NOR_CONTROLLER_ID 0

static int port_convert_dummy_info(const spi_nor_protocol_info_t *snor_proto,
								   const uint8_t *mode_buf, uint32_t mode_buf_len,
								   fc_protocol_info_t *fc_proto,
								   uint8_t *dummy_buf, uint32_t dummy_buf_len)
{
	if ((snor_proto->mode_clocks_num) || (snor_proto->dummy_clocks_num))
	{
		uint32_t buf_len, byte_len, totol_bit, left_bit, bit_mask;

		fc_proto->dummy_field_line_num = snor_proto->addr_field_line_num;

		totol_bit = snor_proto->mode_clocks_num + snor_proto->dummy_clocks_num;
		totol_bit = totol_bit * snor_proto->addr_field_line_num;
		if (totol_bit % 8)
			snor_warn("abnormal mode clocks(%u) or dummy clocks(%u)",
					  snor_proto->mode_clocks_num, snor_proto->dummy_clocks_num);

		fc_proto->dummy_field_byte_len = (totol_bit + 7) >> 3;

		totol_bit = snor_proto->mode_clocks_num * snor_proto->addr_field_line_num;
		byte_len = totol_bit >> 3;
		left_bit = totol_bit % 8;

		buf_len = mode_buf_len;
		if (buf_len > dummy_buf_len)
			buf_len = dummy_buf_len;

		if ((byte_len > buf_len) ||
				((byte_len == buf_len) && (left_bit != 0)))
		{
			snor_err("invalid mode field byte len: %u, buf_len: %u", byte_len, buf_len);
			return -1;
		}

		memset(dummy_buf, 0, dummy_buf_len);
		if (byte_len)
			memcpy(dummy_buf, mode_buf, byte_len);

		if (left_bit)
		{
			bit_mask = 0xFF;
			bit_mask >>= (8 - left_bit);
			dummy_buf[byte_len] = (mode_buf[byte_len] & bit_mask);
		}
	}
	else
	{
		fc_proto->dummy_field_line_num = 0;
	}

	return 0;
}

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)

static int port_fc_setup_mem_map_proto(unsigned int id, const spi_nor_mem_map_proto_t *mmap_proto)
{
	int ret = 0;
	fc_mem_map_operation_t fc_opr;
	fc_protocol_info_t *fc_proto = &fc_opr.proto;

	fc_proto->cmd_field_line_num = mmap_proto->proto.cmd_field_line_num;
	fc_proto->addr_field_line_num = mmap_proto->proto.addr_field_line_num;
	fc_proto->payload_field_line_num = mmap_proto->proto.payload_field_line_num;

	fc_opr.cmd = mmap_proto->cmd;

	if (mmap_proto->proto.addr_field_line_num)
	{
		fc_proto->addr_field_byte_len = mmap_proto->proto.addr_field_byte_len;
	}

	ret = port_convert_dummy_info(&mmap_proto->proto, mmap_proto->mode, sizeof(mmap_proto->mode),
								  fc_proto, (uint8_t *)fc_opr.dummy, sizeof(fc_opr.dummy));
	if (ret)
	{
		snor_err("port_convert_dummy_info failed, ret: %d", ret);
		return -1;
	}

	ret = hal_flash_ctrl_setup_mem_map_opr(id, &fc_opr);
	if (ret < 0)
	{
		snor_err("hal_flash_ctrl_setup_mem_map_opr failed, ret: %d\n", ret);
		return ret;
	}

	return 0;
}

static int port_fc_add_mem_map(unsigned int id, const spi_nor_mem_map_t *mmap)
{
	int ret = 0;
	fc_mem_map_info_t fc_mmap;
	fc_mmap.mem_addr = mmap->mem_addr;
	fc_mmap.data_addr = mmap->data_addr;
	fc_mmap.len = mmap->len;

	ret = hal_flash_ctrl_add_mem_map(id, &fc_mmap);
	if (ret)
	{
		snor_err("hal_flash_ctrl_add_mem_map failed, ret: %d\n", ret);
		return ret;
	}

	return 0;
}

static int port_fc_del_mem_map(unsigned int id, const spi_nor_mem_map_t *mem_map)
{
	return -1;
}

#endif

static int port_convert_std_opr(const spi_nor_operation_t *snor_opr,
								fc_operation_t *fc_opr)
{
	int ret;

	fc_protocol_info_t *fc_proto = &fc_opr->proto;

	fc_proto->cmd_field_line_num = snor_opr->proto.cmd_field_line_num;
	fc_proto->addr_field_line_num = snor_opr->proto.addr_field_line_num;
	fc_proto->payload_field_line_num = snor_opr->proto.payload_field_line_num;

	fc_opr->cmd = snor_opr->cmd;

	if (snor_opr->proto.addr_field_line_num)
	{
		fc_proto->addr_field_byte_len = snor_opr->proto.addr_field_byte_len;
		fc_opr->addr = snor_opr->addr;
	}

	ret = port_convert_dummy_info(&snor_opr->proto, snor_opr->mode, sizeof(snor_opr->mode),
								  &fc_opr->proto, (uint8_t *)fc_opr->dummy, sizeof(fc_opr->dummy));
	if (ret)
	{
		snor_err("port_convert_dummy_info failed, ret: %d", ret);
		return -1;
	}

	if (fc_proto->payload_field_line_num)
	{
		if (snor_opr->payload_direction == SNOR_PYALOAD_DIRECTION_INPUT)
			fc_opr->is_tx_payload = 1;
		else
			fc_opr->is_tx_payload = 0;

		fc_opr->payload_buf = snor_opr->payload_buf;
		fc_opr->payload_len = snor_opr->payload_len;
	}


	return 0;
}

/* poll opr not support addr, mode and dummy field */
static int port_convert_poll_opr(const spi_nor_poll_operation_t *snor_opr,
								 fc_poll_operation_t *poll_opr)
{
	fc_operation_t *std_opr = &poll_opr->std_opr;
	fc_protocol_info_t *fc_proto = &std_opr->proto;

	fc_proto->cmd_field_line_num = snor_opr->proto.cmd_field_line_num;
	fc_proto->addr_field_line_num = snor_opr->proto.addr_field_line_num;
	fc_proto->payload_field_line_num = snor_opr->proto.payload_field_line_num;

	fc_proto->dummy_field_line_num = 0;
	fc_proto->addr_field_byte_len = 0;
	fc_proto->dummy_field_byte_len = 0;

	std_opr->is_tx_payload = 0;
	std_opr->cmd = snor_opr->cmd;
	std_opr->payload_len = snor_opr->data_len;

	poll_opr->bit_mask = snor_opr->bit_mask;
	poll_opr->match_value = snor_opr->match_value;
	poll_opr->poll_cnt = snor_opr->poll_cnt;

	return 0;
}

static int port_fc_exec_snor_operation(unsigned int id, const spi_nor_operation_t *snor_opr)
{
	int ret;
	fc_operation_t fc_opr;
	fc_poll_operation_t poll_opr;

	ret = port_convert_std_opr(snor_opr, &fc_opr);
	if (ret)
	{
		snor_err("port_convert_std_opr failed, ret: %d\n", ret);
		return ret;
	}

	fc_opr.poll_opr = NULL;
	if (snor_opr->poll_opr)
	{
		ret = port_convert_poll_opr(snor_opr->poll_opr, &poll_opr);
		if (ret)
		{
			snor_err("port_convert_poll_opr failed, ret: %d", ret);
			return -2;
		}
		fc_opr.poll_opr = &poll_opr;
	}
	ret = hal_flash_ctrl_exec_operation(id, &fc_opr);
	if (ret < 0)
	{
		snor_err("hal_flash_ctrl_exec_operation failed, ret: %d\n", ret);
		return ret;
	}

	return 0;

}

const spi_nor_controller_ops_t s_fc_controller_ops =
{
	.init = hal_flash_ctrl_init,
	.exec_opr = port_fc_exec_snor_operation,
#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	.set_output_clk_freq = hal_flash_ctrl_set_output_clk_freq,
	.setup_mem_map_proto = port_fc_setup_mem_map_proto,
	.add_mem_map = port_fc_add_mem_map,
	.del_mem_map = port_fc_del_mem_map,
#endif
};

const spi_nor_controller_t g_fc_controller =
{
	.id = DEFAULT_SPI_NOR_CONTROLLER_ID,
	.ops = &s_fc_controller_ops,
};

