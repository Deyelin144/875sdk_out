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

#include "spi_nor.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sunxi_hal_common.h>
#include <hal_time.h>

#include "sfdp.h"

//#define DUMP_SPI_NOR_OBJECT_AFTER_SCAN

#define SPINOR_CMD_READ 0x03 /* Read data bytes (low frequency) */
#define SPINOR_CMD_READ_FAST 0x0B /* Read data bytes (high frequency) */
#define SPINOR_CMD_PP 0x02 /* Page program (up to 256 bytes) */
#define SPINOR_CMD_PP_1_1_4 0x32 /* Quad page program1 */
#define SPINOR_CMD_PP_1_4_4 0x38 /* Quad page program2 */
#define SPINOR_CMD_BE_4K  0x20 /* Erase 4KiB block */
#define SPINOR_CMD_BE_32K 0x52 /* Erase 32KiB block */
#define SPINOR_CMD_BE_64K 0xD8 /* Erase 64KiB block */
#define SPINOR_CMD_ERASE_CHIP 0x60
#define SPINOR_CMD_WREN 0x06
#define SPINOR_CMD_WRDI 0x04
#define SPINOR_CMD_READ_SR1 0x05
#define SPINOR_CMD_WRITE_SR1 0x01
#define SPINOR_CMD_READ_SCUR 0x2B
#define SPINOR_CMD_RESET_EN 0x66
#define SPINOR_CMD_RESET 0x99
#define SPINOR_CMD_ENTER_DPD 0xB9
#define SPINOR_CMD_EXIT_DPD 0xAB

#define SPINOR_CMD_READ_SFDP 0x5A /* Read SFDP */
#define SPINOR_CMD_READ_JEDEC_ID 0x9F

/* command with 4-byte address */
#define SPINOR_CMD_READ_4B 0x13 /* Read data bytes (low frequency) */
#define SPINOR_CMD_READ_FAST_4B 0x0C /* Read data bytes (high frequency) */
#define SPINOR_CMD_READ_1_1_2_4B 0x3C /* Read data bytes (Dual Output SPI) */
#define SPINOR_CMD_READ_1_2_2_4B 0xBC /* Read data bytes (Dual I/O SPI) */
#define SPINOR_CMD_READ_1_1_4_4B 0x6C /* Read data bytes (Quad Output SPI) */
#define SPINOR_CMD_READ_1_4_4_4B 0xEC /* Read data bytes (Quad I/O SPI) */
#define SPINOR_CMD_PP_4B  0x12 /* Page program (up to 256 bytes) */
#define SPINOR_CMD_PP_1_1_4_4B 0x34 /* Quad page program1 */
#define SPINOR_CMD_PP_1_4_4_4B 0x3E /* Quad page program2 */
#define SPINOR_CMD_BE_4K_4B 0x21 /* Erase 4KiB block */
#define SPINOR_CMD_BE_32K_4B 0x5C /* Erase 32KiB block */
#define SPINOR_CMD_BE_64K_4B 0xDC /* Erase 64KiB block */

#define NOR_BUSY_MASK BIT(0)
#define NOR_SR_BIT_WEL BIT(1)

#define MAX_WAIT_LOOP (((unsigned int)(-1))/2)

#ifdef SPI_NOR_FLASH_PM_SUPPORT
static struct pm_devops s_nor_pm_devops;
#endif

static int spi_nor_hwcaps2type(u32 hwcaps, const int table[][2], size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (table[i][0] == (int)hwcaps)
			return table[i][1];

	return -EINVAL;
}

int spi_nor_hwcaps_read2proto_type(u32 hwcaps)
{
	static const int hwcaps_read2cmd[][2] =
	{
		{ SNOR_HWCAPS_READ, SNOR_READ_PROTO_GENERAL },
		{ SNOR_HWCAPS_READ_FAST, SNOR_READ_PROTO_FAST },
		{ SNOR_HWCAPS_READ_1_1_2, SNOR_READ_PROTO_1_1_2 },
		{ SNOR_HWCAPS_READ_1_2_2, SNOR_READ_PROTO_1_2_2 },
		{ SNOR_HWCAPS_READ_2_2_2, SNOR_READ_PROTO_2_2_2 },
		{ SNOR_HWCAPS_READ_1_1_4, SNOR_READ_PROTO_1_1_4 },
		{ SNOR_HWCAPS_READ_1_4_4, SNOR_READ_PROTO_1_4_4 },
		{ SNOR_HWCAPS_READ_4_4_4, SNOR_READ_PROTO_4_4_4 },
	};

	return spi_nor_hwcaps2type(hwcaps, hwcaps_read2cmd,
							   ARRAY_SIZE(hwcaps_read2cmd));
}

static inline int spi_nor_lock_init(spi_nor_t *nor)
{
	int ret = hal_mutex_init(&nor->lock);
	if (ret)
	{
		snor_err("init lock for spi nor flash failed, ret: %d", ret);
		return -1;
	}

	return 0;
}

int spi_nor_lock(spi_nor_t *nor)
{
	int ret = hal_mutex_lock(&nor->lock);
	if (ret)
	{
		snor_err("lock spi nor flash failed, ret: %d", ret);
		return -1;
	}

	return 0;
}

int spi_nor_unlock(spi_nor_t *nor)
{
	int ret = hal_mutex_unlock(&nor->lock);
	if (ret)
	{
		snor_err("unlock spi nor flash failed, ret: %d", ret);
		return -1;
	}

	return 0;
}

static inline void spi_nor_msleep(unsigned int msec)
{
	hal_msleep(msec);
}

static __attribute__((unused)) inline void spi_nor_udelay(unsigned int usec)
{
	hal_udelay(usec);
}

int nor_write_enable(spi_nor_t *nor);

int nor_read_status(spi_nor_t *nor, uint8_t *sr)
{
	spi_nor_operation_t opr;

	memset(&opr.proto, 0, sizeof(opr.proto));
	opr.proto.cmd_field_line_num = 1;
	opr.cmd = SPINOR_CMD_READ_SR1;

	opr.proto.payload_field_line_num = 1;
	opr.payload_direction = SNOR_PYALOAD_DIRECTION_OUTPUT;
	opr.payload_buf = sr;
	opr.payload_len = 1;

	opr.poll_opr = NULL;

	return nor->controller->ops->exec_opr(nor->controller->id, &opr);
}

