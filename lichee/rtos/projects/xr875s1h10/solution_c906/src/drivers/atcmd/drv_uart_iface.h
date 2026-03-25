#ifndef __DRV_UART_IFACE_H__
#define __DRV_UART_IFACE_H__

#include <hal_gpio.h>

#define MCU_WAKEUP_PIN         GPIOA(11)

int drv_mcu_iface_send_data(uint8_t* data, uint32_t len);
int drv_mcu_iface_recv_data(uint8_t *data, uint32_t len,uint32_t timeout_ms);
int drv_mcu_iface_irq_init(hal_irq_handler_t cb);
int drv_mcu_iface_irq_deinit();
int drv_mcu_iface_uart_init();
int drv_mcu_iface_uart_deinit();
int drv_mcu_iface_uart_suspend();
int drv_mcu_iface_uart_resume();
#endif /* DRV_MCU_IFACE_H */
