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

#include <xtensa/hal.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/config/core.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/xtruntime.h>

#include <platform.h>
#include <irqs.h>
#include <aw_io.h>
#include <console.h>
#include <string.h>
#include <stdio.h>
#include "spinlock.h"
#include <sunxi_hal_common.h>
#ifdef CONFIG_DRIVERS_SOUND
#include <snd_core.h>
#endif
#ifdef CONFIG_DRIVERS_SOUND_V2
extern int sunxi_sound_card_initialize(void);
#endif
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
#include <AudioSystem.h>
#endif
#if defined(CONFIG_DRIVERS_CCMU) || defined(CONFIG_DRIVERS_CCU)
#include <hal_clk.h>
#endif
#ifdef CONFIG_COMPONENTS_PM
#include <pm_init.h>
#endif

#ifdef CONFIG_COMPONENTS_XTENSA_HIFI5_NNLIB_LIBRARY
#include <xa_nnlib_standards.h>
#endif

#ifdef CONFIG_COMPONENTS_XTENSA_HIFI5_VFPU_LIBRARY
#include <NatureDSP_Signal_id.h>
#endif

#ifdef CONFIG_COMPONENTS_OMX_SYSTEM
#include <rpdata_common_interface.h>
#endif

#ifdef CONFIG_DRIVERS_SPI_NOR_NG
#include <sunxi_hal_spinor.h>
#endif

#include "versioninfo.h"

extern int amp_init(void);

int32_t console_uart = UART_UNVALID;

void cache_config(void)
{
	/* 0x0~0x20000000-1 is cacheable */
	xthal_set_region_attribute((void *)0x00000000, 0x20000000, XCHAL_CA_WRITEBACK, 0);

	/* 0x20000000~0x40000000-1 is non-cacheable */
	xthal_set_region_attribute((void *)0x20000000, 0x40000000, XCHAL_CA_BYPASS, 0);

	/* 0x4000000~0x80000000-1 is non-cacheable */
	xthal_set_region_attribute((void *)0x40000000, 0x40000000, XCHAL_CA_BYPASS, 0);

	/* 0x80000000~0xC0000000-1 is non-cacheable */
	xthal_set_region_attribute((void *)0x80000000, 0x40000000, XCHAL_CA_BYPASS, 0);

	/* 0xC0000000~0xFFFFFFFF is  non-cacheable */
	xthal_set_region_attribute((void *)0xC0000000, 0x40000000, XCHAL_CA_BYPASS, 0);

	/* set prefetch level */
	xthal_set_cache_prefetch(XTHAL_PREFETCH_BLOCKS(8) |XTHAL_DCACHE_PREFETCH_HIGH | XTHAL_ICACHE_PREFETCH_HIGH |XTHAL_DCACHE_PREFETCH_L1);
}

// #define CONFIG_DEFAULT_DSP_CORE_CLK_FREQ 320000000
static uint32_t s_dsp_core_clk_freq = 0;

static int get_dsp_core_clk_freq(uint32_t *dsp_clk_freq)
{
#if defined(CONFIG_DRIVERS_CCMU) || defined(CONFIG_DRIVERS_CCU)
	hal_clk_t dsp_div_clk;
	dsp_div_clk = hal_clock_get(HAL_SUNXI_CCU, CLK_DSP_DIV);
	if (!dsp_div_clk)
		return -1;

	*dsp_clk_freq = hal_clk_recalc_rate(dsp_div_clk);
	return 0;
#else
	printf("Warning: no CCMU driver support, use default frequency!\n");
	*dsp_clk_freq = CONFIG_DEFAULT_DSP_CORE_CLK_FREQ;
	return 0;
#endif
}

unsigned int xtbsp_clock_freq_hz(void)
{
	if (s_dsp_core_clk_freq)
		return s_dsp_core_clk_freq;

	uint32_t dsp_clk_freq = 0;
	int ret = get_dsp_core_clk_freq(&dsp_clk_freq);
	if (ret)
	{
		printf("get_dsp_core_clk_freq failed, ret: %d\n", ret);
		dsp_clk_freq = CONFIG_DEFAULT_DSP_CORE_CLK_FREQ;
	}

	if (!dsp_clk_freq)
	{
		printf("Warning: invalid DSP core clock frequency, use default frequency!\n");
		dsp_clk_freq = CONFIG_DEFAULT_DSP_CORE_CLK_FREQ;
	}

	s_dsp_core_clk_freq = dsp_clk_freq;

	return s_dsp_core_clk_freq;
}

void print_banner(void)
{
	/* remove, print banner after start schedule */
}

char *dsp_get_version(void)
{
    return FIRMWARE_VERSION;
}

void print_banner_after_schedule(void)
{
	printf("\r\n"
	       " *******************************************\r\n"
	       " **     Welcome to DSP FreeRTOS           **\r\n"
	       " ** Copyright (C) 2019-2022 AllwinnerTech **\r\n"
	       " **                                       **\r\n"
	       " **      starting xtensa FreeRTOS V1.1    **\r\n"
	       " **    Date:%s, Time:%s    **\r\n"
	       " *******************************************\r\n"
	       "\r\n", __DATE__, __TIME__);
}

void app_init(void)
{
	printf("DSP core clock frequency: %uHz\n", s_dsp_core_clk_freq);
	print_banner_after_schedule();

#ifdef CONFIG_COMPONENTS_XTENSA_HIFI5_NNLIB_LIBRARY
	const char *name = xa_nnlib_get_lib_name_string();
	const char *aver = xa_nnlib_get_lib_api_version_string();
	printf("Name:%s API version:%s \n",name,aver);
#endif

#ifdef CONFIG_COMPONENTS_XTENSA_HIFI5_VFPU_LIBRARY
	char nature_lib_ver[32] = {0};
	char natrre_api_ver[32] = {0};
	NatureDSP_Signal_get_library_version(nature_lib_ver);
	NatureDSP_Signal_get_library_api_version(natrre_api_ver);
	printf("VFPU lib version:%s  API version:%s\n",nature_lib_ver,natrre_api_ver);
#endif

#ifdef CONFIG_COMPONENTS_AMP
	amp_init();
#endif
#ifdef CONFIG_COMPONENTS_PM
	pm_init(1, NULL);
#endif

#ifdef CONFIG_DRIVERS_SPI_NOR_NG
	hal_spi_nor_init(0);
#endif

#ifdef CONFIG_DRIVERS_SOUND
	sunxi_soundcard_init();
#endif

#ifdef CONFIG_DRIVERS_SOUND_V2
	sunxi_sound_card_initialize();
#endif

#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
	AudioSystemInit();
#endif
#ifdef CONFIG_COMPONENTS_OMX_SYSTEM
	rpdata_ctrl_init();
#endif

}

