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

#include "flash_controller.h"

#include <hal_atomic.h>
#include <hal_interrupt.h>

#include "platform_flashc.h"
#include "flash_controller_reg.h"

#ifdef CONFIG_USE_FLASH_CONTROLLER_ON_AMP
#include <hal_hwspinlock.h>
#define AMP_LOCK_TIMEOUT 5000
#endif

#define MAX_FC_IDLE_STATUS_CHECK_CNT 65536

#define MAX_FC_ADDR_FIELD_BYTE_LEN 4

//#define DUMP_FC_REG_AFTER_INIT

#ifdef FLASH_CONTROLLER_PM_SUPPORT
static struct pm_devops s_fc_pm_devops;
#endif

static uint8_t fc_read_reg8(const flash_controller_t *fc, uint32_t reg_offset)
{
	return fc_readb(fc->reg_base_addr + reg_offset);
}

static uint32_t fc_read_reg(const flash_controller_t *fc, uint32_t reg_offset)
{
	return fc_readl(fc->reg_base_addr + reg_offset);
}

static void fc_write_reg8(const flash_controller_t *fc, uint32_t reg_offset, uint8_t reg_data)
{
	fc_writeb(reg_data, fc->reg_base_addr + reg_offset);
}

static void fc_write_reg(const flash_controller_t *fc, uint32_t reg_offset, uint32_t reg_data)
{
	fc_writel(reg_data, fc->reg_base_addr + reg_offset);
}

static void fc_mod_reg(const flash_controller_t *fc, uint32_t reg_offset, uint32_t filed_value, uint32_t filed_bit_mask)
{
	uint32_t reg_addr = fc->reg_base_addr + reg_offset, reg_data;
	reg_data = fc_readl(reg_addr);
	reg_data &= ~filed_bit_mask;
	reg_data |= (filed_value & filed_bit_mask);
	fc_writel(reg_data, reg_addr);
}

__attribute__((unused)) void dump_fc_reg(const flash_controller_t *fc)
{
	fc_info("---------------------------------------------------");
	fc_info("bus_clk_gating(0x%x bit22~23):0x%08x",0x4003C000+0x04, *((volatile u32 *)(0x4003C000+0x04)));
	fc_info("module_reset_ctl(0x%x bit22~23):0x%08x",0x4003C000+0x0C, *((volatile u32 *)(0x4003C000+0x0C)));
	fc_info("flash_spi_clk_ctl(0x%x):0x%08x",0x4003C000+0x54, *((volatile u32 *)(0x4003C000+0x54)));

	fc_info("sys_clk_ctl(0x%x):0x%08x",0x4004C400+0xE0, *((volatile u32 *)(0x4004C400+0xE0)));
	fc_info("dpll1_out_cfg(0x%x):0x%08x",0x4004C400+0xA4, *((volatile u32 *)(0x4004C400+0xA4)));

	fc_info("---------------------------------------------------");

	uint32_t base_addr, reg_addr, reg_data;

	base_addr = fc->reg_base_addr;
	reg_addr = base_addr + MEM_CTRL_REG_OFF;
	reg_data = fc_readl(reg_addr);
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, reg_data);

	reg_addr = base_addr + QSPI_CONTROLLER_CFG_REG_OFF;
	reg_data = fc_readl(reg_addr);
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, reg_data);

	reg_addr = base_addr + MEM_MAP_MODE_CACHE_CFG_REG_OFF;
	reg_data = fc_readl(reg_addr);
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, reg_data);


	reg_addr = base_addr + MEM_MAP_MODE_READ_PROTOCOL_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + MEM_MAP_MODE_WRITE_PROTOCOL_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + MEM_MAP_MODE_DUMMY_DATA_HIGH_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + MEM_MAP_MODE_DUMMY_DATA_LOW_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INDIRECT_MODE_PROTOCOL_CFG_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INDIRECT_MODE_FLASH_DATA_ADDR_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INDIRECT_MODE_DUMMY_DATA_HIGH_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INDIRECT_MODE_DUMMY_DATA_LOW_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INDIRECT_MODE_IO_LATENCY_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INDIRECT_MODE_WRITE_PAYLOAD_LEN_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INDIRECT_MODE_READ_PAYLOAD_LEN_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INDIRECT_MODE_START_SEND_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + FIFO_LEVEL_CFG_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + FIFO_STATUS_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INT_CTRL_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	reg_addr = base_addr + INT_STATUS_REG_OFF;
	fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));

	uint32_t interval = MEM_MAP_CFG_REG_ADDR_INTERVAL;
	reg_addr = base_addr + MEM_MAP0_START_ADDR_REG_OFF;
	for (int i = 0; i < FLASH_CONTROLLER_MEM_MAP_NUM; i++)
	{
		fc_info("addr: 0x%08x, data: 0x%08x", reg_addr, fc_readl(reg_addr));
		fc_info("addr: 0x%08x, data: 0x%08x", reg_addr + 4, fc_readl(reg_addr + 4));
		fc_info("addr: 0x%08x, data: 0x%08x", reg_addr + 8, fc_readl(reg_addr + 8));
		reg_addr += interval;
	}
}

#ifdef DEBUG_FC_OPERATION
void dump_std_opr(const fc_operation_t *opr, int need_brief)
{
	const fc_protocol_info_t *proto = &opr->proto;

	if (need_brief)
		fc_info("--------Flash Controller standard operation info--------");
	fc_info("cmd line: %u", opr->proto.cmd_field_line_num);
	fc_info("addr line: %u", opr->proto.addr_field_line_num);
	fc_info("dummy line: %u", opr->proto.dummy_field_line_num);
	fc_info("payload line: %u", opr->proto.payload_field_line_num);

	if (proto->cmd_field_line_num)
		fc_info("cmd: 0x%02x", opr->cmd);

	if (proto->addr_field_line_num)
	{
		fc_info("addr byte len: %u", proto->addr_field_byte_len);
		fc_info("addr: 0x%08x", opr->addr);
	}

	if (proto->dummy_field_line_num)
	{
		fc_info("dummy byte len: %u", proto->dummy_field_byte_len);
		fc_info("dummy[0]: 0x%08x", *((uint32_t *)&opr->dummy[0]));
		fc_info("dummy[1]: 0x%08x", *((uint32_t *)&opr->dummy[4]));
	}

	if (proto->payload_field_line_num)
	{
		fc_info("is_tx_payload: %u", opr->is_tx_payload);
		fc_info("payload_buf: 0x%p", opr->payload_buf);
		fc_info("payload_len: 0x%08x", opr->payload_len);
	}
}

