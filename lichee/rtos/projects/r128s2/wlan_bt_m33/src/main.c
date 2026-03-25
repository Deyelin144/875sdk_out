#include "serial.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "interrupt.h"
#include <portmacro.h>
#include <autoconf.h>
#include <cli_console.h>
#include <aw_version.h>

#ifdef CONFIG_COMPONENT_ULOG
#include <ulog.h>
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"



#ifdef CONFIG_AW_COMPONENT_XBTC_UART
#include "xbtc_uart.h"
#endif
/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/* The queue used by both tasks. */
static xQueueHandle xQueue = NULL;

void taskfunction1(void *param)
{
    const portTickType xDelay = pdMS_TO_TICKS(1000);
    for ( ; ; ) {
        /* printf("in %s\n", param); */
        vTaskDelay( xDelay );
    }
}

void cpu0_app_test(void *str)
{
    while(1)
    {
        printf("%s\n", str);
        vTaskDelay(1000);
    }
}

#ifdef CONFIG_AW_COMPONENT_XBTC_UART
static void init_config(void)
{
    xbtc_uart_config config;
    config.config.baudrate = UART_BAUDRATE_115200;
    config.config.word_length = UART_WORD_LENGTH_8;
    config.config.stop_bit = UART_STOP_BIT_1;
    config.config.parity = UART_PARITY_NONE;
    config.flow_control = TRUE;

    xbtc_uart_init(UART_2, &config);
}

#endif

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

void cpu0_app_entry(void * param)
{
    int flash_ready = 0;
    (void)param;

    print_banner();
#ifdef CONFIG_COMPONENTS_AW_DEVFS
    devfs_mount("/dev");
#endif

#ifdef CONFIG_DRIVERS_SPINOR
    flash_ready = !hal_spinor_init(0);
#elif defined(CONFIG_DRIVERS_FLASHC)
    flash_ready = !hal_flashc_init();
#endif

#ifdef CONFIG_COMPONENT_LITTLEFS
    if (flash_ready)
        littlefs_mount("/dev/UDISK", "/data", true);
    else
        printf("flash not ready, skip mount\n");
#endif

#ifdef CONFIG_COMPONENTS_AMP
    amp_init();
#endif

#ifdef CONFIG_COMPONENT_CLI
    vCommandConsoleStart(0x1000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif
#ifdef CONFIG_AW_COMPONENT_XBTC_UART
    init_config();
#endif
    vTaskDelete(NULL);
}