static int nor_check_busy(spi_nor_t *nor, int *is_busy)
{
	int ret;
	unsigned char reg;

	ret = nor_read_status(nor, &reg);
	if (ret)
		return ret;

	*is_busy = 0;
	if (reg & NOR_BUSY_MASK)
		*is_busy = 1;

	return 0;
}

/**
 * As the minimum time is 1ms, to save time, we wait ready under 2 step:
 * 1. sleep on ms, which take mush time.
 * 2. check times on cpu. It will be ready soon in this case
 */
int nor_wait_ready(spi_nor_t *nor, int ms, int times)
{
	int ret, is_busy, check_success_cnt = 0;
	int _ms = ms, _times = times;

	do
	{
		ret = nor_check_busy(nor, &is_busy);
		if (!ret)
		{
			check_success_cnt++;
			if (is_busy == 0)
				return 0;
		}

		if (_ms)
			spi_nor_msleep(1);
	}
	while (--_ms > 0);

	do
	{
		ret = nor_check_busy(nor, &is_busy);
		if (!ret)
		{
			check_success_cnt++;
			if (is_busy == 0)
				return 0;
		}


	}
	while (--_times > 0);

	/* check the last time */
	ret = nor_check_busy(nor, &is_busy);
	if (!ret)
	{
		check_success_cnt++;
		if (is_busy == 0)
			return 0;
	}

	snor_err("wait nor flash for %d ms and %d loop timeout", ms, times);

	if (!check_success_cnt)
		return -EIO;

	return -EBUSY;
}

int nor_send_cmd(spi_nor_t *nor, unsigned char cmd)
{
	int ret;
	spi_nor_operation_t opr;

	memset(&opr.proto, 0, sizeof(opr.proto));
	opr.proto.cmd_field_line_num = 1;
	opr.cmd = cmd;
	opr.poll_opr = NULL;

	ret = nor->controller->ops->exec_opr(nor->controller->id, &opr);
	if (ret)
		return ret;

	return nor_wait_ready(nor, 0, 500);
}

static int spi_nor_write_enable(spi_nor_t *nor)
{
	int ret;
	unsigned char sr;

	ret = nor_send_cmd(nor, SPINOR_CMD_WREN);
	if (ret)
	{
		snor_err("send WREN failed, ret: %d", ret);
		return ret;
	}

	ret = nor_read_status(nor, &sr);
	if (ret)
		return ret;

	if (!(sr & NOR_SR_BIT_WEL))
	{
		snor_err("enable write failed");
		return -EINVAL;
	}
	return 0;
}

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
static int spi_nor_reset(spi_nor_t *nor)
{
	int ret;
	spi_nor_operation_t opr;

	memset(&opr.proto, 0, sizeof(opr.proto));
	opr.proto.cmd_field_line_num = 1;
	opr.cmd = SPINOR_CMD_RESET_EN;
	opr.poll_opr = NULL;

	ret = nor->controller->ops->exec_opr(nor->controller->id, &opr);
	if (ret)
		return ret;

	opr.cmd = SPINOR_CMD_RESET;

	ret = nor->controller->ops->exec_opr(nor->controller->id, &opr);
	if (ret)
		return ret;

	spi_nor_msleep(1);

	return 0;
}

static int spi_nor_enter_deep_power_down_mode(const spi_nor_t *nor)
{
	spi_nor_operation_t opr;

	memset(&opr.proto, 0, sizeof(opr.proto));
	opr.proto.cmd_field_line_num = 1;
	opr.cmd = SPINOR_CMD_ENTER_DPD;
	opr.poll_opr = NULL;

	return nor->controller->ops->exec_opr(nor->controller->id, &opr);
}

static int spi_nor_exit_deep_power_down_mode(const spi_nor_t *nor)
{
	int ret;
	spi_nor_operation_t opr;

	memset(&opr.proto, 0, sizeof(opr.proto));
	opr.proto.cmd_field_line_num = 1;
	opr.cmd = SPINOR_CMD_EXIT_DPD;
	opr.poll_opr = NULL;

	ret = nor->controller->ops->exec_opr(nor->controller->id, &opr);

	spi_nor_udelay(50);
	return ret;
}
#endif

static int spi_nor_read_jedec_id(spi_nor_t *nor, unsigned char *buf, unsigned int len)
{
	spi_nor_operation_t opr;

	if (len != SNOR_MAX_JEDEC_ID_LEN)
		return -1;

	memset(&opr.proto, 0, sizeof(opr.proto));
	opr.proto.cmd_field_line_num = 1;
	opr.proto.payload_field_line_num = 1;

	opr.cmd = SPINOR_CMD_READ_JEDEC_ID;
	opr.payload_direction = SNOR_PYALOAD_DIRECTION_OUTPUT;
	opr.payload_buf = buf;
	opr.payload_len = len;

	opr.poll_opr = NULL;

	return nor->controller->ops->exec_opr(nor->controller->id, &opr);
}

int spi_nor_read_sfdp(spi_nor_t *nor, unsigned int addr, unsigned int len, unsigned char *buf)
{
	spi_nor_operation_t opr;

	memset(&opr.proto, 0, sizeof(opr.proto));
	opr.proto.addr_field_byte_len = 3;
	opr.proto.mode_clocks_num = 0;
	opr.proto.dummy_clocks_num = 8;
	opr.proto.cmd_field_line_num = 1;
	opr.proto.addr_field_line_num = 1;
	opr.proto.payload_field_line_num = 1;

	opr.cmd = SPINOR_CMD_READ_SFDP;
	opr.addr = addr;
	memset(opr.mode, 0, sizeof(opr.mode));
	opr.payload_direction = SNOR_PYALOAD_DIRECTION_OUTPUT;
	opr.payload_buf = buf;
	opr.payload_len = len;

	opr.poll_opr = NULL;

	return nor->controller->ops->exec_opr(nor->controller->id, &opr);
}