void dump_poll_opr(const fc_poll_operation_t *opr, int need_brief)
{
	if (need_brief)
		fc_info("--------Flash Controller poll operation info--------");

	fc_info("bit mask: %u", opr->bit_mask);
	fc_info("match value: %u", opr->match_value);
	fc_info("poll cnt: %u", opr->poll_cnt);

	fc_info("--------std operation info--------");
	dump_std_opr(&opr->std_opr, 0);
}
#endif

static inline int fc_lock_init(flash_controller_t *fc)
{
	int ret = hal_mutex_init(&fc->mutex);
	if (ret)
	{
		fc_err("init mutex for fc failed, ret: %d", ret);
		return -FC_RET_CODE_MUTEX_INIT_FAILED;
	}

	return 0;
}

static int fc_lock(flash_controller_t *fc)
{
	int ret = hal_mutex_lock(&fc->mutex);
	if (ret)
	{
		fc_err("lock fc object failed, ret: %d", ret);
		return -FC_RET_CODE_MUTEX_LOCK_FAILED;
	}

	return 0;
}

static int fc_unlock(flash_controller_t *fc)
{
	int ret = hal_mutex_unlock(&fc->mutex);
	if (ret)
	{
		fc_err("unlock fc object failed, ret: %d", ret);
		return -FC_RET_CODE_MUTEX_UNLOCK_FAILED;
	}

	return 0;
}

static int is_fc_busy(const flash_controller_t *fc)
{
	return !!(fc_read_reg(fc, INDIRECT_MODE_START_SEND_REG_OFF) & 0x1);
}

static int is_mem_map_mode_enable(const flash_controller_t *fc)
{
	return (fc_read_reg(fc, MEM_CTRL_REG_OFF) & MEM_MAP_MODE_ENABLE_MASK);
}

static void fc_enable_mem_map_mode(flash_controller_t *fc)
{
	fc_mod_reg(fc, MEM_CTRL_REG_OFF,
			   1 << MEM_MAP_MODE_ENABLE_SHIFT |
			   0 << CONTINUOUS_READ_MODE_ENABLE_SHIFT |
			   0 << WARP_AROUND_MODE_ENABLE_SHIFT |
			   1 << BUS_ARBITRATION_ENABLE_SHIFT,
			   MEM_MAP_MODE_ENABLE_MASK |
			   CONTINUOUS_READ_MODE_ENABLE_MASK |
			   WARP_AROUNDMODE_ENABLE_MASK|
			   BUS_ARBITRATION_ENABLE_MASK);
}

static void fc_disable_mem_map_mode(flash_controller_t *fc)
{
	fc_mod_reg(fc, MEM_CTRL_REG_OFF,
			   0 << MEM_MAP_MODE_ENABLE_SHIFT |
			   1 << BUS_ARBITRATION_ENABLE_SHIFT,
			   MEM_MAP_MODE_ENABLE_MASK |
			   BUS_ARBITRATION_ENABLE_MASK);
}


static int wait_fc_idle(const flash_controller_t *fc, int max_check_cnt)
{
	int check_cnt = 0;
	while(1)
	{
		check_cnt++;
		if (!is_fc_busy(fc))
			return 0;

		if (check_cnt >= max_check_cnt)
		{
			fc_err("wait fc idle timeout, check_cnt: %d", check_cnt);
			return -FC_RET_CODE_WAIT_FC_IDLE_TIMEOUT;
		}
	}

	return 0;
}

static uint32_t fc_get_flash_addr_len(const flash_controller_t *fc)
{
	uint32_t reg_data = fc_read_reg(fc, MEM_CTRL_REG_OFF);
	reg_data &= FLASH_ADDR_SIZE_MODE_MASK;
	reg_data >>= FLASH_ADDR_SIZE_MODE_SHIFT;
	reg_data += 1;
	return reg_data;
}

static void fc_set_flash_addr_len(const flash_controller_t *fc, uint32_t addr_len)
{
	fc_mod_reg(fc, MEM_CTRL_REG_OFF,
			   (addr_len - 1) << FLASH_ADDR_SIZE_MODE_SHIFT,
			   FLASH_ADDR_SIZE_MODE_MASK);
}

static void fc_set_flash_data_addr(const flash_controller_t *fc, uint32_t addr, uint32_t addr_byte_len)
{
	uint32_t mask = 0xFFFFFFFF;
	mask >>= (4 - addr_byte_len) * 8;
	fc_write_reg(fc, INDIRECT_MODE_FLASH_DATA_ADDR_REG_OFF, addr & mask);
}

static void fc_set_dummy_data(const flash_controller_t *fc, const uint8_t *buf, uint32_t len)
{
	uint32_t mask;
	uint32_t *dummy_data;

	if (len == 0)
		return;

	if (len <= 4)
	{
		mask = 0xFFFFFFFF;
		mask >>= (4 - len) * 8;
		dummy_data = (uint32_t *)buf;
		fc_write_reg(fc, INDIRECT_MODE_DUMMY_DATA_HIGH_REG_OFF, *dummy_data & mask);
	}

	if (len > 4)
	{
		len -= 4;
		mask = 0xFFFFFFFF;
		mask >>= (4 - len) * 8;
		dummy_data = (uint32_t *)buf;
		dummy_data ++;
		fc_write_reg(fc, INDIRECT_MODE_DUMMY_DATA_LOW_REG_OFF, *dummy_data & mask);
	}

}

static uint32_t line_num_to_field_value(uint32_t line_num)
{
	uint32_t filed_value = 0;
	switch (line_num)
	{
		case 0:
		case 1:
		case 2:
			filed_value = line_num;
			break;
		case 4:
			filed_value = 3;
			break;
		case 8:
			filed_value = 4;
			break;
		default:
			break;
	}

	return filed_value;
}

static int is_line_num_valid(uint32_t line_num)
{
	if ((line_num == 0) ||
			(line_num == 1) ||
			(line_num == 2) ||
			(line_num == 4) ||
			(line_num == 8))
	{
		return 1;
	}

	return 0;
}

