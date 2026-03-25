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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "interrupt.h"
#include <portmacro.h>
#include <cli_console.h>
#include <aw_version.h>

#ifdef CONFIG_COMPONENTS_AW_DEVFS
#include <devfs.h>
#endif

#ifdef CONFIG_COMPONENT_LITTLEFS
#include <littlefs.h>
#endif

#ifdef CONFIG_DRIVERS_XRADIO
#include <wlan_file.h>
#ifdef CONFIG_FREQ_OFFSET_FROM_SDD
#include <sdd.h>
#include <hal_clk.h>
#endif
#endif

#ifdef CONFIG_COMPONENT_ULOG
#include <ulog.h>
#endif

#ifdef CONFIG_DRIVERS_MSGBOX
#include <hal_msgbox.h>
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "tinatest.h"
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
#include <AudioSystem.h>
#endif
#ifdef CONFIG_DRIVERS_POWER
#include <sunxi_hal_power.h>
#endif
#ifdef CONFIG_DRIVERS_POWER_PROTECT
#include <sunxi_hal_power_protect.h>
#endif

#ifdef CONFIG_COMPONENT_CLI
#include <console.h>
#endif

#ifdef CONFIG_COMPONENTS_WLAN_CSFC
#include <wlan_csfc.h>
#endif



extern int hal_flashc_init(void);
extern int pm_init(int argc, char **argv);
extern int amp_init(void);
extern int g2d_probe(void);
extern int lcd_fb_probe(void);
extern int disp_probe(void);
extern int AudioSystemInit(void);
extern int sunxi_soundcard_init(void);
extern int sunxi_sound_card_initialize(void);
extern int cmd_rtplayer_test(int argc, char ** argv);
extern int sunxi_gpadc_key_init(void);
#ifdef CONFIG_COMPONENT_THERMAL
extern int thermal_init(void);
#endif
#ifdef CONFIG_COMPONENTS_TCPIP
extern void cmd_tcpip_init(void);
#endif

extern int gt911_init(void);
static void print_banner()
{
    printf("\r\n");
    printf(" *******************************************\r\n");
    printf(" **    Welcome to R128 FreeRTOS %-10s**\r\n", TINA_RT_VERSION_NUMBER);
    printf(" ** Copyright (C) 2019-2022 AllwinnerTech **\r\n");
    printf(" **                                       **\r\n");
    printf(" **      starting riscv FreeRTOS V1.1     **\r\n");
    printf(" **    Date:%s, Time:%s    **\r\n", __DATE__, __TIME__);
    printf(" *******************************************\r\n");
    printf("\r\n");
}

#ifdef CONFIG_RTPLAYER_TEST
static void boot_play(void)
{
	int argc = 3;
	char *argv[] = {
		"rtplayer_test",
		"/data/boot.mp3",
		"no_shell_input",
	};
	cmd_rtplayer_test(3, argv);
}
#endif

#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
static struct ahw_alias_config g_r128_name_alias[] = {
        {
                "default", "playback", 0,
        },
        {
                "default", "amp-cap", 1,
        },
};

void *get_ahw_name_alias(int *num)
{
	*num = ARRAY_SIZE(g_r128_name_alias);

	return g_r128_name_alias;
}
#endif

void cpu0_app_entry(void *param)
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

#ifdef CONFIG_COMPONENTS_AMP
    amp_init();
#endif

#ifdef CONFIG_DRIVERS_SPINOR
    flash_ready = !hal_spinor_init(0);
#elif defined(CONFIG_DRIVERS_FLASHC)
    flash_ready = !hal_flashc_init();
#elif defined(CONFIG_AMP_FLASHC_STUB)
    flash_ready = !hal_flashc_init();
#elif defined(CONFIG_DRIVERS_NAND_FLASH)
    extern int hal_nand_init(void);
    flash_ready = !hal_nand_init();
#endif

#ifdef CONFIG_COMPONENTS_PM
	hal_gpio_pm_register();
#endif

#ifdef CONFIG_COMPONENT_LITTLEFS
    if (flash_ready)
    {
        littlefs_mount("/dev/UDISK", "/data", true);
        #ifdef CONFIG_COMPONENT_SECURE_STORAGE
        littlefs_mount("/dev/secret", "/secret", true);
        #endif
#ifdef CONFIG_DRIVERS_XRADIO
        littlefs_mount("/dev/reserve", "/reserve", true); // for support WIFI file OTA
        char *wlan_bl = "/reserve/wlan_bl.bin";
        char *wlan_fw = "/reserve/wlan_fw.bin";
        char *wlan_etf = "/reserve/etf_r128.bin";
        char *sys_sdd = "/reserve/sys_sdd_40M.bin";
        wlan_set_file_path(FILE_WLAN_BL, wlan_bl, strlen(wlan_bl));
        wlan_set_file_path(FILE_WLAN_FW, wlan_fw, strlen(wlan_fw));
        wlan_set_file_path(FILE_WLAN_ETF, wlan_etf, strlen(wlan_etf));
        xr_set_sdd_file_path(sys_sdd, strlen(sys_sdd));
#endif
    }
    else
    {
        printf("flash not ready, skip mount\n");
    }
#endif

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

#ifdef CONFIG_COMPONENTS_TCPIP
    //tcpip stack init
    cmd_tcpip_init();
#endif

#ifdef CONFIG_COMPONENTS_WLAN_CSFC
	// wifi fast connect
	wlan_csfc();
#endif

#ifdef CONFIG_DRAGONMAT
	if (!access(TINATEST_OK_FILE, R_OK)) {
		printf("tinatest found %s\n", TINATEST_OK_FILE);
	} else {
		printf("start tinatest\n");
		int tinatest(int argc, char **argv);
		tinatest(1, NULL);
	}
#endif

#if (defined CONFIG_TESTCASE_REBOOT) && (!defined CONFIG_DRAGONMAT)
		int tinatest(int argc, char **argv);
		tinatest(1, NULL);
#endif

#ifdef CONFIG_DRIVERS_GPIO_EX_AW9523
extern int aw9523_gpio_init(void);
       aw9523_gpio_init();
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

#ifdef CONFIG_DRIVERS_USB
extern void sunxi_usb_init(void);
	sunxi_usb_init();
#endif

#ifdef CONFIG_COMPONENT_CLI
    vCommandConsoleStart(0x1000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif

#if (defined CONFIG_DRIVERS_SDMMC) && (defined CONFIG_COMPONENT_ELMFAT)
	sunxi_driver_sdmmc_init();
#endif

#ifdef CONFIG_DRIVERS_G2D
	g2d_probe();
#endif

#ifdef CONFIG_DRIVERS_SPILCD
	lcd_fb_probe();
#endif

#ifdef CONFIG_DISP2_SUNXI
	disp_probe();
#endif

#ifdef CONFIG_RTPLAYER_TEST
	boot_play();
#endif

#ifdef CONFIG_COMPONENT_THERMAL
	thermal_init();
#endif
#ifdef CONFIG_DRIVERS_POWER
    hal_power_init();
#endif

#ifdef CONFIG_DRIVERS_POWER_PROTECT
    sunxi_power_protect_init();
#endif

#ifdef CONFIG_DRIVERS_GPADC_KEY
       sunxi_gpadc_key_init();
#endif

    vTaskDelete(NULL);
}
