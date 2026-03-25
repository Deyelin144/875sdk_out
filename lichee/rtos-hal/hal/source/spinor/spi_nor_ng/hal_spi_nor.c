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
#include <hal_time.h>

#ifdef CONFIG_COMPONENTS_AW_BLKPART
#include <blkpart.h>
#include <gpt_part.h>
#endif

//#define DUMP_SPI_NOR_OBJECT_AFTER_XIP_INIT

static spi_nor_t s_spi_nor[CONFIG_MAX_SPI_NOR_FLASH_NUM];

static spi_nor_t *get_spi_nor_object(unsigned int id)
{
	if (id >= CONFIG_MAX_SPI_NOR_FLASH_NUM)
		return NULL;

	return &s_spi_nor[id];
}

int hal_spi_nor_read(unsigned int id, unsigned int addr, unsigned char *buf, unsigned int len)
{
	spi_nor_t *nor;
	nor = get_spi_nor_object(id);
	if (!nor)
	{
		return -1;
	}

	return spi_nor_read(nor, addr, len, buf);
}

int hal_spi_nor_write(unsigned int id, unsigned int addr, unsigned char *buf, unsigned int len)
{
	spi_nor_t *nor;
	nor = get_spi_nor_object(id);
	if (!nor)
	{
		return -1;
	}

	return spi_nor_write(nor, addr, len, buf);
}

int hal_spi_nor_erase(unsigned int id, unsigned int addr, unsigned int len)
{
	spi_nor_t *nor;
	nor = get_spi_nor_object(id);
	if (!nor)
	{
		return -1;
	}

	return spi_nor_erase(nor, addr, len);
}

extern int spi_nor_blkpart_init(const spi_nor_t *nor);
extern int spi_nor_xip_init(spi_nor_t *nor);

extern spi_nor_controller_t g_fc_controller;

int hal_spi_nor_init(unsigned int id)
{
	int ret = 0;

	spi_nor_t *nor;
	nor = get_spi_nor_object(id);
	if (!nor)
	{
		return -1;
	}

	nor->controller = &g_fc_controller;

	ret = spi_nor_init(nor);
	if (ret < 0)
		return -2;

#ifdef CONFIG_XIP_NEED_INIT
	ret = spi_nor_xip_init(nor);
	if (ret)
	{
		printf("spi_nor_xip_init failed, ret: %d\n", ret);
		return -3;
	}
#endif

#ifdef DUMP_SPI_NOR_OBJECT_AFTER_XIP_INIT
	dump_spi_nor_object(nor);
#endif

#ifdef CONFIG_ACCESS_SPI_NOR_BY_BLKPART
#if defined(CONFIG_ARCH_SUN20IW2) && defined(CONFIG_ARCH_RISCV)
	ret = spi_nor_blkpart_init(nor);
	if (ret)
	{
		printf("nor_blkpart_init failed, ret: %d\n", ret);
		return -3;
	}
#endif
#endif

	return 0;
}

/*
 * In order to be compatible with code which use legacy APIs,
 * provide these legacy APIs below, don't use it in new code!!!
 */
int nor_read(unsigned int addr, char *buf, unsigned int len)
{
	return hal_spi_nor_read(0, addr, (unsigned char *)buf, len);
}

int nor_write(unsigned int addr, char *buf, unsigned int len)
{
	return hal_spi_nor_write(0, addr, (unsigned char *)buf, len);
}

int nor_erase(unsigned int addr, unsigned int len)
{
	return hal_spi_nor_erase(0, addr, len);
}
int nor_init(void)
{
	return hal_spi_nor_init(0);
}

int32_t hal_spinor_read_data(uint32_t addr, void *buf, uint32_t cnt)
{
	return hal_spi_nor_read(0, addr, (unsigned char *)buf, cnt);
}

int32_t hal_spinor_panic_read_data(uint32_t addr, void *buf, uint32_t cnt)
{
	return hal_spi_nor_read(0, addr, (unsigned char *)buf, cnt);
}

int32_t hal_spinor_panic_program_data(uint32_t addr, const void *buf, uint32_t cnt)
{
	return hal_spi_nor_write(0, addr, (unsigned char *)buf, cnt);
}

int32_t hal_spinor_panic_erase_sector(uint32_t addr, uint32_t size)
{
	return hal_spi_nor_erase(0, addr, size);
}