static int fc_check_field_line_num(const fc_protocol_info_t *proto, int is_mem_map_mode)
{
	uint32_t line_num;
	const char *mode_str = "indirect";
	if (is_mem_map_mode)
	{
		mode_str = "mem map";
	}

	line_num = proto->cmd_field_line_num;
	if ((!is_line_num_valid(line_num))
			|| (is_mem_map_mode && (line_num == 0)))
	{
		fc_err("invalid cmd field line num: %u, mode: %s", line_num, mode_str);
		return -FC_RET_CODE_INVALID_CMD_LINE;
	}
	line_num = proto->addr_field_line_num;
	if (!is_line_num_valid(line_num)
			|| (is_mem_map_mode && (line_num == 0)))
	{
		fc_err("invalid addr field line num: %u, mode: %s", line_num, mode_str);
		return -FC_RET_CODE_INVALID_ADDR_LINE;
	}
	line_num = proto->dummy_field_line_num;
	if (!is_line_num_valid(line_num))
	{
		fc_err("invalid optional field line num: %u, mode: %s", line_num, mode_str);
		return -FC_RET_CODE_INVALID_DUMMY_LINE;
	}
	line_num = proto->payload_field_line_num;
	if (!is_line_num_valid(line_num)
			|| (is_mem_map_mode && (line_num == 0)))
	{
		fc_err("invalid payload field line num: %u, mode: %s", line_num, mode_str);
		return -FC_RET_CODE_INVALID_PAYLOAD_LINE;
	}

	if (!is_mem_map_mode)
	{
		if ((!proto->cmd_field_line_num) &&
				(!proto->addr_field_line_num) &&
				(!proto->dummy_field_line_num) &&
				(!proto->payload_field_line_num))
		{
			fc_err("the line num of all protocol field is zero!");
			return -FC_RET_CODE_INVALID_ALL_LINE;
		}
	}

	return 0;
}

static int fc_check_std_opr(const fc_operation_t *opr)
{
	int ret = 0;
	uint8_t byte_len;
	const fc_protocol_info_t *proto;

	proto = &opr->proto;

	ret = fc_check_field_line_num(proto, 0);
	if (ret)
	{
		return ret;
	}

	if (proto->addr_field_line_num != 0)
	{
		byte_len = proto->addr_field_byte_len;
		if ((byte_len == 0) || (byte_len > MAX_FC_ADDR_FIELD_BYTE_LEN))
		{
			fc_err("invalid addr field byte len: %u", byte_len);
			return -FC_RET_CODE_INVALID_ADDR_LEN;
		}
	}

	if (proto->dummy_field_line_num != 0)
	{
		byte_len = proto->dummy_field_byte_len;
		if ((byte_len == 0) || (byte_len > MAX_FC_DUMMY_FIELD_BYTE_LEN))
		{
			fc_err("invalid dummy field byte len: %u", byte_len);
			return -FC_RET_CODE_INVALID_DUMMY_LEN;
		}
	}

	if (proto->payload_field_line_num != 0)
	{
		if (!opr->payload_buf)
		{

			fc_err("payload buf is NULL when payload is present, payload_len: %u", opr->payload_len);
			return -FC_RET_CODE_INVALID_PAYLOAD_BUF;
		}

		if (!opr->payload_len)
		{
			fc_err("payload len is 0 when payload is present, payload_buf: 0x%p", opr->payload_buf);
			return -FC_RET_CODE_INVALID_PAYLOAD_LEN;
		}
	}

	return 0;
}

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
static int fc_check_mem_map_opr(const fc_mem_map_operation_t *opr)
{
	int ret = 0;
	uint8_t byte_len;
	const fc_protocol_info_t *proto;

	proto = &opr->proto;

	ret = fc_check_field_line_num(proto, 1);
	if (ret)
	{
		return ret;
	}

	byte_len = proto->addr_field_byte_len;
	if ((byte_len == 0) || (byte_len > MAX_FC_ADDR_FIELD_BYTE_LEN))
	{
		fc_err("invalid addr field byte len: %u", byte_len);
		return -FC_RET_CODE_INVALID_ADDR_LEN;
	}

	if (proto->dummy_field_line_num != 0)
	{
		byte_len = proto->dummy_field_byte_len;
		if ((byte_len == 0) || (byte_len > MAX_FC_DUMMY_FIELD_BYTE_LEN))
		{
			fc_err("invalid dummy field byte len: %u", byte_len);
			return -FC_RET_CODE_INVALID_DUMMY_LEN;
		}
	}

	return 0;
}
#endif

