#ifndef __LCD_COMMON_FUNC_H__
#define __LCD_COMMON_FUNC_H__

#define LCD_COMMON_DLY_FLAG  0x80 // 延时标志位, 时间范围在 0-0xff, 若超出则要多延时几次

typedef struct {
    uint8_t data_size; /**< 范围在 0-0x7f, 若超出则要分成多组数据 */
    union {
        uint8_t cmd;
        uint8_t dly;
    };
    uint8_t data[];
} lcd_common_reg_para_t;

/**
 * @brief  LCD 屏幕寄存器初始
 * @param  sel 屏幕 id
 * @param  array 寄存器参数数组
 * @param  array_size 寄存器参数数组大小
 * @return 0:成功, -1:失败 
 */
int lcd_common_reg_init(uint32_t sel, uint8_t *array, uint32_t array_size);

#endif /* __LCD_COMMON_FUNC_H__ */