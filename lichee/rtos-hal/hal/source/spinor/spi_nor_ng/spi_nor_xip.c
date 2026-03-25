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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sunxi_hal_common.h>
#include <sunxi_hal_spinor.h>

#if defined(CONFIG_XIP_NEED_INIT)
#include <blkpart.h>
#include <gpt_part.h>


#define MAX_XIP_CFG_BUF_LEN 128

#define DUMP_XIP_PART_DATA

#ifdef DUMP_XIP_PART_DATA
#include <awlog.h>
#endif

#ifdef CONFIG_COMPONENTS_AW_BLKPART

static __attribute__((unused)) int read_gpt_data(uint8_t **buf)
{
	int ret = 0;
	uint8_t *gpt_buf;
	uint32_t buf_len = GPT_TABLE_SIZE;

	gpt_buf = malloc(buf_len);
	if (!gpt_buf)
	{
		return -ENOMEM;
	}

	memset(gpt_buf, 0, buf_len);
	ret = nor_read(GPT_ADDRESS, (char *)gpt_buf, buf_len);
	if (ret)
	{
		printf("read gpt data from nor flash failed - %d\n", ret);
		ret = -EIO;
		goto exit;
	}

	*buf = gpt_buf;

exit:
	if (ret)
		free(gpt_buf);
	return ret;
}
#endif

#define MAX_PART_NAME_LEN 16
typedef struct xip_cfg
{
	unsigned long mem_addr;
	uint32_t len;
	char part_name[MAX_PART_NAME_LEN + 1];
} xip_cfg_t;

int get_xip_part_info(uint8_t *gpt_buf, const char *xip_part_name, uint32_t *xip_part_addr, uint32_t *xip_part_size)
{
	int ret;
	unsigned int start_sector;
	unsigned int sectors;

	ret = get_part_info_by_name(gpt_buf, (char *)xip_part_name, &start_sector, &sectors);
	if (ret)
	{
		return -EIO;
	}

	*xip_part_addr = start_sector << SECTOR_SHIFT;
	*xip_part_size = sectors << SECTOR_SHIFT;

	return 0;
}

static int get_xip_data_info(uint8_t *gpt_buf, const char *xip_part_name, uint32_t *xip_data_addr, uint32_t *xip_data_len)
{
	int ret = 0;
	uint32_t xip_part_addr = 0, xip_part_size;

	ret = get_xip_part_info(gpt_buf, xip_part_name, &xip_part_addr, &xip_part_size);
	if (ret)
	{
		printf("get_xip_part_info failed, ret: %d\n", ret);
		return -1;
	}

	printf("XIP partition addr: 0x%x, size: 0x%x\n", xip_part_addr, xip_part_size);

#define IH_SIZE 128

	uint32_t buf_len = IH_SIZE;
	char *ih_buf = malloc(buf_len);
	if (ih_buf == NULL)
	{
		printf("malloc failed\n");
		return -1;
	}

	memset(ih_buf, 0, buf_len);
	ret = nor_read(xip_part_addr, ih_buf, buf_len);
	if (ret)
	{
		printf("read XIP partition data failed, ret: %d\n", ret);
		ret = -2;
		goto free_ih_buf;
	}

	if (memcmp(ih_buf, "AWIH", 4) == 0)
	{
		printf("Found RTOS image header in XIP partition\n");
		xip_part_addr += IH_SIZE;
		xip_part_size -= IH_SIZE;
	}

#ifdef DUMP_XIP_PART_DATA
	aw_hexdump((char *)ih_buf, IH_SIZE);
#endif

	*xip_data_addr = xip_part_addr;
	*xip_data_len = xip_part_size;

free_ih_buf:
	if (ih_buf)
		free(ih_buf);

	return ret;
}

static int parse_addr(const char *arg, unsigned long *addr)
{
	char *ptr = NULL;
	errno = 0;
	*addr = strtol(arg, &ptr, 0);
	if ((errno != 0) || (*ptr != '\0') || (ptr == arg))
	{
		printf("addr: '%s' is invalid\n", arg);
		//printf("errno: %d, *ptr: '%c', ptr: %p\n", errno, *ptr, ptr);
		return -1;
	}
	return 0;
}

#define SEPARATOR_CHAR1 ':'
#define SEPARATOR_CHAR2 '@'

