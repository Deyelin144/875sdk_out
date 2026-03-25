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
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
#include <AudioSystem.h>
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

#ifdef CONFIG_DRIVERS_SPI_NOR_NG
#include <sunxi_hal_spinor.h>
#endif

#include <hal_gpio.h>
#include <sys/stat.h>
#include "kernel/os/os.h"
#include <sunxi_hal_rtc.h>
#include "drivers/pm/dev_pm.h"
#include "drivers/led/dev_led.h"
#include "drivers/rtc/dev_rtc.h"
#include "drivers/sd/dev_sd.h"
#include "drivers/atcmd/mcu_at_device.h"
#include "fs_demo.h"
#include "sdk_entry.h"
#include "prcm-sun20iw2/prcm.h"

extern int hal_power_init(void);
extern int hal_flashc_init(void);
extern int pm_init(int argc, char **argv);
extern int amp_init(void);
extern int g2d_probe(void);
extern int sunxi_soundcard_init(void);
extern int sunxi_sound_card_initialize(void);
extern int lcd_fb_probe(void);
extern int disp_probe(void);
extern int sunxi_gpadc_key_init(void);
extern int cpufreq_vf_init(void);
extern int sunxi_driver_sdmmc_init(void);
#ifdef CONFIG_COMPONENT_THERMAL
extern int thermal_init(void);
#endif
#ifdef CONFIG_COMPONENTS_TCPIP
extern void cmd_tcpip_init(void);
#endif

static void print_banner()
{
    printf("\r\n");
    printf(" *******************************************\r\n");
    printf(" **    Welcome to GU 206 FreeRTOS %-9s**\r\n", TINA_RT_VERSION_NUMBER);
    printf(" **   Copyright (C) 2019-2022 XradioTech  **\r\n");
    printf(" **                                       **\r\n");
    printf(" **      starting riscv FreeRTOS V1.1     **\r\n");
    printf(" **    Date:%s, Time:%s    **\r\n", __DATE__, __TIME__);
    printf(" *******************************************\r\n");
    printf("\r\n");
}

#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
static struct ahw_alias_config g_xr875_name_alias[] = {
        {
                "default", "playback", 0,
        },
        {
                "default", "amp-cap", 1,
        },
};

void *get_ahw_name_alias(int *num)
{
	*num = ARRAY_SIZE(g_xr875_name_alias);

	return g_xr875_name_alias;
}
#endif

#ifdef CONFIG_DRIVERS_WATCHDOG
#include <sunxi_hal_watchdog.h>

static XR_OS_Timer_t s_watchdog_timer;
static void watchdog_feed_callback(void *arg)
{
    printf("feed dog: %lu sec.\n", XR_OS_GetTime());
    hal_watchdog_feed();
}

static void platform_watchdog_enable(void)
{
    XR_OS_Status status = XR_OS_OK;

    hal_watchdog_init();
    // hal_watchdog_info();

    XR_OS_TimerSetInvalid(&s_watchdog_timer);
    status = XR_OS_TimerCreate(&s_watchdog_timer,
                               XR_OS_TIMER_PERIODIC,
                               watchdog_feed_callback,
                               NULL,
                               5 * 1000);
    if (XR_OS_OK != status) {
        printf("watchdog init fail: %d.\n", status);
        hal_watchdog_stop(11);
        return;
    }

    hal_watchdog_start(11);
    XR_OS_TimerStart(&s_watchdog_timer);
}

void platform_watchdog_disable(void)
{
    if (XR_OS_TimerIsValid(&s_watchdog_timer)) {
        XR_OS_TimerStop(&s_watchdog_timer);
    }

    hal_watchdog_stop(11);
}

void platform_watchdog_hungry(void)
{
    if (XR_OS_TimerIsValid(&s_watchdog_timer)) {
        XR_OS_TimerStop(&s_watchdog_timer);
    }
}
#else
void platform_watchdog_disable(void)
{
    // TODO
}
#endif /* CONFIG_DRIVERS_WATCHDOG */

