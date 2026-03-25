#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "interrupt.h"
#include <portmacro.h>
#include <console.h>
#include <aw_version.h>

#include <hal_msgbox.h>

#ifdef CONFIG_COMPONENT_ULOG
#include <ulog.h>
#endif

#include "FreeRTOS.h"
#include "task.h"

#include <devfs.h>
#include <littlefs.h>

#ifdef CONFIG_DRIVERS_XRADIO
#ifdef CONFIG_FREQ_OFFSET_FROM_SDD
#include <sdd.h>
#include <hal_clk.h>
#endif
#endif

void sun20i_boot_c906(void);
void sun20i_boot_dsp(void);

static void print_banner()
{
    printf("\r\n");
    printf(" *******************************************\r\n");
    printf(" **    Welcome to R128 FreeRTOS %-10s**\r\n", TINA_RT_VERSION_NUMBER);
    printf(" ** Copyright (C) 2019-2022 AllwinnerTech **\r\n");
    printf(" **                                       **\r\n");
    printf(" **      starting armv8m FreeRTOS V1.0    **\r\n");
    printf(" **    Date:%s, Time:%s    **\r\n", __DATE__, __TIME__);
    printf(" *******************************************\r\n");
    printf("\r\n");
}

extern int pm_init(int argc, char **argv);
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
    int hal_flashc_init(void);
    flash_ready = !hal_flashc_init();
#endif

    if (flash_ready) {
#ifdef CONFIG_COMPONENT_LITTLEFS
        littlefs_mount("/dev/UDISK", "/data", true);
#elif defined(CONFIG_COMPONENT_SPIFFS)
        spiffs_mount("/dev/UDISK", "/data", true);
#endif
    } else
        printf("flash not ready, skip mount\n");

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

#ifdef CONFIG_ARCH_ARMV8M_DEFAULT_BOOT_RISCV
    sun20i_boot_c906();
#endif
#ifdef CONFIG_ARCH_ARMV8M_DEFAULT_BOOT_DSP
    sun20i_boot_dsp();
#endif

#ifdef CONFIG_COMPONENTS_AMP
    int amp_init(void);
    amp_init();
#endif

#ifdef CONFIG_COMPONENT_CLI
    vCommandConsoleStart(0x1000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif

#ifdef CONFIG_DRIVERS_SDMMC
    sunxi_driver_sdmmc_init();
#endif

    vTaskDelete(NULL);
}