int __parse_cfg(const char *str, xip_cfg_t *cfg)
{
	int index = 0, ret;
	if (!str)
		return -1;

	while (str[index])
	{
		if (str[index] == SEPARATOR_CHAR2)
		{
			if (index == 0)
			{
				return -2;
			}
			if (index > (MAX_PART_NAME_LEN))
			{
				return -3;
			}

			strncpy(cfg->part_name, str, index);
			cfg->part_name[index] = '\0';
			index++;
			ret = parse_addr(&str[index], &cfg->mem_addr);
			if (ret)
				return -4;

			return 0;
		}
		index++;
	}

	return -5;
}


int parse_cfg(char *str, xip_cfg_t *cfg, uint32_t cfg_num)
{
	int start = 0, index = 0, ret;
	uint32_t cfg_index = 0;
	if (!str)
		return -1;

	while (str[index])
	{
		if (str[index] == SEPARATOR_CHAR1)
		{
			if (index == 0)
			{
				start = 1;
				continue;
			}

			if (cfg_index >= cfg_num)
			{
				printf("warning:  cfg_index: %d, cfg_num: %u\n", cfg_index, cfg_num);
				return 0;
			}

			str[index] = '\0';
			ret = __parse_cfg(&str[start], &cfg[cfg_index]);
			if (ret)
			{
				printf("__parse_cfg failed, ret: %d\n", ret);
				continue;
			}
			printf("name: '%s'\n", cfg[cfg_index].part_name);
			printf("addr: 0x%08lx\n", cfg[cfg_index].mem_addr);

			start=index + 1;
			cfg_index++;
		}
		index++;
	}

	if (start != index)
	{
		if (cfg_index > cfg_num)
		{
			return 0;
		}

		ret = __parse_cfg(&str[start], &cfg[cfg_index]);
		if (ret)
		{
			printf("__parse_cfg failed, ret: %d\n", ret);
			return -3;
		}
		printf("name: '%s'\n", cfg[cfg_index].part_name);
		printf("addr: 0x%08lx\n", cfg[cfg_index].mem_addr);

		cfg_index++;
	}
	return 0;
}

extern int get_xip_env(const char *env_name, unsigned char *gpt_buf, char *cfg_buf, uint32_t cfg_buf_len);

static int parse_xip_cfg(uint8_t *gpt_buf, xip_cfg_t *cfg, uint32_t cfg_num)
{
	int ret = 0;
	if (!gpt_buf)
		return -1;

	char cfg_buf[MAX_XIP_CFG_BUF_LEN];
	memset(cfg_buf, 0, MAX_XIP_CFG_BUF_LEN);

	ret = get_xip_env(CONFIG_XIP_CFG_ENV_NAME, gpt_buf, cfg_buf, MAX_XIP_CFG_BUF_LEN);
	if (ret)
	{
		printf("get_xip_env failed, ret: %d\n", ret);
		ret = -2;
		goto exit;
	}

	printf("XIP env: '%s'\n", cfg_buf);

	ret = parse_cfg(cfg_buf, cfg, cfg_num);
	if (ret)
	{
		printf("parse_cfg failed, ret: %d\n", ret);
		ret = -2;
		goto exit;

	}

exit:
	return ret;
}

int spi_nor_xip_init(spi_nor_t *nor)
{
	int ret = 0, i;
	xip_cfg_t *xip = NULL;
	uint32_t xip_data_addr = 0, xip_data_len = 0;
	uint8_t *gpt_buf = NULL;

	ret = read_gpt_data(&gpt_buf);
	if (ret)
		return -1;

	xip_cfg_t xip_cfg[3];
	ret = parse_xip_cfg(gpt_buf, xip_cfg, 3);
	if (ret)
	{
		printf("parse_xip_cfg failed, ret: %d\n", ret);
		ret = -2;
		goto exit;
	}

	for (i = 0; i < ARRAY_SIZE(xip_cfg); i++)
	{
		xip = &xip_cfg[i];
		printf("Mapping the XIP partition '%s' to 0x%p\n", xip->part_name, (void *)xip->mem_addr);
		ret = get_xip_data_info(gpt_buf, xip->part_name, &xip_data_addr, &xip_data_len);
		if (ret)
		{
			printf("get_xip_data_info failed, part_name: '%s', ret: %d\n", xip->part_name, ret);
			continue;
		}
		printf("XIP data addr: 0x%x, len: 0x%x\n", xip_data_addr, xip_data_len);

		ret = spi_nor_add_mem_map(nor, xip->mem_addr, xip_data_len, xip_data_addr);
		if (ret)
		{
			printf("spi_nor_add_mem_map failed, mem_addr: 0x%lx, ret: %d\n", xip->mem_addr, ret);
			continue;
		}
	}

exit:
	free(gpt_buf);
	return ret;
}
#endif