static int spi_nor_read_data(spi_nor_t *nor, unsigned int addr, unsigned char *buf, unsigned int len)
{
	spi_nor_operation_t opr;
	spi_nor_rw_proto_t *read_proto = &nor->info.read_proto[nor->cfg.read_proto_type];

	memcpy(&opr.proto, &read_proto->proto, sizeof(opr.proto));
	opr.cmd = read_proto->cmd;

	opr.addr = addr;
	memset(opr.mode, 0, sizeof(opr.mode));
	opr.payload_direction = SNOR_PYALOAD_DIRECTION_OUTPUT;
	opr.payload_buf = buf;
	opr.payload_len = len;

	opr.poll_opr = NULL;
	return nor->controller->ops->exec_opr(nor->controller->id, &opr);
}

int spi_nor_read(spi_nor_t *nor, unsigned int addr, unsigned int len, unsigned char *buf)
{
	int ret;

	snor_dbg("from 0x%08x, len %zd", addr, len);

	ret = spi_nor_lock(nor);
	if (ret)
		return ret;

	ret = spi_nor_read_data(nor, addr, buf, len);

	spi_nor_unlock(nor);
	return ret;
}

static int __spi_nor_write_data(spi_nor_t *nor, unsigned int addr, unsigned int len, const uint8_t *buf)
{
	spi_nor_operation_t opr;
	spi_nor_poll_operation_t poll_opr;

	memcpy(&opr.proto, &nor->cfg.write_proto, sizeof(opr.proto));
	opr.cmd = nor->cfg.write_cmd;
	opr.addr = addr;
	memset(opr.mode, 0, sizeof(opr.mode));
	opr.payload_direction = SNOR_PYALOAD_DIRECTION_INPUT;
	opr.payload_buf = (uint8_t *)buf;
	opr.payload_len = len;

	memset(&poll_opr.proto, 0, sizeof(poll_opr.proto));
	poll_opr.proto.cmd_field_line_num = 1;
	poll_opr.proto.payload_field_line_num = 1;
	poll_opr.cmd = SPINOR_CMD_READ_SR1;
	poll_opr.data_len = 1;
	poll_opr.bit_mask = 0x1;
	poll_opr.match_value = 0x0;
	poll_opr.poll_cnt = 0xFFFF;
	opr.poll_opr = &poll_opr;

	return nor->controller->ops->exec_opr(nor->controller->id, &opr);
}

static int spi_nor_write_data(spi_nor_t *nor, unsigned int addr, unsigned int len, const uint8_t *buf)
{
	int ret = -EINVAL;

	ret = spi_nor_write_enable(nor);
	if (ret)
		return ret;

	ret = __spi_nor_write_data(nor, addr, len, buf);
	if (ret)
		return ret;

	return nor_wait_ready(nor, 0, 100 * 1000);
}

int spi_nor_write(spi_nor_t *nor, unsigned int addr, unsigned int len, const unsigned char *buf)
{
	int ret = 0;

	ret = spi_nor_lock(nor);
	if (ret)
	{
		snor_err("write: lock nor failed\n");
		goto out;
	}

	snor_dbg("try to write addr 0x%x with len %u\n", addr, len);

	ret = nor_wait_ready(nor, 0, 500);
	if (ret)
		goto unlock;

	while (len)
	{
		unsigned int align_addr = ALIGN_UP(addr + 1, nor->info.page_size);
		unsigned int wlen = MIN(align_addr - addr, len);

		ret = spi_nor_write_data(nor, addr, wlen, buf);
		if (ret)
			goto unlock;

		addr += wlen;
		buf += wlen;
		len -= wlen;
	}

unlock:
	spi_nor_unlock(nor);
out:
	if (ret)
		snor_err("write address 0x%x with len %u failed\n", addr, len);
	return ret;
}

static int spi_nor_erase_do(spi_nor_t *nor, char cmd, unsigned int addr)
{
	int ret;
	spi_nor_operation_t opr;
	spi_nor_poll_operation_t poll_opr;

	ret = spi_nor_write_enable(nor);
	if (ret)
		goto out;

	memset(&opr.proto, 0, sizeof(opr.proto));
	opr.proto.cmd_field_line_num = 1;
	opr.proto.addr_field_line_num = 1;
	opr.proto.addr_field_byte_len= nor->cfg.addr_mode;
	opr.cmd = cmd;
	opr.addr = addr;

	memset(&poll_opr.proto, 0, sizeof(poll_opr.proto));
	poll_opr.proto.cmd_field_line_num = 1;
	poll_opr.proto.payload_field_line_num = 1;
	poll_opr.cmd = SPINOR_CMD_READ_SR1;
	poll_opr.data_len = 1;
	poll_opr.bit_mask = 0x1;
	poll_opr.match_value = 0x0;
	poll_opr.poll_cnt = 0xFFFFFF;
	opr.poll_opr = &poll_opr;

	ret = nor->controller->ops->exec_opr(nor->controller->id, &opr);

out:
	if (ret)
		snor_err("erase address 0x%x with cmd 0x%x failed, ret: %d", addr, cmd, ret);
	return ret;
}

static inline int nor_erase_4k(spi_nor_t *nor, unsigned int addr)
{
	int ret;

	ret = spi_nor_erase_do(nor, nor->cfg.erase_4k_cmd, addr);
	if (ret)
		goto out;

	ret = nor_wait_ready(nor, 30, MAX_WAIT_LOOP);
out:
	return ret;
}

static inline int nor_erase_32k(spi_nor_t *nor, unsigned int addr)
{
	int ret;

	ret = spi_nor_erase_do(nor, nor->cfg.erase_32k_cmd, addr);
	if (ret)
		goto out;

	ret = nor_wait_ready(nor, 120, MAX_WAIT_LOOP);
out:
	return ret;
}

static inline int nor_erase_64k(spi_nor_t *nor, unsigned int addr)
{
	int ret;

	ret = spi_nor_erase_do(nor, nor->cfg.erase_64k_cmd, addr);
	if (ret)
		goto out;

	ret = nor_wait_ready(nor, 150, MAX_WAIT_LOOP);
out:
	return ret;
}