static void fc_setup_std_operation(const flash_controller_t *fc, const fc_operation_t *opr)
{
	uint32_t cmd = 0, dummy_bit_num = 0, cmd_line, addr_line, dummy_line, payload_line;
	const fc_protocol_info_t *proto = &opr->proto;

	cmd_line = proto->cmd_field_line_num;
	if (cmd_line)
	{
		cmd = opr->cmd;
		cmd = (cmd << INDIRECT_MODE_CMD_DATA_SHIFT) & INDIRECT_MODE_CMD_DATA_MASK;
		cmd_line = (line_num_to_field_value(cmd_line) << INDIRECT_MODE_CMD_LINES_SHIFT) &
				   INDIRECT_MODE_CMD_LINES_BIT_MASK;
	}

	addr_line = proto->addr_field_line_num;
	if (addr_line)
	{
		/* not set addr len here. because this filed is common used by indirect mode and mem map mode */
		//fc_set_flash_addr_len(fc, proto->addr_field_byte_len);
		fc_set_flash_data_addr(fc, opr->addr, proto->addr_field_byte_len);
		addr_line = (line_num_to_field_value(addr_line) << INDIRECT_MODE_ADDR_LINES_SHIFT) &
					INDIRECT_MODE_ADDR_LINES_BIT_MASK;
	}

	dummy_line = proto->dummy_field_line_num;
	if (dummy_line)
	{
		dummy_bit_num = proto->dummy_field_byte_len * 8;
		dummy_bit_num = (dummy_bit_num << INDIRECT_MODE_DUMMY_FIELD_LEN_SHIFT) &
						INDIRECT_MODE_DUMMY_FIELD_LEN_BIT_MASK;

		fc_set_dummy_data(fc, opr->dummy, proto->dummy_field_byte_len);
		dummy_line = (line_num_to_field_value(dummy_line) << INDIRECT_MODE_DUMMY_FIELD_LINES_SHIFT) &
					 INDIRECT_MODE_DUMMY_FIELD_LINES_BIT_MASK;
	}

	payload_line = proto->payload_field_line_num;
	if (payload_line)
	{
		uint32_t reg_offset = INDIRECT_MODE_READ_PAYLOAD_LEN_REG_OFF;
		if (opr->is_tx_payload)
			reg_offset = INDIRECT_MODE_WRITE_PAYLOAD_LEN_REG_OFF;

		fc_write_reg(fc, reg_offset, opr->payload_len);
		payload_line = (line_num_to_field_value(payload_line) << INDIRECT_MODE_PAYLOAD_LINES_SHIFT) &
					   INDIRECT_MODE_PAYLOAD_LINES_BIT_MASK;
	}

	fc_dbg("cmd: 0x%08x, dummy_bit_num: 0x%08x, "
		   "cmd_line: 0x%08x, addr_line: 0x%08x, dummy_line: 0x%08x, payload_line: 0x%08x,",
		   cmd, dummy_bit_num,
		   cmd_line, addr_line, dummy_line, payload_line);

	fc_mod_reg(fc, INDIRECT_MODE_PROTOCOL_CFG_REG_OFF,
			   cmd | dummy_bit_num | cmd_line | addr_line | dummy_line | payload_line,
			   INDIRECT_MODE_CMD_DATA_MASK |
			   INDIRECT_MODE_DUMMY_FIELD_LEN_BIT_MASK |
			   INDIRECT_MODE_CMD_LINES_BIT_MASK |
			   INDIRECT_MODE_ADDR_LINES_BIT_MASK |
			   INDIRECT_MODE_DUMMY_FIELD_LINES_BIT_MASK |
			   INDIRECT_MODE_PAYLOAD_LINES_BIT_MASK);
}

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
static void fc_setup_mem_map_operation(const flash_controller_t *fc, const fc_mem_map_operation_t *opr)
{
	uint32_t cmd = 0, dummy_bit_num = 0, cmd_line, addr_line, dummy_line, payload_line;
	const fc_protocol_info_t *proto = &opr->proto;

	cmd_line = proto->cmd_field_line_num;
	if (cmd_line)
	{
		cmd = opr->cmd;
		cmd = (cmd << INDIRECT_MODE_CMD_DATA_SHIFT) & INDIRECT_MODE_CMD_DATA_MASK;
		cmd_line = (line_num_to_field_value(cmd_line) << INDIRECT_MODE_CMD_LINES_SHIFT) &
				   INDIRECT_MODE_CMD_LINES_BIT_MASK;
	}

	addr_line = proto->addr_field_line_num;
	if (addr_line)
	{
		/* not set addr len here. because this filed is common used by indirect mode and mem map mode */
		fc_set_flash_addr_len(fc, proto->addr_field_byte_len);
		addr_line = (line_num_to_field_value(addr_line) << INDIRECT_MODE_ADDR_LINES_SHIFT) &
					INDIRECT_MODE_ADDR_LINES_BIT_MASK;
	}

	dummy_line = proto->dummy_field_line_num;
	if (dummy_line)
	{
		dummy_bit_num = proto->dummy_field_byte_len * 8;

		fc_set_dummy_data(fc, opr->dummy, proto->dummy_field_byte_len);

		dummy_bit_num = (dummy_bit_num << INDIRECT_MODE_DUMMY_FIELD_LEN_SHIFT) &
						INDIRECT_MODE_DUMMY_FIELD_LEN_BIT_MASK;
		dummy_line = (line_num_to_field_value(dummy_line) << INDIRECT_MODE_DUMMY_FIELD_LINES_SHIFT) &
					 INDIRECT_MODE_DUMMY_FIELD_LINES_BIT_MASK;
	}

	payload_line = proto->payload_field_line_num;
	if (payload_line)
	{
		payload_line = (line_num_to_field_value(payload_line) << MEM_MAP_MODE_PAYLOAD_LINES_SHIFT) &
					   MEM_MAP_MODE_PAYLOAD_LINES_BIT_MASK;
	}

	fc_dbg("cmd: 0x%08x, dummy_bit_num: 0x%08x, "
		   "cmd_line: 0x%08x, addr_line: 0x%08x, dummy_line: 0x%08x, payload_line: 0x%08x,",
		   cmd, dummy_bit_num,
		   cmd_line, addr_line, dummy_line, payload_line);


	fc_mod_reg(fc, MEM_MAP_MODE_READ_PROTOCOL_REG_OFF,
			   cmd | dummy_bit_num | cmd_line | addr_line | dummy_line | payload_line,
			   MEM_MAP_MODE_CMD_DATA_MASK |
			   MEM_MAP_MODE_DUMMY_FIELD_LEN_BIT_MASK |
			   MEM_MAP_MODE_CMD_LINES_BIT_MASK |
			   MEM_MAP_MODE_ADDR_LINES_BIT_MASK |
			   MEM_MAP_MODE_DUMMY_FIELD_LINES_BIT_MASK |
			   MEM_MAP_MODE_PAYLOAD_LINES_BIT_MASK);
}
#endif

static __attribute__((unused)) int check_cache_status(const flash_controller_t *fc)
{
	int ret = 0;
	uint32_t reg_data, fifo_check_cnt = 0, max_check_cnt = 10000000;

	while (1)
	{
		fifo_check_cnt++;
		reg_data = fc_read_reg(fc, MEM_CTRL_REG_OFF);
		reg_data = (reg_data & INVALIDATE_CACHE_ENABLE_MASK) >> INVALIDATE_CACHE_ENABLE_SHIFT;
		if (reg_data == 0)
			break;

		if (fifo_check_cnt >= max_check_cnt)
		{
			ret = -1;
			break;
		}
	}

	return ret;
}

static int check_tx_rx_fifo_status(const flash_controller_t *fc)
{
	uint32_t reg_data, fifo_check_cnt = 0, max_check_cnt = 10000000;

	while (1)
	{
		fifo_check_cnt++;
		reg_data = fc_read_reg(fc, MEM_CTRL_REG_OFF);
		reg_data = (reg_data & (RESET_TX_FIFO_MASK | RESET_RX_FIFO_MASK));
		if (reg_data == 0)
			break;

		if (fifo_check_cnt >= max_check_cnt)
			return -1;
	}

	fc_dbg("tx rx FIFO, element num: %u, fifo_check_cnt: %u", reg_data, fifo_check_cnt);
	return 0;
}

