#ifndef __TP_BOARD_CONFIG_H__
#define __TP_BOARD_CONFIG_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TP_NODE_NAME   "touchscreen"
#define TP_DRIVER_NAME "touchscreen"

#define TP_INVALID_GPIO (0xffff)

typedef enum {
    TP_PROPERTY_UNDEFINED = 0, /**< 未知 */
    TP_PROPERTY_INTGER,        /**< 属性类型: 整形 */
    TP_PROPERTY_STRING,        /**< 属性类型: 字符串 */
    TP_PROPERTY_GPIO,          /**< 属性类型: GPIO (输入输出) */
    TP_PROPERTY_PIN,           /**< 属性类型: GPIO (其他功能复用) */
} tp_proerty_type_t;

typedef struct {
    int port;
    int port_num;
    int mul_sel;
    int pull;
    int drv_level;
    int data;
    int gpio;
} tp_gpio_set_t;

typedef struct {
    uint8_t tp_used;
    char tp_driver_name[32];
    uint8_t tp_revert_mode;
    uint8_t tp_exchange_flag;
    uint16_t tp_max_x;
    uint16_t tp_max_y;
    uint16_t tp_twi_id;
    uint16_t tp_addr;
    tp_gpio_set_t tp_int;
    tp_gpio_set_t tp_reset;
} tp_board_config_t;

/**
 * @brief  是否支持 fex 配置文件
 * @return true:支持, false:不支持
 */
bool tp_board_config_is_support(void);

/**
 * @brief  获取解析 fex 配置文件得到的驱动名称
 * @return 指向驱动名称的指针
 */
char *tp_board_config_drv_name_get(void);

/**
 * @brief  解析 fex 配置文件
 * @return 0: 成功, -1:失败
 */
int tp_board_config_parse(void);

/**
 * @brief  硬件初始化
 * @return >=0: 中断号, -1:失败
 */
int tp_board_config_hw_init(void);

/**
 * @brief  硬件反初始化
 * @param  irq_num 初始化成功得到的中断号
 * @return 0: 成功, -1:失败
 */
int tp_board_config_hw_deinit(uint32_t irq_num);

/**
 * @brief  I2C 读操作
 * @param  buf 存放数据的缓冲区
 * @param  len 缓冲区长度
 * @return 0: 成功, -1:失败
 */
int tp_board_config_i2c_read(uint8_t *buf, uint32_t len);

/**
 * @brief  I2C 读操作
 * @param  addr 寄存器地址
 * @param  buf 存放数据的缓冲区
 * @param  len 缓冲区长度
 * @return 0: 成功, -1:失败
 */
int tp_board_config_i2c_read_with_addr(uint8_t addr, uint8_t *buf, uint32_t len);

/**
 * @brief  I2C 写操作
 * @param  buf 要发送数据的缓冲区
 * @param  len 缓冲区长度
 * @return 0: 成功, -1:失败
 * @note   缓冲区包括: 地址 + 内容
 */
int tp_board_config_i2c_write(uint8_t *buf, uint32_t len);

/***
 * @brief  获取 TP 区域 x 方向的大小
 * @return 0:不支持, >0: x 方向的大小
 */
uint16_t tp_board_config_tp_max_x_get(void);

/***
 * @brief  获取 TP 区域 y 方向的大小
 * @return 0:不支持, >0: y 方向的大小
 */
uint16_t tp_board_config_tp_max_y_get(void);

/**
 * @brief  坐标反转
 * @return 0:默认, 1: x 反转, 2: y 反转
 */
uint8_t tp_board_config_tp_revert_mode_get(void);

/**
 * @brief  坐标交换
 * @return 0:默认, 1: x 和 y 对调
 */
uint8_t tp_board_config_tp_exchange_flag_get(void);

/**
 * @brief  TP 复位引脚设为高电平
 */
void tp_board_config_reset_pin_high(void);

/**
 * @brief  TP 复位引脚设为低电平
 */
void tp_board_config_reset_pin_low(void);

#ifdef __cplusplus
}
#endif

#endif /* __TP_BOARD_CONFIG_H__ */