static inline int nor_erase_all(spi_nor_t *nor)
{
	int ret;

	ret = spi_nor_write_enable(nor);
	if (ret)
		goto out;

	ret = nor_send_cmd(nor, SPINOR_CMD_ERASE_CHIP);
	if (ret)
		goto out;

	ret = nor_wait_ready(nor, 26 * 1000, MAX_WAIT_LOOP);
out:
	return ret;
}

int spi_nor_erase(spi_nor_t *nor, unsigned int addr, unsigned int len)
{
	int ret = -EBUSY;

	spi_nor_info_t *info = &nor->info;
	spi_nor_cfg_t *cfg = &nor->cfg;

	ret = spi_nor_lock(nor);
	if (ret)
	{
		snor_err("erase: lock nor failed");
		return ret;
	}

	if (!len || (addr + len > info->total_size))
	{
		snor_err("invalid addr(0x%08x) or len(%u) when erase, flash size: %u",
				 addr, len, info->total_size);
		ret = -EINVAL;
		goto unlock;
	}

	uint32_t min_erase_size = info->total_size;

	if (info->hwcaps & SNOR_HWCAPS_64K_ERASE_BLK)
	{
		min_erase_size = SZ_64K;
	}

	if (info->hwcaps & SNOR_HWCAPS_32K_ERASE_BLK)
	{
		min_erase_size = SZ_32K;
	}

	if (info->hwcaps & SNOR_HWCAPS_4K_ERASE_BLK)
	{
		min_erase_size = SZ_4K;
	}

	if (len % min_erase_size)
	{
		snor_err("erase len(%u) is not align to min erase size %u\n", len, min_erase_size);
		ret = -EINVAL;
		goto unlock;
	}

	snor_dbg("try to erase at addr 0x%08x with len %u\n", addr, len);

	if (addr == 0 && len == info->total_size)
	{
		ret = nor_erase_all(nor);
		goto unlock;
	}

	while (len)
	{

		if ((len >= SZ_64K) &&
				(addr % SZ_64K == 0) &&
				(info->hwcaps & SNOR_HWCAPS_64K_ERASE_BLK))
		{
			snor_dbg("try to erase 64k at 0x%08x\n", addr);
			ret = nor_erase_64k(nor, addr);
			if (ret)
				goto unlock;
			addr += SZ_64K;
			len -= SZ_64K;
			continue;
		}

		if ((len >= SZ_32K) &&
				(addr % SZ_32K == 0) &&
				(info->hwcaps & SNOR_HWCAPS_32K_ERASE_BLK))
		{
			snor_dbg("try to erase 32k at 0x%08x\n", addr);
			ret = nor_erase_32k(nor, addr);
			if (ret)
				goto unlock;
			addr += SZ_32K;
			len -= SZ_32K;
			continue;
		}

		if ((len >= SZ_4K) &&
				(addr % SZ_4K == 0) &&
				(info->hwcaps & SNOR_HWCAPS_4K_ERASE_BLK))
		{
			snor_dbg("try to erase 4k at 0x%08x\n", addr);
			ret = nor_erase_4k(nor, addr);
			if (ret)
				goto unlock;
			addr += SZ_4K;
			len -= SZ_4K;
			continue;
		}

		snor_err("no erase cmd matched, addr 0x%08x, len 0x%08x, hwcaps: 0x%08x, flag:0x%08x\n",
				 addr, len, info->hwcaps, cfg->flag);
		break;
	}

	ret = len ? -EINVAL : 0;
unlock:
	spi_nor_unlock(nor);
	return ret;
}

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
static int spi_nor_set_clk_freq(const spi_nor_t *nor, unsigned int clk_freq)
{
	int ret = nor->controller->ops->set_output_clk_freq(nor->controller->id, clk_freq);
	if (ret)
	{
		snor_err("set clk freq failed, ret: %d", ret);
		return -1;
	}

	return 0;
}

static __attribute__((unused))
spi_nor_mem_map_t *spi_nor_find_empty_mem_map(spi_nor_t *nor, unsigned int *index)
{
	unsigned int i;
	spi_nor_mem_map_cfg_t *mmap = &nor->cfg.mmap;

	for (i = 0; i < SNOR_MAX_MEM_MAP_CNT; i++)
	{
		if (!mmap->is_valid[i])
		{
			if (index)
				*index = i;
			return &mmap->maps[i];
		}
	}

	return NULL;
}

static __attribute__((unused)) int is_mem_map_full(spi_nor_t *nor)
{

	if (spi_nor_find_empty_mem_map(nor, NULL))
	{
		return 0;
	}
	return 1;
}

static spi_nor_mem_map_t *spi_nor_find_mem_map(spi_nor_t *nor, unsigned int *index,
		unsigned long mem_addr, unsigned int len, unsigned int data_addr)
{
	unsigned int i;
	spi_nor_mem_map_cfg_t *mmap = &nor->cfg.mmap;
	spi_nor_mem_map_t *map;

	for (i = 0; i < SNOR_MAX_MEM_MAP_CNT; i++)
	{
		if (!mmap->is_valid[i])
			continue;

		map = &mmap->maps[i];
		if ((mem_addr == map->mem_addr) &&
				(len == map->len) &&
				(data_addr == map->data_addr))
		{
			if (index)
				*index = i;
			return map;
		}
	}

	return NULL;
}

static int __spi_nor_add_mem_map(const spi_nor_t *nor, const spi_nor_mem_map_t *mmap)
{
	int ret = 0;
	if (!nor->controller->ops->add_mem_map)
	{
		return -1;
	}

	ret = nor->controller->ops->add_mem_map(nor->controller->id, mmap);
	if (ret)
	{
		snor_err("add_mem_map failed, ret: %d", ret);
		return -2;
	}

	return 0;
}