int wait_read_fifo_empty(const flash_controller_t *fc)
{
	uint32_t reg_data, fifo_check_cnt = 0, max_check_cnt = 10000000;

	while (1)
	{
		fifo_check_cnt++;
		reg_data = fc_read_reg(fc, FIFO_STATUS_REG_OFF);
		reg_data = (reg_data & READ_FIFO_DATA_CNT_MASK) >> READ_FIFO_DATA_CNT_SHIFT;
		if (reg_data == 0)
		{
			if (fifo_check_cnt != 1)
				fc_info("fifo_check_cnt: %u", fifo_check_cnt);
			break;
		}

		if (fifo_check_cnt >= max_check_cnt)
		{
			fc_err("fifo_check_cnt: %u", fifo_check_cnt);
			return -1;
		}
	}

	fc_dbg("Read FIFO, element num: %u, fifo_check_cnt: %u", reg_data, fifo_check_cnt);
	return 0;
}

static void fc_indirect_mode_prepare(flash_controller_t *fc)
{
#ifdef CONFIG_DISABLE_INTERRUPT_WHEN_USING_INDIRECT_MODE
	fc->irq_flags = hal_spin_lock_irqsave(&fc->spinlock);
#else
	/* if irq disable, do not suspend scheduler in case of system error */
	if (!hal_interrupt_is_disable())
		vTaskSuspendAll();
#endif
	wait_read_fifo_empty(fc);
	fc_disable_mem_map_mode(fc);

	int ret;
	fc_mod_reg(fc, MEM_CTRL_REG_OFF,
			   (1 << RESET_TX_FIFO_SHIFT) |
			   (1 << RESET_RX_FIFO_SHIFT),
			   RESET_TX_FIFO_MASK |
			   RESET_RX_FIFO_MASK);

	ret =check_tx_rx_fifo_status(fc);
	if (ret)
		fc_err("check_tx_rx_fifo_status failed");
}

static void fc_indirect_mode_unprepare(flash_controller_t *fc)
{
	int ret = 0;

	fc_mod_reg(fc, MEM_CTRL_REG_OFF,
			   (1 << RESET_TX_FIFO_SHIFT) |
			   (1 << RESET_RX_FIFO_SHIFT),
			   RESET_TX_FIFO_MASK |
			   RESET_RX_FIFO_MASK);

	ret =check_tx_rx_fifo_status(fc);
	if (ret)
		fc_err("check_tx_rx_fifo_status failed");

	fc_mod_reg(fc, MEM_CTRL_REG_OFF,
			   1 << INVALIDATE_CACHE_ENABLE_SHIFT,
			   INVALIDATE_CACHE_ENABLE_MASK);

	ret =check_cache_status(fc);
	if (ret)
		fc_err("check_cache_status failed");

	fc_enable_mem_map_mode(fc);
#ifdef CONFIG_DISABLE_INTERRUPT_WHEN_USING_INDIRECT_MODE
	hal_spin_unlock_irqrestore(&fc->spinlock, fc->irq_flags);
#else
	/* if irq disable, do not suspend scheduler in case of system error */
	if (!hal_interrupt_is_disable())
		xTaskResumeAll();

#endif
}


#ifdef CONFIG_USE_FLASH_CONTROLLER_ON_AMP
static int fc_amp_lock(const flash_controller_t *fc)
{
	int lock_ret = hal_hwspin_lock_timeout(fc->amp_lock_id, AMP_LOCK_TIMEOUT);
	if (lock_ret != HWSPINLOCK_OK)
		return -FC_RET_CODE_AMP_LOCK_TIMEOUT;

	return 0;
}

static void fc_amp_unlock(const flash_controller_t *fc)
{
	hal_hwspin_unlock(fc->amp_lock_id);
}
#endif

static int check_write_fifo_status(const flash_controller_t *fc)
{
	uint32_t reg_data, fifo_check_cnt = 0, max_check_cnt = 10000000;

	while (1)
	{
		fifo_check_cnt++;
		reg_data = fc_read_reg(fc, FIFO_STATUS_REG_OFF);
		reg_data = (reg_data & WRITE_FIFO_DATA_CNT_MASK) >> WRITE_FIFO_DATA_CNT_SHIFT;
		if (reg_data <= 100)
			break;

		if (fifo_check_cnt >= max_check_cnt)
			return -FC_RET_CODE_CHECK_WRITE_FIFO_TIMEOUT;
	}

	return 0;
}

static int check_read_fifo_status(const flash_controller_t *fc)
{
	uint32_t reg_data, fifo_check_cnt = 0, max_check_cnt = 10000000;

	while (1)
	{
		fifo_check_cnt++;
		reg_data = fc_read_reg(fc, FIFO_STATUS_REG_OFF);
		reg_data = (reg_data & READ_FIFO_DATA_CNT_MASK) >> READ_FIFO_DATA_CNT_SHIFT;
		if (reg_data != 0)
			break;

		if (fifo_check_cnt >= max_check_cnt)
			return -FC_RET_CODE_CHECK_READ_FIFO_TIMEOUT;
	}

	return 0;
}


#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
static __attribute__((unused)) int fc_check_mem_map(flash_controller_t *fc, const fc_mem_map_info_t *mmap)
{
	return 0;
}

static int fc_find_empty_mem_map(flash_controller_t *fc, int *index)
{
	int i;
	uint32_t reg_offset = MEM_MAP0_OFFSET_REG_OFF, interval = MEM_MAP_CFG_REG_ADDR_INTERVAL;
	for (i = 0; i < FLASH_CONTROLLER_MEM_MAP_NUM; i++)
	{
		if (!(fc_read_reg(fc, reg_offset) & MEM_MAP_ENABLE_MASK))
		{

			*index = i;
			return 0;
		}
		reg_offset += interval;
	}

	return -1;
}

static __attribute__((unused)) int fc_enable_module_clk(const flash_controller_t *fc)
{
	hal_clk_status_t ret;

	hal_clk_type_t clk_type = fc->input_clk_type;
	hal_clk_id_t clk_id = fc->input_clk_id;

	hal_clk_t mclk = hal_clock_get(clk_type, clk_id);
	if (!mclk)
	{
		fc_err("get clk handle failed, type: %d, id: %d", clk_type, clk_id);
		return -FC_RET_CODE_GET_CLK_FAILED;
	}

	ret = hal_clock_enable(mclk);
	if (ret)
	{
		fc_err("hal_clock_enable failed, ret: %d", ret);
		hal_clock_put(mclk);
		return -FC_RET_CODE_ENABLE_CLK_FAILED;
	}

	fc_dbg("Enable module clock");

	hal_clock_put(mclk);
	return 0;
}

