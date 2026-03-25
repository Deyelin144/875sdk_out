/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECqHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "interrupt.h"
#include <portmacro.h>
#include <cli_console.h>
#include <aw_version.h>
#include <fcntl.h>
#include "versioninfo.h"

#ifdef CONFIG_COMPONENTS_AW_DEVFS
#include <devfs.h>
#endif

#ifdef CONFIG_COMPONENT_LITTLEFS
#include <littlefs.h>
#endif

#ifdef CONFIG_DRIVERS_MSGBOX
#include <hal_msgbox.h>
#endif

#ifdef CONFIG_COMPONENT_ULOG
#include <ulog.h>
#endif

#ifdef CONFIG_DRIVERS_XRADIO
#ifdef CONFIG_FREQ_OFFSET_FROM_SDD
#include <sdd.h>
#include <hal_clk.h>
#endif
#endif

#include "FreeRTOS.h"
#include "task.h"
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
#include <AudioSystem.h>
#endif
#ifdef CONFIG_DRIVERS_LSPSRAM
extern int platform_psram_chip_config(void);
#endif

#ifdef CONFIG_COMPONENT_CLI
#include <console.h>
#endif

#ifdef CONFIG_COMPONENTS_AW_OTA_V2_NETWORK
#ifdef CONFIG_COMPONENTS_WLAN_CSFC
#include <wlan_csfc.h>
#endif
#endif

#ifdef CONFIG_DRIVERS_SPI_NOR_NG
#include <sunxi_hal_spinor.h>
#endif

extern int hal_flashc_init(void);
extern int pm_init(int argc, char **argv);
extern int amp_init(void);
#ifdef CONFIG_COMPONENTS_TCPIP
extern void cmd_tcpip_init(void);
#endif
extern void sun20i_boot_c906(void);
extern void sun20i_boot_dsp(void);

static void print_banner()
{
    printf("\r\n");
    printf(" *******************************************\r\n");
    printf(" **    Welcome to XR875 FreeRTOS %-9s**\r\n", TINA_RT_VERSION_NUMBER);
    printf(" **  Copyright (C) 2019-2022 XradioTech   **\r\n");
    printf(" **                                       **\r\n");
    printf(" **      starting armv8m FreeRTOS V1.1    **\r\n");
    printf(" **    Date:%s, Time:%s    **\r\n", __DATE__, __TIME__);
    printf(" *******************************************\r\n");
    printf("\r\n");
}

char *arm_get_version(void)
{
    return FIRMWARE_VERSION;
}

void cpu0_app_entry(void * param)
{
    int flash_ready = 0;
	(void)param;

    print_banner();
#ifdef CONFIG_COMPONENTS_AW_DEVFS
    devfs_mount("/dev");
#endif

#ifdef CONFIG_COMPONENTS_PM
    pm_init(1, NULL);
#endif

#ifdef CONFIG_DRIVERS_SPINOR
    flash_ready = !hal_spinor_init(0);
#elif defined(CONFIG_DRIVERS_SPI_NOR_NG)
	flash_ready = !hal_spi_nor_init(0);
#elif defined(CONFIG_DRIVERS_FLASHC)
    flash_ready = !hal_flashc_init();
#endif

    if (flash_ready) {
#ifdef CONFIG_COMPONENT_LITTLEFS
        littlefs_mount("/dev/UDISK", "/data", true);
#elif defined(CONFIG_COMPONENT_SPIFFS)
        spiffs_mount("/dev/UDISK", "/data", true);
#endif
    }
    else
        printf("flash not ready, skip mount\n");

#ifdef CONFIG_COMPONENTS_AW_OTA_V2_NETWORK
#ifdef CONFIG_DRIVERS_XRADIO
#ifdef CONFIG_FREQ_OFFSET_FROM_SDD
    int ret;
    uint16_t value;
    struct sdd sdd;

    ret = sdd_request(&sdd);
    if (ret > 0) {
        struct sdd_ie *id = sdd_find_ie(&sdd, SDD_WLAN_DYNAMIC_SECT_ID, SDD_XTAL_TRIM_ELT_ID);
        if (id) {
            value = id->data[0] | (id->data[1] << 8);
            hal_clk_ccu_aon_set_freq_trim(value);
            printf("get xtal trim from SDD, set trim to %d %s!\n\n",
                   value, (value == hal_clk_ccu_aon_get_freq_trim()) ? "success" : "fail");
        } else {
            printf("sdd xtal trim element is not found!\n");
        }
        sdd_release(&sdd);
    } else {
        printf("sdd is not found!\n");
    }
#endif
#endif
#endif

#ifdef CONFIG_COMPONENTS_TCPIP
    //tcpip stack init
    cmd_tcpip_init();
#endif

#ifdef CONFIG_COMPONENTS_AW_OTA_V2_NETWORK
#ifdef CONFIG_COMPONENTS_WLAN_CSFC
	// wifi fast connect
	wlan_csfc();
#endif
#endif

#ifdef CONFIG_ARCH_ARMV8M_DEFAULT_BOOT_DSP
    sun20i_boot_dsp();
#endif

#ifdef CONFIG_COMPONENTS_AMP
    amp_init();
#endif
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
    AudioSystemInit();
#endif

#ifdef CONFIG_COMPONENT_CLI
    vCommandConsoleStart(0x1000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif

#ifdef CONFIG_DRIVERS_SDMMC
    sunxi_driver_sdmmc_init();
#endif

#ifdef CONFIG_ARCH_ARMV8M_DEFAULT_BOOT_RISCV
    sun20i_boot_c906();
#endif

#ifdef CONFIG_DRIVERS_REGULATOR
    hal_soc_regulator_init();
#endif
#ifdef CONFIG_DRIVERS_LSPSRAM
    platform_psram_chip_config();
#endif

    vTaskDelete(NULL);
}