static int spi_nor_add_mem_map_when_resume(const spi_nor_t *nor)
{
	int i, ret = 0;
	const spi_nor_mem_map_t *mmap;

	for (i = 0; i < SNOR_MAX_MEM_MAP_CNT; i++)
	{
		if (nor->cfg.mmap.is_valid[i])
		{
			mmap = &nor->cfg.mmap.maps[i];
			ret = __spi_nor_add_mem_map(nor, mmap);
		}
	}

	return ret;
}

int spi_nor_add_mem_map(spi_nor_t *nor,
						unsigned long mem_addr, unsigned int len, unsigned int data_addr)
{
	int ret = 0;
	unsigned int mmap_index = 0;

	ret = spi_nor_lock(nor);
	if (ret)
	{
		snor_err("lock nor failed");
		return ret;
	}

	if (!spi_nor_find_empty_mem_map(nor, &mmap_index))
	{
		ret = -2;
		goto unlock;
	}

	if (!len || (data_addr + len > nor->info.total_size))
	{
		snor_err("invalid addr(0x%08x) or len(%u) when add mem map, flash size: %u",
				 data_addr, len, nor->info.total_size);
		ret = -3;
		goto unlock;

	}

	spi_nor_mem_map_t mmap;
	mmap.mem_addr = mem_addr;
	mmap.len = len;
	mmap.data_addr = data_addr;

	ret = __spi_nor_add_mem_map(nor, &mmap);
	//ret = nor->controller->ops->add_mem_map(nor->controller->id, &mmap);
	if (ret)
	{
		goto unlock;
	}

	nor->cfg.mmap.maps[mmap_index] = mmap;
	nor->cfg.mmap.is_valid[mmap_index] = 1;
	ret = 0;
unlock:
	spi_nor_unlock(nor);
	return ret;
}

int spi_nor_del_mem_map(spi_nor_t *nor, unsigned long mem_addr, unsigned int len, unsigned int data_addr)
{
	int ret = 0;
	unsigned int mmap_index = 0;

	ret = spi_nor_lock(nor);
	if (ret)
	{
		snor_err("lock nor failed");
		return ret;
	}

	if (!spi_nor_find_mem_map(nor, &mmap_index, mem_addr, len, data_addr))
	{
		ret = -2;
		goto unlock;
	}

	if (!nor->controller->ops->del_mem_map)
	{
		ret = -3;
		goto unlock;
	}

	spi_nor_mem_map_t mmap;
	mmap = nor->cfg.mmap.maps[mmap_index];

	ret = nor->controller->ops->del_mem_map(nor->controller->id, &mmap);
	if (ret)
	{
		snor_err("del_mem_map failed, ret: %d", ret);
		goto unlock;
	}

	nor->cfg.mmap.is_valid[mmap_index] = 0;
	ret = 0;
unlock:
	spi_nor_unlock(nor);
	return ret;
}

static int spi_nor_setup_mem_map_proto(spi_nor_t *nor, const spi_nor_mem_map_proto_t *mmap_proto)
{
	int ret = 0;

	if (!nor->controller->ops->setup_mem_map_proto)
	{
		return -1;
	}

	ret = nor->controller->ops->setup_mem_map_proto(nor->controller->id, mmap_proto);
	if (ret)
	{
		snor_err("setup_mem_map_proto failed, ret: %d", ret);
		return -2;
	}

	ret = 0;

	return ret;
}

static int spi_nor_recover_mem_map_when_resume(spi_nor_t *nor)
{
	int ret = 0;
	ret = spi_nor_setup_mem_map_proto(nor, &nor->cfg.mmap.mmap_proto);
	if (ret)
		return -1;

	ret = spi_nor_add_mem_map_when_resume(nor);
	if (ret)
		return -2;
	return 0;
}
#endif

static void dump_spi_nor_proto(const spi_nor_protocol_info_t *proto, int need_header)
{
	if (need_header)
		snor_info("--------SPI NOR flash protocol info--------");

	snor_info("cmd line: %u", proto->cmd_field_line_num);
	snor_info("addr line: %u", proto->addr_field_line_num);
	snor_info("payload line: %u", proto->payload_field_line_num);

	if (proto->addr_field_line_num)
	{
		snor_info("addr len: %u", proto->addr_field_byte_len);
		snor_info("mode clk num: %u", proto->mode_clocks_num);
		snor_info("dummy clk num: %u", proto->dummy_clocks_num);
	}
}

static void dump_spi_nor_read_proto_info(const spi_nor_rw_proto_t *read_proto, int need_header)
{
	if (need_header)
		snor_info("--------SPI NOR flash read proto info--------");

	snor_info("cmd: 0x%02x", read_proto->cmd);
	dump_spi_nor_proto(&read_proto->proto, 0);
}

static void dump_spi_nor_info(const spi_nor_info_t *info, int need_header)
{
	if (need_header)
		snor_info("--------SPI NOR flash info--------");

	snor_info("model: '%s'", info->model);
	snor_info("JEDEC ID: %02x %02x %02x",
			  info->id[0], info->id[1], info->id[2]);

	snor_info("total: %uB", info->total_size);
	snor_info("block: %uB", info->blk_size);
	snor_info("page: %uB", info->page_size);
	snor_info("hwcaps: 0x%08x", info->hwcaps);

	for (int i = 0; i < SNOR_READ_PROTO_TYPE_MAX; i++)
	{
		snor_info("--------read proto info[%d]--------", i);
		dump_spi_nor_read_proto_info(&info->read_proto[i], 0);
	}
}

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
static void dump_spi_nor_mem_map_proto_info(const spi_nor_mem_map_proto_t *mmap_proto, int need_header)
{
	if (need_header)
		snor_info("--------SPI NOR flash mem map proto info--------");

	dump_spi_nor_proto(&mmap_proto->proto, 0);

	snor_info("cmd: 0x%02x", mmap_proto->cmd);
	for (int i = 0; i < SNOR_MAX_MODE_FIELD_BYTE_LEN; i++)
	{
		snor_info("mode[%d]: 0x%02x", i, mmap_proto->mode[0]);
	}
}