static __attribute__((unused)) int fc_disable_module_clk(const flash_controller_t *fc)
{
	hal_clk_status_t ret;

	hal_clk_type_t clk_type = fc->input_clk_type;
	hal_clk_id_t clk_id = fc->input_clk_id;

	hal_clk_t mclk = hal_clock_get(clk_type, clk_id);
	if (!mclk)
	{
		fc_err("get clk handle failed, type: %d, id: %d", clk_type, clk_id);
		return -FC_RET_CODE_GET_CLK_FAILED;
	}

	ret = hal_clock_disable(mclk);
	if (ret)
	{
		fc_err("hal_clock_disable failed, ret: %d", ret);
		hal_clock_put(mclk);
		return -FC_RET_CODE_DISABLE_CLK_FAILED;
	}

	fc_dbg("Disable module clock");

	hal_clock_put(mclk);
	return 0;
}

static int fc_reset_module(const flash_controller_t *fc)
{
	return plat_fc_reset_module(fc);
}

static inline int fc_init_module_clk(flash_controller_t *fc, uint32_t clk_freq)
{
	return plat_fc_init_module_clk(fc, clk_freq, &fc->input_clk_freq);
}

static inline int fc_init_pinmux(const flash_controller_t *fc)
{
	return plat_fc_init_pinmux(fc);
}

static void fc_init_cache_cfg(const flash_controller_t *fc)
{
	/* set read cache line size to 32Bytes */
	fc_mod_reg(fc, MEM_MAP_MODE_CACHE_CFG_REG_OFF, 2 << READ_CACHE_LINE_SIZE_SHIFT,
			   READ_CACHE_LINE_SIZE_BIT_MASK);
}

static void fc_init_spi_timing(const flash_controller_t *fc)
{
	uint32_t clk_polarity;
	uint32_t clk_phase;
	uint32_t cs_polarity;
	uint32_t rx_lsb_first;

	fc_spi_timing_cfg_t timing = fc->spi_timing_cfg;

	clk_polarity = timing.clk_polarity;
	clk_polarity = (clk_polarity << SPI_CLK_POLARITY_SHIFT) & SPI_CLK_POLARITY_BIT_MASK;

	clk_phase = timing.clk_phase;
	clk_phase = (clk_phase << SPI_CLK_PHASE_SHIFT) & SPI_CLK_PHASE_BIT_MASK;

	if (timing.cs_active_low)
		cs_polarity = 0;
	else
		cs_polarity = 1;

	cs_polarity = (cs_polarity << SPI_CS_POLARITY_SHIFT) & SPI_CS_POLARITY_BIT_MASK;;

	rx_lsb_first = timing.rx_lsb_first;
	rx_lsb_first = (rx_lsb_first << SPI_RX_ENDIAN_SHIFT) & SPI_RX_ENDIAN_BIT_MASK;

	fc_mod_reg(fc, QSPI_CONTROLLER_CFG_REG_OFF,
			   clk_polarity | clk_phase | cs_polarity | rx_lsb_first,
			   SPI_CLK_POLARITY_BIT_MASK |
			   SPI_CLK_PHASE_BIT_MASK |
			   SPI_CS_POLARITY_BIT_MASK |
			   SPI_RX_ENDIAN_BIT_MASK);

	/* we must not set the IO output high-impedance when idle, otherwise the first
	 * switch from high-impedance state to high will be recognized as low level
	 * when we run with high frequency clock. eg: If IO1 output high-impedance
	 * when idle and flash controller is setup to 96MHz QSPI mode, the bit 21 will
	 * be zero when we read the address 0x602000(actual we will read data at 0x402000)
	 */
	fc_mod_reg(fc, MEM_CTRL_REG_OFF,
			   (1 << IO1_OUTPUT_WHEN_IDLE_SHIFT) |
			   (1 << IO2_OUTPUT_WHEN_IDLE_SHIFT) |
			   (1 << IO3_OUTPUT_WHEN_IDLE_SHIFT),
			   IO1_OUTPUT_WHEN_IDLE_BIT_MASK |
			   IO2_OUTPUT_WHEN_IDLE_BIT_MASK |
			   IO3_OUTPUT_WHEN_IDLE_BIT_MASK);
}

static void fc_set_sample_delay(const flash_controller_t *fc)
{
	uint32_t rx_sample_delay;
	uint32_t clk_freq;

	clk_freq = fc->input_clk_freq;
	if (clk_freq <= CONFIG_FC_SAMPLE_DELAY_0_MAX_CLK_FREQ)
		rx_sample_delay = 0;
	else if (clk_freq <= CONFIG_FC_SAMPLE_DELAY_1_MAX_CLK_FREQ)
		rx_sample_delay = 1;
	else if (clk_freq <= CONFIG_FC_SAMPLE_DELAY_2_MAX_CLK_FREQ)
		rx_sample_delay = 2;
	else
		rx_sample_delay = 3;

	fc_info("RX sample delay: %u", rx_sample_delay);
	rx_sample_delay = (rx_sample_delay << RX_SAMPLE_DELAY_SHIFT) & RX_SAMPLE_DELAY_BIT_MASK;

	fc_mod_reg(fc, QSPI_CONTROLLER_CFG_REG_OFF, rx_sample_delay, RX_SAMPLE_DELAY_BIT_MASK);
}

#endif

static int fc_exec_std_operation(flash_controller_t *fc, const fc_operation_t *opr_info)
{
	int ret = 0;

	uint32_t payload_len;
	uint8_t *buf;

	if (opr_info->proto.addr_field_line_num)
	{
		fc_set_flash_addr_len(fc, opr_info->proto.addr_field_byte_len);
	}

	fc_mod_reg(fc, INDIRECT_MODE_START_SEND_REG_OFF, START_SEND_MASK, START_SEND_MASK);

	if (opr_info->proto.payload_field_line_num)
	{
		payload_len = opr_info->payload_len;
		buf = opr_info->payload_buf;
		if (opr_info->is_tx_payload)
		{
			while (payload_len--)
			{
				ret = check_write_fifo_status(fc);
				if (ret)
				{
					return ret;
				}
				fc_write_reg8(fc, INDIRECT_MODE_WRITE_DATA_REG_OFF, *buf);
				buf++;
			}
		}
		else
		{
			while (payload_len--)
			{
				ret = check_read_fifo_status(fc);
				if (ret)
				{
					return ret;
				}

				*buf = fc_read_reg8(fc, INDIRECT_MODE_READ_DATA_REG_OFF);
				buf++;
			}
		}
	}

	//wait operation complete
	ret = wait_fc_idle(fc, MAX_FC_IDLE_STATUS_CHECK_CNT);
	if (ret)
	{
		return ret;
	}

	return 0;
}