void cpu0_app_entry(void *param)
{
    int flash_ready = 0;
    (void)param;
    HAL_PRCM_SelectEXTLDOVolt(PRCM_EXT_LDO_3V1);

    print_banner();
#ifdef CONFIG_DRIVERS_WATCHDOG
    platform_watchdog_enable();
#endif

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
#elif defined(CONFIG_DRIVERS_SPI_NOR_NG)
	flash_ready = !hal_spi_nor_init(0);
#elif defined(CONFIG_DRIVERS_FLASHC)
    flash_ready = !hal_flashc_init();
#elif defined(CONFIG_AMP_FLASHC_STUB)
    flash_ready = !hal_flashc_init();
#endif
    extern int hal_nand_init(void);
    flash_ready = !hal_nand_init();

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
    }
    else
    {
        printf("flash not ready, skip mount\n");
    }
#endif

	cpufreq_vf_init();
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
	// wlan_csfc();
#endif

#ifdef CONFIG_COMPONENT_CLI
    vCommandConsoleStart(0x1000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif

    mcu_at_device_init();

// #ifdef CONFIG_DRIVERS_SOUND
//     sunxi_soundcard_init();
// #endif

#ifdef CONFIG_DRIVERS_SOUND_V2
    sunxi_sound_card_initialize();
#endif

extern void cherryadb_init(uint32_t reg_base);
#ifdef CONFIG_CHERRY_USB_ADB_DEMO
    cherryadb_init(0);
#endif

    dev_pm_on(DEV_POWER_OFF_MODE_HIBERNATION);

#ifdef CONFIG_DRIVERS_G2D
	g2d_probe();
#endif

// #ifdef CONFIG_RTPLAYER_TEST
// 	boot_play();
// #endif

#ifdef CONFIG_DRIVERS_POWER
    hal_power_init();
#endif

#ifdef CONFIG_DRIVERS_POWER_PROTECT
    sunxi_power_protect_init();
#endif


#ifdef CONFIG_COMPONENT_THERMAL
	thermal_init();
#endif

#ifdef CONFIG_DISP2_SUNXI
	disp_probe();
#endif

    // rtc初始化要放在adc前面,因为adc回调中,使用了rtc的函数
    dev_rtc_init(NULL, NULL);
    dev_pm_init();
    // hal_rtc_init();

#ifdef CONFIG_DRIVERS_GPADC_KEY
    //    sunxi_gpadc_key_init();
#endif
#ifdef CONFIG_DRIVERS_SDMMC
    int sdmmc_ret = 0;
    int sdmmc_cnt = 0;
sdmmc_retry:
    sdmmc_ret = sunxi_driver_sdmmc_init(); // sdmmc 初始化不要和 usb 初始化放在一起, 尽量错开
    if ((sdmmc_ret < 0) && (sdmmc_cnt++ < 3)) {
        XR_OS_MSleep(100);
        goto sdmmc_retry;
    }
#endif

#ifdef CONFIG_DRIVERS_SPILCD
	int fb_cnt = 0;
    struct stat sdmmc_stat;

    while (fb_cnt++ < 50) { // 确保 sdmmc 挂载, 用于 TP 升级
        if (stat("/sdmmc", &sdmmc_stat) >= 0) {
            break;
        }
        XR_OS_MSleep(20);
    }
    lcd_fb_probe();
#endif

#ifdef CONFIG_COMPONENTS_AW_OTA_V2
    if (ota_init()) {
        printf("no need to OTA!\n");
    } else {
        printf("ota_update by thread!\n");
        xTaskCreate(ota_task_thread, "ota_update_task", 8192, NULL, 0, NULL);
    }
#endif

    dev_led_init();
    dev_sd_detect_init();

    if (sdk_entry_init() != 0) {
        printf("[sdk_entry] init failed.\n");
    }

    vTaskDelete(NULL);
}
