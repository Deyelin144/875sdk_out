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

//#include <sunxi_hal_common.h>
#include <sunxi_hal_spinor.h>

#include <blkpart.h>
#include <part_efi.h>

static struct blkpart norblk;

struct syspart
{
	char name[MAX_BLKNAME_LEN];
	u32 bytes;
};

static const struct syspart s_sys_parts[] =
{
#ifdef CONFIG_ARCH_SUN20IW2
	{"boot0", 64 * 1024},
	{"boot0-bk", 64 * 1024},
	{"gpt", 16 * 1024},
#else
	/* contain boot0 and gpt, gpt offset is (128-16)k */
	{"boot0", CONFIG_COMPONENTS_AW_BLKPART_LOGICAL_OFFSET * 1024},
#endif
};

int spi_nor_blkpart_init(const spi_nor_t *nor)
{
	int ret, index;
	char *gpt_buf;

	struct gpt_part *gpt_part;
	struct part *part;

	gpt_buf = malloc(GPT_TABLE_SIZE);
	if (!gpt_buf)
	{
		ret = -ENOMEM;
		goto err;
	}
	memset(gpt_buf, 0, GPT_TABLE_SIZE);

	ret = nor_read(GPT_ADDRESS, gpt_buf, GPT_TABLE_SIZE);
	if (ret)
	{
		printf("get gpt from nor flash failed - %d\n", ret);
		goto err;
	}

	memset(&norblk, 0, sizeof(struct blkpart));
	norblk.name = "nor";
	norblk.page_bytes = nor->info.page_size;

	norblk.erase = nor_erase;
	norblk.program = nor_write;
	norblk.read = nor_read;
	norblk.sync = NULL;

	norblk.noncache_erase = nor_erase;
	norblk.noncache_program = nor_write;
	norblk.noncache_read = nor_read;
	norblk.total_bytes = nor->info.total_size;
	norblk.blk_bytes = nor->info.blk_size;

	ret = gpt_part_cnt(gpt_buf);
	if (ret < 0)
	{
		printf("get part count from gpt failed\n");
#ifdef CONFIG_COMPONENTS_AW_BLKPART_NO_GPT
		printf("when no gpt, hardcode 2 part, 0-2M:rtos 2M-end:UDISK\n");
		norblk.n_parts = 2;
		norblk.parts = malloc(norblk.n_parts * sizeof(struct part));
		part = &norblk.parts[0];
		snprintf(part->name, MAX_BLKNAME_LEN, "%s", "rtos");
		part->bytes = 2*1024*1024;
		part->off = 0;
		part = &norblk.parts[1];
		part->bytes = norblk.total_bytes - 2*1024*1024;
		part->off = 2*1024*1024;
		snprintf(part->name, MAX_BLKNAME_LEN, "%s", "UDISK");
		ret = add_blkpart(&norblk);
		if (ret)
			goto free_parts;
		return 0;
#else
		goto err;
#endif
	}
#ifdef CONFIG_RESERVE_IMAGE_PART
	norblk.n_parts = ret + ARRAY_SIZE(s_sys_parts) + 2;
#else
	norblk.n_parts = ret + ARRAY_SIZE(s_sys_parts);
#endif
	norblk.parts = malloc(norblk.n_parts * sizeof(struct part));
	if (!norblk.parts)
		goto err;
	printf("total %u part\n", norblk.n_parts);

	for (index = 0; index < ARRAY_SIZE(s_sys_parts); index++)
	{
		part = &norblk.parts[index];
		part->bytes = s_sys_parts[index].bytes;
		part->off = BLKPART_OFF_APPEND;
		strcpy(part->name, s_sys_parts[index].name);
	}

	foreach_gpt_part(gpt_buf, gpt_part)
	{
		part = &norblk.parts[index++];
		part->bytes = gpt_part->sects << SECTOR_SHIFT;
		part->off = BLKPART_OFF_APPEND;
		//snprintf(part->name, MAX_BLKNAME_LEN, "%s", gpt_part->name);
		strncpy(part->name, gpt_part->name, MAX_BLKNAME_LEN - 1);
		part->name[MAX_BLKNAME_LEN - 1] = '\0';
#ifdef CONFIG_RESERVE_IMAGE_PART
		if (!strcmp("rtosA", part->name) || !strcmp("rtosB", part->name))
		{
			int rtos_index = index - 1;
			struct part *last_part = part;
			int toc_package_size = get_rtos_toc_package_size(gpt_buf, last_part->name, norblk.page_bytes);
			int rtos_offset = get_rtos_offset(gpt_buf, last_part->name);
			if (toc_package_size > 0 && rtos_offset > 0)
			{
				part = &norblk.parts[index++];
				part->bytes = norblk.parts[rtos_index].bytes - toc_package_size;
				part->off = rtos_offset + toc_package_size;
				if (!strcmp("rtosA", last_part->name))
					snprintf(part->name, MAX_BLKNAME_LEN, "%s", "reserveA");
				else
					snprintf(part->name, MAX_BLKNAME_LEN, "%s", "reserveB");
			}
			else
			{
				norblk.n_parts --;
			}
		}
#endif
	}
	norblk.parts[--index].bytes = BLKPART_SIZ_FULL;

	ret = add_blkpart(&norblk);
	if (ret)
		goto free_parts;

#ifdef CONFIG_COMPONENTS_PSTORE
	{
		int pstore_init(uint32_t addr, uint32_t size);
		struct part *pstore = get_part_by_name("pstore");
		if (pstore)
		{
			uint32_t off = (uint32_t)(pstore->off & 0xFFFFFFFF);
			uint32_t bytes = (uint32_t)(pstore->bytes & 0xFFFFFFFF);
			pstore_init(off, bytes);
		}
	}
#endif

	/* check bytes align */
	for (index = 0; index < norblk.n_parts; index++)
	{
		part = &norblk.parts[index];
		if (part->bytes % nor->info.blk_size)
		{
			printf("part %s with bytes %llu should align to block size %u\n",
				   part->name, part->bytes, nor->info.blk_size);
			goto del_blk;
		}
	}

	free(gpt_buf);
	return 0;

del_blk:
	del_blkpart(&norblk);
free_parts:
	free(norblk.parts);
err:
	free(gpt_buf);
	printf("init blkpart for nor failed - %d\n", ret);
	return ret;
}