static int fc_exec_poll_operation(flash_controller_t *fc, fc_poll_operation_t *poll_opr)
{
	int ret = 0;
	uint32_t data_buf = 0, bit_mask, poll_cnt = 0;

	bit_mask = poll_opr->bit_mask;
	poll_opr->std_opr.payload_buf = (uint8_t *)&data_buf;

#ifdef DEBUG_FC_OPERATION
	fc_info("--------poll operation info--------, data_buf: 0x%p", &data_buf);
	dump_poll_opr(poll_opr, 0);
#endif

	fc_setup_std_operation(fc, &poll_opr->std_opr);

	while (1)
	{
		poll_cnt++;

		/* we must set operation even though the operation info is same with last,
		 * otherwise hardware will work abnormally.
		 */
		fc_setup_std_operation(fc, &poll_opr->std_opr);
		ret = fc_exec_std_operation(fc, &poll_opr->std_opr);
		if (ret)
		{
			fc_err("fc_exec_std_operation failed, ret: %d, poll_cnt: %u", ret, poll_cnt);
#ifdef DEBUG_FC_OPERATION
			dump_fc_reg(fc);
			fc_info("--------poll operation info--------, data_buf: 0x%p", &data_buf);
			dump_poll_opr(poll_opr, 0);
#endif
			return ret;
		}

		if ((data_buf & bit_mask) == poll_opr->match_value)
		{
			fc_dbg("poll_cnt: %d", poll_cnt);
			return 0;
		}

		if (poll_cnt >= poll_opr->poll_cnt)
		{
			return -FC_RET_CODE_POLL_OPR_TIMEOUT;
		}
	}

	return 0;
}

int flash_controller_exec_operation(flash_controller_t *fc, const fc_operation_t *std_opr)
{
	int ret = 0;
	uint32_t flash_addr_len = 0;
	int mem_map_enable;
	fc_poll_operation_t *poll_opr =NULL;

#ifdef DEBUG_FC_OPERATION
	fc_info("--------standard operation info--------");
	dump_std_opr(std_opr, 0);
#endif

	ret = fc_check_std_opr(std_opr);
	if (ret)
	{
		return ret;
	}

	poll_opr = std_opr->poll_opr;
	if (poll_opr)
	{
#ifdef DEBUG_FC_OPERATION
		fc_info("--------poll operation info--------");
		dump_poll_opr(poll_opr, 0);
#endif

		ret = fc_check_std_opr(&poll_opr->std_opr);
		if (ret)
		{
			return ret;
		}
	}

	ret = fc_lock(fc);
	if (ret)
	{
		return ret;
	}

#ifdef CONFIG_USE_FLASH_CONTROLLER_ON_AMP
	ret = fc_amp_lock(fc);
	if (ret)
	{
		goto exit_with_unlock;
	}
#endif

	if (is_fc_busy(fc))
	{
		ret = -FC_RET_CODE_HW_BUSY;
		goto exit_with_amp_unlock;
	}

	fc_setup_std_operation(fc, std_opr);

	mem_map_enable = is_mem_map_mode_enable(fc);
	if (mem_map_enable)
	{
		fc_indirect_mode_prepare(fc);
		flash_addr_len = fc_get_flash_addr_len(fc);
	}

	ret = fc_exec_std_operation(fc, std_opr);

	if (mem_map_enable)
	{
		if (ret)
			fc_err("fc_exec_operation failed, ret: %d", ret);

		if (poll_opr)
		{
			ret = fc_exec_poll_operation(fc, poll_opr);
		}

		fc_set_flash_addr_len(fc, flash_addr_len);
		fc_indirect_mode_unprepare(fc);
	}

exit_with_amp_unlock:
#ifdef CONFIG_USE_FLASH_CONTROLLER_ON_AMP
	fc_amp_unlock(fc);
#endif

exit_with_unlock:
	fc_unlock(fc);

	return ret;
}

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
int flash_controller_setup_mem_map_opr(flash_controller_t *fc, const fc_mem_map_operation_t *opr)
{
	int ret = 0;

	ret = fc_check_mem_map_opr(opr);
	if (ret)
	{
		return -1;
	}

	if (fc_lock(fc))
	{
		return -2;
	}

	fc_setup_mem_map_operation(fc, opr);

	fc_unlock(fc);
	return ret;
}

int flash_controller_add_mem_map(flash_controller_t *fc, const fc_mem_map_info_t *mmap)
{
	int ret = 0, mmap_index;
	unsigned long start_addr, end_addr;
	uint32_t offset;

	start_addr = mmap->mem_addr;
	if (start_addr < fc->start_addr)
	{
		return -1;
	}

	end_addr = mmap->mem_addr + mmap->len; //not include it self
	if (end_addr > (fc->end_addr + 1))
	{

		return -2;
	}

	if (start_addr & (FLASH_CONTROLLER_MEM_MAP_ADDR_ALIGN - 1))
	{
		fc_err("mem addr is not align to %d byte boundary", FLASH_CONTROLLER_MEM_MAP_ADDR_ALIGN);
		return -3;
	}

	if ((end_addr & MEM_MAP_ADDR_MASK) != end_addr)
	{
		return -4;
	}

	if (fc_lock(fc))
	{
		return -5;
	}

	if (fc_find_empty_mem_map(fc, &mmap_index))
	{
		ret = -6;
		goto exit_with_unlock;
	}

	fc_dbg("mem_addr: 0x%08lx, data_addr: 0x%08x, len: %u", start_addr, mmap->data_addr, mmap->len);

	offset = mmap->data_addr;

	uint32_t reg_offset = mmap_index * MEM_MAP_CFG_REG_ADDR_INTERVAL;

	fc_dbg("reg_offset: 0x%08x, start: 0x%08lx, end: 0x%08lx, offset: 0x%08x",
		   reg_offset, start_addr, end_addr, offset);

	fc_mod_reg(fc, MEM_MAP0_START_ADDR_REG_OFF + reg_offset,
			   start_addr << MEM_MAP_ADDR_SHIFT, MEM_MAP_ADDR_MASK);

	fc_mod_reg(fc, MEM_MAP0_END_ADDR_REG_OFF + reg_offset,
			   end_addr << MEM_MAP_ADDR_SHIFT, MEM_MAP_ADDR_MASK);

	fc_mod_reg(fc, MEM_MAP0_OFFSET_REG_OFF + reg_offset,
			   offset << MEM_MAP_OFFSET_SHIFT, MEM_MAP_OFFSET_MASK);

	fc_mod_reg(fc, MEM_MAP0_OFFSET_REG_OFF + reg_offset,
			   1 << MEM_MAP_ENABLE_SHIFT, MEM_MAP_ENABLE_MASK);

	fc_enable_mem_map_mode(fc);

exit_with_unlock:
	fc_unlock(fc);

	return ret;
}