static void dump_spi_nor_mem_map(const spi_nor_mem_map_t *mmap, int need_header)
{
	if (need_header)
		snor_info("--------SPI NOR flash mem map info--------");

	snor_info("mem_addr: 0x%lx", mmap->mem_addr);
	snor_info("data_addr: %u(0x%08x)", mmap->data_addr, mmap->data_addr);
	snor_info("len: %u(0x%08x)", mmap->len, mmap->len);
}

static void dump_spi_nor_mem_map_cfg(const spi_nor_mem_map_cfg_t *mmap_cfg, int need_header)
{
	if (need_header)
		snor_info("--------SPI NOR flash mem map configuration--------");

	dump_spi_nor_mem_map_proto_info(&mmap_cfg->mmap_proto, 0);

	for (int i = 0; i < SNOR_MAX_MEM_MAP_CNT; i++)
	{
		snor_info("--------mem map info[%d]--------", i);
		snor_info("valid: %u", mmap_cfg->is_valid[i]);
		dump_spi_nor_mem_map(&mmap_cfg->maps[i], 0);
	}
}
#endif

static void dump_spi_nor_cfg(const spi_nor_cfg_t *cfg, int need_header)
{
	if (need_header)
		snor_info("--------SPI NOR flash configuration--------");

	snor_info("addr_mode: %u", cfg->addr_mode);
	snor_info("read_proto_type: %d", cfg->read_proto_type);
	snor_info("write_cmd: 0x%02x", cfg->write_cmd);
	snor_info("erase_4k_cmd: 0x%02x", cfg->erase_4k_cmd);
	snor_info("erase_32k_cmd: 0x%02x", cfg->erase_32k_cmd);
	snor_info("erase_64k_cmd: 0x%02x", cfg->erase_64k_cmd);

	snor_info("--------write protocol--------");
	dump_spi_nor_proto(&cfg->write_proto, 0);

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	snor_info("--------mem map cfg--------");
	dump_spi_nor_mem_map_cfg(&cfg->mmap, 0);
#endif
}

__attribute__((unused)) void dump_spi_nor_object(const spi_nor_t *nor)
{
	snor_info("----------------SPI NOR flash object info----------------");

	snor_info("------------Infomation------------");
	dump_spi_nor_info(&nor->info, 0);
	snor_info("------------Configuration------------");
	dump_spi_nor_cfg(&nor->cfg, 0);
}

int spi_nor_set_read_proto_info(spi_nor_read_proto_type_t proto_type, spi_nor_protocol_info_t *proto)
{
	if ((proto_type < 0) || (proto_type >= SNOR_READ_PROTO_TYPE_MAX))
	{
		snor_err("invalid read proto type: %d", proto_type);
		return -1;
	}

	proto->cmd_field_line_num = 1;
	switch (proto_type)
	{
		case SNOR_READ_PROTO_GENERAL:
		case SNOR_READ_PROTO_FAST:
			proto->addr_field_line_num = 1;
			proto->payload_field_line_num = 1;
			break;
		case SNOR_READ_PROTO_1_1_2:
			proto->addr_field_line_num = 1;
			proto->payload_field_line_num = 2;
			break;
		case SNOR_READ_PROTO_1_2_2:
			proto->addr_field_line_num = 2;
			proto->payload_field_line_num = 2;
			break;
		case SNOR_READ_PROTO_2_2_2:
			proto->cmd_field_line_num = 2;
			proto->addr_field_line_num = 2;
			proto->payload_field_line_num = 2;
			break;
		case SNOR_READ_PROTO_1_1_4:
			proto->addr_field_line_num = 1;
			proto->payload_field_line_num = 4;
			break;
		case SNOR_READ_PROTO_1_4_4:
			proto->addr_field_line_num = 4;
			proto->payload_field_line_num = 4;
			break;
		case SNOR_READ_PROTO_4_4_4:
			proto->cmd_field_line_num = 4;
			proto->addr_field_line_num = 4;
			proto->payload_field_line_num = 4;
			break;
		default :
			snor_warn("not support read proto type: %d", proto_type);
			break;
	}
	return 0;
}

static int spi_nor_set_read_proto_type(spi_nor_cfg_t *cfg, const spi_nor_info_t *info)
{
	int i;
	for (i = SNOR_READ_PROTO_TYPE_MAX - 1; i >=0; i--)
	{
		/* currently not support QPI */
		if (i == SNOR_READ_PROTO_4_4_4)
			continue;

		if (info->read_proto[i].proto.cmd_field_line_num != 0)
		{
			cfg->read_proto_type = i;
			return 0;
		}
	}

	return -1;
}

static void spi_nor_set_read_proto_addr_len(spi_nor_info_t *info, uint8_t addr_len)
{
	int i;
	for (i = 0; i < SNOR_READ_PROTO_TYPE_MAX; i++)
	{
		info->read_proto[i].proto.addr_field_byte_len = addr_len;
	}
}

static void spi_nor_set_default_read_proto(spi_nor_info_t *info)
{
	spi_nor_read_proto_type_t proto_type;

	proto_type = SNOR_READ_PROTO_GENERAL;
	info->read_proto[proto_type].cmd = SPINOR_CMD_READ;
	spi_nor_set_read_proto_info(proto_type, &info->read_proto[proto_type].proto);

	proto_type = SNOR_READ_PROTO_FAST;
	info->read_proto[proto_type].cmd = SPINOR_CMD_READ_FAST;
	spi_nor_set_read_proto_info(proto_type, &info->read_proto[proto_type].proto);
}

