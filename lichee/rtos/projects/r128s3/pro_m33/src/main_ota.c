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

static void print_banner()
{
    printf("\r\n");
    printf(" *******************************************\r\n");
    printf(" **    Welcome to XR875 FreeRTOS %-9s**\r\n", TINA_RT_VERSION_NUMBER);
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int loacl_upgrade_cp_file_to_file(const char *from, const char *to)
{
    int fd_to, fd_from, ret = -1, rlen, wlen;
    char buf[1024] = {0};

    printf("start write data to %s\n", to);

    fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd_to < 0)
    {
        printf("open %s failed - %d", to, fd_to);
        return -1;
    }

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
    {
        printf("open %s failed - %d", from, fd_from);
        goto close_to;
    }

    // int cnt = 0;
    printf("update");
    while ((rlen = read(fd_from, buf, 1024)))
    {
        if (rlen < 0)
        {
            printf("read %s failed - %d\n", from, rlen);
            goto close_from;
        }

        wlen = write(fd_to, buf, rlen);
        if (wlen != rlen)
        {
            printf("write %s failed - %d\n", to, wlen);
            goto close_from;
        }

        printf(".");
    }
    printf("\n");
    printf("write complete to %s\n", to);

    ret = 0;
close_from:
    close(fd_from);
close_to:
    close(fd_to);
    return ret;
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

#ifdef CONFIG_COMPONENTS_AMP
    amp_init();
#endif

#ifdef CONFIG_DRIVERS_SOUND
    sunxi_soundcard_init();
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

#ifdef CONFIG_DRIVERS_SPILCD
	lcd_fb_probe();
#endif
    printf("----> enter recovery system\n");
    vTaskDelete(NULL);
}

