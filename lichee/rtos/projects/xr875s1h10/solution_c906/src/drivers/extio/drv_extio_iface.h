#ifndef __DRV_EXTIO_IFACE_H__
#define __DRV_EXTIO_IFACE_H__

typedef struct {
    uint16_t twi_id;
    uint32_t twi_freq;
    uint16_t dev_addr;
} drv_extio_param_t;

typedef void (*drv_extio_iface_irq_cb_t)(void *arg);

/**
 * @brief  I2C 初始化
 * @param  param 配置参数
 * @return 0:成功, -1:失败
 */
int drv_extio_iface_i2c_init(drv_extio_param_t *param);

/**
 * @brief I2C 注销
 * @param  param 配置参数
 * @return 0:成功, -1:失败
 */
int drv_extio_iface_i2c_deinit(drv_extio_param_t *param);

/**
 * @brief  I2C 发送数据
 * @param  param 配置参数
 * @param  buf 发送缓冲区
 * @param  len 发送缓冲区长度
 * @return 0:成功, -1:失败
 * @note   buf[0] 可以写入 reg_addr
 */
int drv_extio_iface_i2c_send(drv_extio_param_t *param, uint8_t *buf, uint32_t len);

/**
 * @brief  I2C 接收数据
 * @param  param 配置参数
 * @param  buf 接收缓冲区
 * @param  len 接收缓冲区长度
 * @return 0:成功, -1:失败
 * @note   buf[0] 可以写入 reg_addr
 */
int drv_extio_iface_i2c_recv(drv_extio_param_t *param, uint8_t *buf, uint32_t len);

/**
 * @brief  I2C 接收数据
 * @param  param 配置参数
 * @param  reg_addr 外设的寄存器地址
 * @param  buf 接收缓冲区
 * @param  len 接收缓冲区长度
 * @return 0:成功, -1:失败
 */
int drv_extio_iface_i2c_recv_with_addr(drv_extio_param_t *param, uint8_t reg_addr, uint8_t *buf, uint32_t len);

/**
 * @brief  EXTIO 中断初始化
 * @param  irq_cb 中断回调函数
 * @return 0:成功, -1:失败
 */
int drv_extio_iface_irq_init(drv_extio_iface_irq_cb_t irq_cb);

/**
 * @brief  EXTIO 中断注销
 */
void drv_extio_iface_irq_deinit(void);

/**
 * @brief  aw9523b 芯片初始化后单独设置耳机检测引脚 
 */
void aw9523b_earphone_gpio_init(void);

/**
 * @brief aw9523b 芯片检测耳机检测GPIO引脚电平
 * @return 1:未触发,0:触发
 */
uint8_t aw9523b_read_earphone_gpio(void);

#endif /* __DRV_EXTIO_IFACE_H__ */