int flash_controller_del_mem_map(flash_controller_t *fc, const fc_mem_map_info_t *mmap)
{
	int ret = 0;

	if (fc_lock(fc))
	{
		return -1;
	}

	if (is_mem_map_mode_enable(fc))
	{
		//disable
	}

	fc_unlock(fc);
	return ret;
}

int flash_controller_set_output_clk_freq(flash_controller_t *fc, unsigned int clk_freq)
{
	int ret;
	hal_clk_status_t clk_ret;
	uint32_t old_freq, new_freq;

	hal_clk_type_t clk_type = fc->input_clk_type;
	hal_clk_id_t clk_id = fc->input_clk_id;

	ret = fc_lock(fc);
	if (ret)
	{
		return ret;
	}

	hal_clk_t mclk = hal_clock_get(clk_type, clk_id);
	if (!mclk)
	{
		fc_err("get clk handle failed, type: %d, id: %d", clk_type, clk_id);
		ret = -FC_RET_CODE_GET_CLK_FAILED;
		goto exit_with_unlock;
	}

	old_freq = hal_clk_get_rate(mclk);

	clk_ret = hal_clk_set_rate(mclk, clk_freq);
	if (clk_ret)
	{
		fc_err("hal_clk_set_rate failed, ret: %d", clk_ret);
		ret = -FC_RET_CODE_SET_CLK_FREQ_FAILED;
		goto exit_with_put_mclk;
	}

	new_freq = hal_clk_get_rate(mclk);

	fc->input_clk_freq = new_freq;

	fc_info("FC module clk freq change, old: %uHz, new: %uHz, target: %uHz",
			old_freq, new_freq, clk_freq);

	fc_set_sample_delay(fc);
exit_with_put_mclk:
	hal_clock_put(mclk);

exit_with_unlock:
	fc_unlock(fc);

	return ret;
}
#endif

int flash_controller_init(flash_controller_t *fc, unsigned int clk_freq)
{
	int ret;

	if (!hal_mutex_lock(&fc->mutex))
	{
		fc_unlock(fc);
		fc_info("flash controller has been already inited, fc: 0x%p", fc);
		return 0;
	}

	ret = fc_lock_init(fc);
	if (ret)
	{
		return ret;
	}

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
	ret = fc_init_pinmux(fc);
	if (ret)
	{
		goto exit;
	}

	ret = fc_init_module_clk(fc, clk_freq);
	if (ret)
	{
		goto exit;
	}
	ret = fc_reset_module(fc);
	if (ret)
	{
		goto exit;
	}

	fc_init_cache_cfg(fc);
	fc_init_spi_timing(fc);
	fc_set_sample_delay(fc);
#endif

#ifdef CONFIG_DISABLE_INTERRUPT_WHEN_USING_INDIRECT_MODE
	hal_spin_lock_init(&fc->spinlock);
#endif

#ifdef CONFIG_USE_FLASH_CONTROLLER_ON_AMP
	fc->amp_lock_id = SPINLOCK_FC_LOCK_ID;
#endif

#ifdef FLASH_CONTROLLER_PM_SUPPORT
	memset(&fc->pm_dev, 0, sizeof(fc->pm_dev));
	fc->pm_dev.name = "flashc";
	fc->pm_dev.ops = &s_fc_pm_devops;
	fc->pm_dev.data = fc;
	ret = pm_devops_register(&fc->pm_dev);
	if (ret)
		fc_warn("pm_devops_register failed, ret: %d", ret);
#endif

	ret = 0;

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
exit:
#endif
	if (ret)
	{
		hal_mutex_detach(&fc->mutex);
		fc_err("Flash controller init failed!, ret: %d", ret);
	}
	else
	{
		fc_info("Flash controller init success!");
	}

#ifdef DUMP_FC_REG_AFTER_INIT
	dump_fc_reg(fc);
#endif

	return ret;
}

#ifdef FLASH_CONTROLLER_PM_SUPPORT
int fc_suspend(struct pm_device *dev, suspend_mode_t state)
{
	int ret;
	flash_controller_t *fc = dev->data;

	fc_dbg("flash controller(0x%p) suspend", fc);
	switch (state)
	{
		case PM_MODE_SLEEP:
			ret = fc_disable_module_clk(fc);
			if (ret)
				fc_err("fc_disable_module_clk failed, ret: %d", ret);
			break;
		case PM_MODE_STANDBY:
		case PM_MODE_HIBERNATION:
			break;
		default:
			fc_err("invalid pm state: %d", state);
			break;
	}

	return 0;
}

int fc_resume(struct pm_device *dev, suspend_mode_t state)
{
	int ret;
	flash_controller_t *fc = dev->data;
	fc_dbg("flash controller(0x%p) resume", fc);

	switch (state)
	{
		case PM_MODE_SLEEP:
			ret = fc_enable_module_clk(fc);
			if (ret)
				fc_err("fc_disable_module_clk failed, ret: %d", ret);
			break;
		case PM_MODE_STANDBY:
		case PM_MODE_HIBERNATION:
			ret = fc_init_pinmux(fc);
			if (ret)
				fc_err("fc_init_pinmux failed, ret: %d", ret);

			ret = fc_init_module_clk(fc, fc->input_clk_freq);
			if (ret)
				fc_err("fc_init_module_clk failed, ret: %d", ret);

			ret = fc_reset_module(fc);
			if (ret)
				fc_err("fc_reset_module failed, ret: %d", ret);

			fc_init_cache_cfg(fc);
			fc_init_spi_timing(fc);
			fc_set_sample_delay(fc);
			break;
		default:
			fc_err("invalid pm state: %d", state);
			break;
	}

	return 0;
}

static struct pm_devops s_fc_pm_devops =
{
	.suspend_noirq = fc_suspend,
	.resume_noirq = fc_resume,
};
#endif

