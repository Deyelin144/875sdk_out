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
#include <sunxi_hal_spinor.h>

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
#include "recovery/recovery_main.h"
#include "upgrade_logic/upgrade_logic.h"
#include "lv_port/lv_init.h"
#include "native_app_ota/native_app_ota.h"
#include "drivers/pm/drv_pm_iface.h"
#include "drivers/drv_common.h"
#include "prcm-sun20iw2/prcm.h"

extern int hal_flashc_init(void);
extern int pm_init(int argc, char **argv);
extern int amp_init(void);
#ifdef CONFIG_COMPONENTS_TCPIP
extern void cmd_tcpip_init(void);
#endif
extern void sun20i_boot_c906(void);
extern int hal_power_init(void);
extern int lcd_fb_probe(void);
extern int sunxi_driver_sdmmc_init(void);
extern int32_t hal_uart_enable_rx(int32_t uart_port);
extern drv_status_t dev_sd_detect_init(void);

static void device_poweron(void)
{
    volatile uint32_t value = DEV_POWER_REG_READ_32(CODEC_ADC_MBIAS_CTRL_REG);
    /* MBIAS select 2.50V, 确保升压 IC 正常使能 */
    value |= CODEC_ADC_MBIAS_2_5V;

    /* enable Vmbias output */
    value |= CODEC_ADC_MBIAS_EN;
    DEV_POWER_REG_WRITE_32(CODEC_ADC_MBIAS_CTRL_REG, value);

    hal_uart_enable_rx(1); // 使能uart1 rx中断
}

static void print_banner()
{
    printf("\r\n");
    printf(" *******************************************\r\n");
    printf(" **    Welcome to GU 206 FreeRTOS %-9s**\r\n", TINA_RT_VERSION_NUMBER);
    printf(" **  Copyright (C) 2019-2022 XradioTech   **\r\n");
    printf(" **                                       **\r\n");
    printf(" **      starting OTA Recovery    **\r\n");
    printf(" **    Date:%s, Time:%s    **\r\n", __DATE__, __TIME__);
    printf(" *******************************************\r\n");
    printf("\r\n");
}

#ifdef CONFIG_COMPONENTS_AW_OTA_V2
extern int ota_update(void);
extern int ota_init(void);
static void ota_task_thread(void *arg)
{
    int ret = 0;
    printf("ota_update by thread!\n");
    ret = ota_update();
    vTaskDelete(NULL);
}
#endif

#ifdef CONFIG_DRIVERS_WATCHDOG
#include <sunxi_hal_watchdog.h>
#include "kernel/os/os_timer.h"

static XR_OS_Timer_t s_watchdog_timer;

static void watchdog_feed_callback(void *arg)
{
    // printf("feed dog: %lu sec.\n", XR_OS_GetTime());
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

static void *s_upgrade_tid = NULL;
static void upgrade_task(void *arg)
{
    if (upgrade_logic_get_config() != 0) {
        if (upgrade_logic_reboot(upgrade_logic_get_handle(), UPGRADE_MODE_NORMAL) != 0) {
            upgrade_logic_reboot(upgrade_logic_get_handle(), UPGRADE_MODE_RECOVERY);
        }
    } else {
        os_adapter()->msleep(100);
        native_app_ota_enter(NULL);
    }
    os_adapter()->thread_delete(&s_upgrade_tid);
}
void cpu0_app_entry(void * param)
{
    int flash_ready = 0;
	(void)param;

    print_banner();
    HAL_PRCM_SelectEXTLDOVolt(PRCM_EXT_LDO_3V1);
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

#ifdef CONFIG_DRIVERS_NAND_FLASH
    extern int hal_nand_init(void);
    flash_ready = !hal_nand_init();
    printf("---->nand flash init\n");
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

#ifdef CONFIG_COMPONENT_CLI
    vCommandConsoleStart(0x1000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif

    device_poweron();
    mcu_at_device_init();

#ifdef CONFIG_COMPONENTS_AW_OTA_V2_NETWORK
#ifdef CONFIG_COMPONENTS_WLAN_CSFC
	// wifi fast connect
	wlan_csfc();
#endif
#endif

#ifdef CONFIG_COMPONENTS_AMP
    amp_init();
#endif

#ifdef CONFIG_DRIVERS_SOUND
    sunxi_soundcard_init();
#endif

#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
    AudioSystemInit();
#endif


#ifdef CONFIG_DRIVERS_G2D
	g2d_probe();
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

#ifdef CONFIG_ARCH_ARMV8M_DEFAULT_BOOT_RISCV
    sun20i_boot_c906();
#endif

#ifdef CONFIG_DRIVERS_REGULATOR
    hal_soc_regulator_init();
#endif
#ifdef CONFIG_DRIVERS_LSPSRAM
    platform_psram_chip_config();
#endif

#ifdef CONFIG_DISP2_SUNXI
	disp_probe();
#endif

#ifdef CONFIG_DRIVERS_SPILCD
    lcd_fb_probe();
#endif

#ifdef CONFIG_DISP2_SUNXI
	disp_probe();
#endif

#ifdef CONFIG_DRIVERS_POWER
    hal_power_init();
#endif

#ifdef CONFIG_DRIVERS_WATCHDOG
    platform_watchdog_enable();
#endif

    printf("recovery version: %s\n", RECOVERY_VERSION);

    dev_sd_detect_init();

    dev_led_init();
    dev_led_set_bright(0,0);

    extern int bsp_disp_lcd_is_enabled(unsigned int screen_id);
    while (0 == bsp_disp_lcd_is_enabled(0)) {/*等待屏幕初始化完成再跑lvgl*/
        printf("wait for lcd enable\n");
        os_adapter()->msleep(200);
    }

    extern s32 bsp_disp_lcd_set_bright(u32 disp, u32 bright);
    bsp_disp_lcd_set_bright(0, 0);

    int ret = os_adapter()->thread_create(&s_upgrade_tid,
                                      upgrade_task,
                                      NULL,
                                      "upgrade_task",
                                      OS_ADAPTER_PRIORITY_NORMAL + 1,
                                      10 * 1024);
    if (ret != 0) {
        LOG_ERR("upgrade_task create failed");
    }

    vTaskDelete(NULL);
}

