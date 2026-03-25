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
extern int AudioSystemInit(void);
extern int sunxi_soundcard_init(void);
extern int sunxi_sound_card_initialize(void);
extern int lcd_fb_probe(void);
extern int sunxi_gpadc_key_init(void);
#ifdef CONFIG_COMPONENTS_TCPIP
extern void cmd_tcpip_init(void);
#endif

static void print_banner()
{
    printf("\r\n");
    printf(" *******************************************\r\n");
    printf(" **    Welcome to XR875 FreeRTOS %-10s**\r\n", TINA_RT_VERSION_NUMBER);
    printf(" ** Copyright (C) 2019-2022 XradioTech **\r\n");
    printf(" **                                       **\r\n");
    printf(" **      starting riscv FreeRTOS V1.0     **\r\n");
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
#endif

#ifdef CONFIG_COMPONENTS_PM
	hal_gpio_pm_register();
#endif

#ifdef CONFIG_COMPONENT_LITTLEFS
    if (flash_ready)
    {
        littlefs_mount("/dev/UDISK", "/data", true);
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

#ifdef CONFIG_DRIVERS_SOUND
    sunxi_soundcard_init();
#endif

#ifdef CONFIG_DRIVERS_SOUND_V2
    sunxi_sound_card_initialize();
#endif

#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
    AudioSystemInit();
#endif

extern void sunxi_usb_init(void);
#ifdef CONFIG_DRIVERS_USB
    sunxi_usb_init();
#endif

#ifdef CONFIG_COMPONENT_CLI
    vCommandConsoleStart(0x1000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif

#ifdef CONFIG_DRIVERS_G2D
	g2d_probe();
#endif

#ifdef CONFIG_DRIVERS_SPILCD
	lcd_fb_probe();
#endif

#ifdef CONFIG_DRIVERS_GPADC_KEY
       sunxi_gpadc_key_init();
#endif

#ifdef CONFIG_COMPONENTS_AW_OTA_V2
    if (ota_init()) {
        printf("no need to OTA!\n");
    } else {
        printf("ota_update by thread!\n");
        xTaskCreate(ota_task_thread, "ota_update_task", 8192, NULL, 0, NULL);
    }
#endif

    vTaskDelete(NULL);
}
