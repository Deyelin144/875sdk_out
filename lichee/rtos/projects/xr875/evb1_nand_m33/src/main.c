#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "interrupt.h"
#include <portmacro.h>
#include <cli_console.h>
#include <aw_version.h>

#include <hal_thread.h>

#ifdef CONFIG_DRIVERS_MSGBOX
#include <hal_msgbox.h>
#endif

#ifdef CONFIG_COMPONENT_ULOG
#include <ulog.h>
#endif

#include "FreeRTOS.h"
#include "task.h"
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
#include <AudioSystem.h>
#endif
#ifdef CONFIG_DRIVERS_LSPSRAM
extern int platform_psram_chip_config(void);
#endif


extern int hal_flashc_init(void);
extern int pm_init(int argc, char **argv);
extern int amp_init(void);
extern void sun20i_boot_c906(void);
extern void sun20i_boot_dsp(void);

static void print_banner()
{
    printf("\r\n");
    printf(" *******************************************\r\n");
    printf(" **    Welcome to XR875 FreeRTOS %-10s**\r\n", TINA_RT_VERSION_NUMBER);
    printf(" ** Copyright (C) 2019-2022 XradioTech **\r\n");
    printf(" **                                       **\r\n");
    printf(" **      starting armv8m FreeRTOS V1.0    **\r\n");
    printf(" **    Date:%s, Time:%s    **\r\n", __DATE__, __TIME__);
    printf(" *******************************************\r\n");
    printf("\r\n");
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
#elif defined(CONFIG_DRIVERS_FLASHC)
    flash_ready = !hal_flashc_init();
#elif defined(CONFIG_DRIVERS_NAND_FLASH)
    extern int hal_nand_init(void);
	flash_ready = !hal_nand_init();
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
