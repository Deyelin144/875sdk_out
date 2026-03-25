#include "lcd_source.h"
#include "lcd_common_func.h"

int lcd_common_reg_init(uint32_t sel, uint8_t *array, uint32_t array_size)
{
    int ret = 0;

    if ((NULL == array) || (0 == array_size)) {
        ret = -1;
        goto exit;
    }

    uint8_t *reg_param = array;

    uint32_t data_size = 0;
    for (uint32_t i = 0; i < array_size;) {
        data_size = ((lcd_common_reg_para_t *)&reg_param[i])->data_size;
        if (LCD_COMMON_DLY_FLAG == data_size) {
            sunxi_lcd_delay_ms(((lcd_common_reg_para_t *)&reg_param[i])->dly);
            data_size = 0;

            // printf("[lcd_common_reg_init] delay %d ms.\n", ((lcd_common_reg_para_t *)&reg_param[i])->dly);
        } else {
            sunxi_lcd_cmd_write(sel, ((lcd_common_reg_para_t *)&reg_param[i])->cmd);
            for (uint16_t j = 0; j < data_size; j++) {
                sunxi_lcd_para_write(sel, ((lcd_common_reg_para_t *)&reg_param[i])->data[j]);
            }

            // printf("[lcd_common_reg_init] send cmd: 0x%x, data_size: %d.\n", ((lcd_common_reg_para_t *)&reg_param[i])->cmd, data_size);
            // if (data_size > 0) {
            //     printf("[lcd_common_reg_init] data: ");
            //     for (uint16_t j = 0; j < data_size; j++) {
            //         printf("0x%x ", ((lcd_common_reg_para_t *)&reg_param[i])->data[j]);
            //     }
            //     printf("\n");
            // }
        }
        i += 2 + data_size;
        data_size = 0;
    }

    ret = 0;
exit:
    return ret;
}