static void spi_nor_set_4byte_addr_cmd(const spi_nor_cfg_t *cfg, spi_nor_info_t *info)
{
	spi_nor_read_proto_type_t proto_type;
	spi_nor_rw_proto_t *read_proto;
	uint8_t cmd;

	if (cfg->addr_mode != 4)
		return;

	for (proto_type = 0; proto_type < SNOR_READ_PROTO_TYPE_MAX; proto_type++)
	{
		/* currently not support QPI */
		if (proto_type == SNOR_READ_PROTO_4_4_4)
			continue;

		read_proto = &info->read_proto[proto_type];
		if (!read_proto->proto.cmd_field_line_num)
		{
			continue;
		}

		switch (proto_type)
		{
			case SNOR_READ_PROTO_GENERAL:
				cmd = SPINOR_CMD_READ_4B;
				break;
			case SNOR_READ_PROTO_FAST:
				cmd = SPINOR_CMD_READ_FAST_4B;
				break;
			case SNOR_READ_PROTO_1_1_2:
				cmd = SPINOR_CMD_READ_1_1_2_4B;
				break;
			case SNOR_READ_PROTO_1_2_2:
				cmd = SPINOR_CMD_READ_1_2_2_4B;
				break;
			case SNOR_READ_PROTO_2_2_2:
				cmd = 0;
				read_proto->proto.cmd_field_line_num = 0;
				break;
			case SNOR_READ_PROTO_1_1_4:
				cmd = SPINOR_CMD_READ_1_1_4_4B;
				break;
			case SNOR_READ_PROTO_1_4_4:
				cmd = SPINOR_CMD_READ_1_4_4_4B;
				break;
			case SNOR_READ_PROTO_4_4_4:
				cmd = 0;
				read_proto->proto.cmd_field_line_num = 0;
				break;
			default :
				cmd = 0;
				snor_warn("not support read proto type: %d, %u-%u-%u",
						  proto_type,
						  read_proto->proto.cmd_field_line_num,
						  read_proto->proto.addr_field_line_num,
						  read_proto->proto.payload_field_line_num);
				break;
		}
		read_proto->cmd = cmd;
	}
}

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
static void spi_nor_set_mem_map_proto(spi_nor_t *nor)
{
	nor->cfg.mmap.mmap_proto.cmd = nor->info.read_proto[nor->cfg.read_proto_type].cmd;
	memset(&nor->cfg.mmap.mmap_proto.mode, 0, sizeof(nor->cfg.mmap.mmap_proto.mode));

	memcpy(&nor->cfg.mmap.mmap_proto.proto, &nor->info.read_proto[nor->cfg.read_proto_type].proto, sizeof(nor->cfg.mmap.mmap_proto.proto));
}
#endif

static void spi_nor_set_write_proto(spi_nor_cfg_t *cfg)
{
	/*
	 * Because the write speed of SPI NOR flash is very slow than
	 * SPI clock frequency, even though we use Quad page program
	 * command, the write speed can't improve. So we default use
	 * the Standard SPI protocol to program page.
	 */
	cfg->write_proto.addr_field_byte_len = cfg->addr_mode;
	cfg->write_proto.mode_clocks_num = 0;
	cfg->write_proto.dummy_clocks_num = 0;
	cfg->write_proto.cmd_field_line_num = 1;
	cfg->write_proto.addr_field_line_num = 1;
	cfg->write_proto.payload_field_line_num = 1;

	cfg->write_cmd = SPINOR_CMD_PP;
	if (cfg->addr_mode == 4)
	{
		cfg->write_cmd = SPINOR_CMD_PP_4B;
	}
}

static void spi_nor_set_erase_cmd(spi_nor_cfg_t *cfg)
{
	cfg->erase_4k_cmd = SPINOR_CMD_BE_4K;
	cfg->erase_32k_cmd = SPINOR_CMD_BE_32K;
	cfg->erase_64k_cmd = SPINOR_CMD_BE_64K;

	if (cfg->addr_mode == 4)
	{
		cfg->erase_4k_cmd = SPINOR_CMD_BE_4K_4B;
		cfg->erase_32k_cmd = SPINOR_CMD_BE_32K_4B;
		cfg->erase_64k_cmd = SPINOR_CMD_BE_64K_4B;
	}
}

static void spi_nor_set_default_hwcaps(spi_nor_info_t *info)
{
	info->hwcaps |= SNOR_HWCAPS_READ;
	info->hwcaps |= SNOR_HWCAPS_READ_FAST;
}

static int spi_nor_set_addr_mode(spi_nor_t *nor)
{
	if (!(nor->info.hwcaps & SNOR_HWCAPS_3BYTE_ADDR) &&
			!(nor->info.hwcaps & SNOR_HWCAPS_4BYTE_ADDR))
	{
		snor_err("flash support neither 3 byte addr nor 4 byte addr! hwcaps: 0x%08x",
				 nor->info.hwcaps);
		return -1;
	}

	if ((nor->info.total_size > SZ_16M) &&
			!(nor->info.hwcaps & SNOR_HWCAPS_4BYTE_ADDR))

	{
		snor_err("flash size(%uB) is great than 16M but not support 4 byte addr, "
				 "hwcaps: 0x%08x",
				 nor->info.total_size, nor->info.hwcaps);
		return -2;
	}

	if (nor->info.hwcaps & SNOR_HWCAPS_3BYTE_ADDR)
		nor->cfg.addr_mode = 3;

	if ((nor->info.total_size > SZ_16M) &&
			(nor->info.hwcaps & SNOR_HWCAPS_4BYTE_ADDR))
		nor->cfg.addr_mode = 4;

	/*
	 * FIXME: if the flash only support 4 byte addr, whether the normal command
	 * which used in 3 byte addr mode is force to use 4 byte addr?
	 */
	return 0;
}

static int spi_nor_scan(spi_nor_t *nor)
{
	int ret = 0;

	memset(&nor->info, 0, sizeof(nor->info));
	memset(&nor->cfg, 0, sizeof(nor->cfg));

	ret = spi_nor_read_jedec_id(nor, nor->info.id, sizeof(nor->info.id));
	if (ret)
	{
		snor_err("spi_nor_read_jedec_id failed, ret: %d", ret);
		return -1;
	}

	nor->info.model = "SFDP compatible";
	ret = spi_nor_parse_sfdp(nor);
	if (ret)
	{
		snor_err("spi_nor_parse_sfdp failed, ret: %d", ret);
		return -2;
	}

	ret = spi_nor_set_addr_mode(nor);
	if (ret)
	{
		return -3;
	}

	spi_nor_set_default_hwcaps(&nor->info);
	spi_nor_set_default_read_proto(&nor->info);

	spi_nor_set_read_proto_addr_len(&nor->info, nor->cfg.addr_mode);
	spi_nor_set_4byte_addr_cmd(&nor->cfg, &nor->info);

	ret = spi_nor_set_read_proto_type(&nor->cfg, &nor->info);
	if (ret)
	{
		snor_err("spi_nor_set_read_opr_type failed, ret: %d", ret);
		return -2;
	}

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	spi_nor_set_mem_map_proto(nor);
#endif

	spi_nor_set_write_proto(&nor->cfg);
	spi_nor_set_erase_cmd(&nor->cfg);

	return 0;
}

