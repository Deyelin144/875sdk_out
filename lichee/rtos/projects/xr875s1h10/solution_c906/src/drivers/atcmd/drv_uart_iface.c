
#include "drv_uart_iface.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <hal_timer.h>
#include <sunxi_hal_timer.h>
#include <hal_uart.h>
#include <hal_mem.h>
#include "kernel/os/os_thread.h"
#include "kernel/os/os.h"
#include "../drv_log.h"

#define MCU_UART_PORT       UART_2
#define MCU_UART_BAUDRATE   UART_BAUDRATE_115200
#define MCU_UART_PARITY     UART_PARITY_NONE
#define MCU_STOPBITS        UART_STOP_BIT_1
#define MCU_DATABITS        UART_WORD_LENGTH_8


#define MCU_TIME_DEBUG 0

static uint8_t s_mcu_init = 0;
static uint8_t s_mcu_irq = 0;

int drv_mcu_iface_send_data(uint8_t* data, uint32_t len)
{
    int ret;
#if MCU_TIME_DEBUG
    uint32_t timetick = OS_GetTicks();
#endif

    ret = hal_uart_send(MCU_UART_PORT, data, len);

#if MCU_TIME_DEBUG
    if (ret > 0) {
        drv_loge("[%d]tick:%u, len:%d", MCU_UART_PORT, OS_GetTicks() - timetick, ret);
    }
#endif

    if (ret <= 0) {
        drv_loge("[%d]uart write error %d", MCU_UART_PORT, ret);
    }
    return ret;
}


int drv_mcu_iface_recv_data(uint8_t *data, uint32_t len,uint32_t timeout_ms)
{
    int ret;
#if MCU_TIME_DEBUG
    uint32_t timetick = OS_GetTicks();
#endif

    ret = hal_uart_receive_no_block(MCU_UART_PORT, data, len,timeout_ms);

#if MCU_TIME_DEBUG
    if (ret > 0) {
        drv_loge("[%d]tick:%u, len:%d", MCU_UART_PORT, OS_GetTicks() - timetick, ret);
    }
#endif       
    if (ret < 0) {
        drv_loge("[%d]uart recv error %d", MCU_UART_PORT, ret);
    }
    return ret;
}

int drv_mcu_iface_irq_init(hal_irq_handler_t cb)
{
    hal_gpio_set_pull(MCU_WAKEUP_PIN, GPIO_PULL_DOWN_DISABLED);
    hal_gpio_set_direction(MCU_WAKEUP_PIN, GPIO_DIRECTION_INPUT);

    hal_gpio_pinmux_set_function(MCU_WAKEUP_PIN,GPIO_MUXSEL_EINT);

    hal_gpio_to_irq(MCU_WAKEUP_PIN, &s_mcu_irq);
    hal_gpio_irq_request(s_mcu_irq, (hal_irq_handler_t)cb, IRQ_TYPE_EDGE_RISING, NULL);
    hal_gpio_irq_enable(s_mcu_irq);
}

int drv_mcu_iface_irq_deinit()
{
    int ret = -1;
    ret = hal_gpio_irq_disable(s_mcu_irq);
    if (ret)
    {
        drv_loge("disable irq fail %d", ret);
    }

    return ret;
}

int drv_mcu_iface_uart_init()
{
    // 串口初始化
    int ret = 0;
    _uart_config_t uart_config;
    uart_config.baudrate = MCU_UART_BAUDRATE;
    uart_config.word_length = MCU_DATABITS;
    uart_config.stop_bit = MCU_STOPBITS;
    uart_config.parity = MCU_UART_PARITY;

    ret = hal_uart_init(MCU_UART_PORT);
    if (ret != 0) {
        printf("uart init fail.\n");
        goto exit;
    }

    hal_uart_control(MCU_UART_PORT, 0, &uart_config);
    hal_uart_disable_flowcontrol(MCU_UART_PORT);

    drv_logi("dev mcu init success.\n");

    ret = 0;
    s_mcu_init = 1;
exit:
    return ret;
}

int drv_mcu_iface_uart_deinit()
{
    int ret = 0;
    s_mcu_init = 0;
    
    ret = hal_uart_deinit(MCU_UART_PORT);
    if (ret)
    {
        drv_loge("deinit uart fail %d", ret);
    }

    return ret ;
}

#define MCU_UART_RX   GPIOA(16)
#define MCU_UART_TX   GPIOA(17)
int drv_mcu_iface_uart_suspend()
{
    drv_mcu_iface_uart_deinit();

    //休眠时，引脚做输入，降低功耗
    hal_gpio_set_pull(MCU_UART_RX, GPIO_PULL_DOWN);
    hal_gpio_set_direction(MCU_UART_RX, GPIO_DIRECTION_INPUT);
    hal_gpio_pinmux_set_function(MCU_UART_RX,GPIO_MUXSEL_IN);

    hal_gpio_set_pull(MCU_UART_TX, GPIO_PULL_DOWN);
    hal_gpio_set_direction(MCU_UART_TX, GPIO_DIRECTION_INPUT);
    hal_gpio_pinmux_set_function(MCU_UART_TX,GPIO_MUXSEL_IN);
}

int drv_mcu_iface_uart_resume()
{
    hal_gpio_set_pull(MCU_UART_RX, GPIO_PULL_DOWN_DISABLED);
    hal_gpio_pinmux_set_function(MCU_UART_RX,GPIO_MUXSEL_DISABLED);

    hal_gpio_set_pull(MCU_UART_TX, GPIO_PULL_DOWN_DISABLED);
    hal_gpio_pinmux_set_function(MCU_UART_TX,GPIO_MUXSEL_DISABLED);

    drv_mcu_iface_uart_init();
}