static int spi_nor_controller_init(const spi_nor_controller_t *controller)
{
	int ret = 0;

	const spi_nor_controller_ops_t *ops = controller->ops;
	if (!ops->init)
	{
		snor_err("there is no init ops!");
		return -1;
	}

	if (!ops->exec_opr)
	{
		snor_err("there is no exec_opr ops!");
		return -2;
	}

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	if (!ops->set_output_clk_freq)
	{
		snor_err("there is no set_output_clk_freq ops!");
		return -3;
	}
#endif

	ret = ops->init(controller->id, CONFIG_READ_SFDP_CLK_FREQ);
	if (ret)
	{
		snor_err("controller init failed, ret: %d", ret);
		return -4;
	}

	return 0;
}

int spi_nor_init(spi_nor_t *nor)
{
	int ret = 0;

	if (!hal_mutex_lock(&nor->lock))
	{
		spi_nor_unlock(nor);
		snor_info("spi nor flash has been already inited, nor: 0x%p", nor);
		return 0;
	}

	ret = spi_nor_lock_init(nor);
	if (ret)
		goto exit;

	ret = spi_nor_lock(nor);
	if (ret)
		goto exit;

	ret = spi_nor_controller_init(nor->controller);
	if (ret)
		goto unlock;

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	snor_info("Reset SPI NOR flash");
	ret = spi_nor_reset(nor);
	if (ret)
		goto unlock;
#endif

	snor_info("Scan SPI NOR flash");
	ret = spi_nor_scan(nor);
	if (ret)
		goto unlock;

#ifdef DUMP_SPI_NOR_OBJECT_AFTER_SCAN
	dump_spi_nor_object(nor);
#endif

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	ret = spi_nor_set_clk_freq(nor, CONFIG_SPI_NOR_RW_CLK_FREQ);
	if (ret)
		snor_warn("spi_nor_set_clk_freq failed, ret: %d", ret);
#endif

	if (!nor->info.model)
		nor->info.model = "unknown";

	snor_info("SPI NOR flash found, JEDEC ID: %02x %02x %02x",
			  nor->info.id[0], nor->info.id[1], nor->info.id[2]);

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	ret = spi_nor_setup_mem_map_proto(nor, &nor->cfg.mmap.mmap_proto);
	if (ret)
	{
		snor_err("spi_nor_setup_mem_map_proto failed, ret: %d", ret);
		goto unlock;
	}
#endif

	spi_nor_protocol_info_t *read_proto = &nor->info.read_proto[nor->cfg.read_proto_type].proto;
	snor_info("SPI NOR flash other info, model: '%s', size: %uMB, minimal block: %uKB, page: %uB, "
			  "read: %d-%d-%d, write: %d-%d-%d, ",
			  nor->info.model, nor->info.total_size / 1024 / 1024, nor->info.blk_size / 1024,
			  nor->info.page_size,
			  read_proto->cmd_field_line_num,
			  read_proto->addr_field_line_num,
			  read_proto->payload_field_line_num,
			  nor->cfg.write_proto.cmd_field_line_num,
			  nor->cfg.write_proto.addr_field_line_num,
			  nor->cfg.write_proto.payload_field_line_num);

#ifdef SPI_NOR_FLASH_PM_SUPPORT
	memset(&nor->pm_dev, 0, sizeof(nor->pm_dev));
	nor->pm_dev.name = "SPI NOR Flash";
	nor->pm_dev.ops = &s_nor_pm_devops;
	nor->pm_dev.data = nor;
	ret = pm_devops_register(&nor->pm_dev);
	if (ret)
		snor_warn("pm_devops_register failed, ret: %d", ret);
#endif

	ret = 0;

unlock:
	spi_nor_unlock(nor);

exit:
	if (ret)
		snor_err("SPI NOR flash init failed!");
	else
		snor_info("SPI NOR flash init success!");

	return ret;
}


#ifdef SPI_NOR_FLASH_PM_SUPPORT
static int spi_nor_suspend(struct pm_device *dev, suspend_mode_t state)
{
	int ret = 0;
	spi_nor_t *nor = dev->data;

	snor_info("spi nor flash(0x%p) suspend", nor);
	switch (state)
	{
		case PM_MODE_SLEEP:
		case PM_MODE_STANDBY:
		case PM_MODE_HIBERNATION:
			ret = spi_nor_enter_deep_power_down_mode(nor);
			if (ret)
				snor_err("spi_nor_enter_deep_power_down_mode failed, ret: %d", ret);
			break;
		default:
			snor_err("invalid pm state: %d", state);
			break;
	}

	return 0;
}

static int spi_nor_resume(struct pm_device *dev, suspend_mode_t state)
{
	int ret = 0;
	spi_nor_t *nor = dev->data;

	snor_info("spi nor flash(0x%p) resume", nor);
	switch (state)
	{
		case PM_MODE_SLEEP:
		case PM_MODE_STANDBY:
		case PM_MODE_HIBERNATION:
			ret = spi_nor_exit_deep_power_down_mode(nor);
			if (ret)
				snor_err("spi_nor_exit_deep_power_down_mode failed, ret: %d", ret);

			if (state == PM_MODE_SLEEP)
				break;

			ret = spi_nor_recover_mem_map_when_resume(nor);
			if (ret)
				snor_err("spi_nor_recover_mem_map_when_resume failed, ret: %d", ret);
			break;
		default:
			snor_err("invalid pm state: %d", state);
			break;
	}

	return 0;
}

static struct pm_devops s_nor_pm_devops =
{
	.suspend_noirq = spi_nor_suspend,
	.resume_noirq = spi_nor_resume,
};
#endif
