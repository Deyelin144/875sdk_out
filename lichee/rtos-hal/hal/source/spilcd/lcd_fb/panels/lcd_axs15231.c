#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "compiler.h"
#include "../../../../../../rtos/drivers/drv/input/tp/config/tp_board_config.h"
#include "lcd_common_func.h"
#include "lcd_axs15231.h"
#include "kernel/os/os.h"

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);
static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

#define LCD_AXS15231_FW_UPGRADE          1 // 0:关闭, 1:开启 (带有 flash 的 AXS15231 的升级 走 upgrade)
#define LCD_AXS15231_FW_DOWNLOAD         0 // 0:关闭, 1:开启 (不带 flash 的 AXS15231 的升级 走 download)
#define LCD_AXS15231_FW_VERIFY           0 // 0:关闭升级成功后的校验, 1:开启 (同时作用于 upgrade 和 download)
#define LCD_AXS15231_FW_FORCE            0 // 0:根据版本号升级, 1:强制升级

#define LCD_AXS15231_PARAM_INIT_TYPE     0 // 0:由 LCD 自带固件初始化, 1:由代码初始化 SPI, 2:由代码初始化 DSPI, 3:由代码初始化 QSPI

#define LCD_AXS15231_FIRMWARE_LFS_PATH   "/data/lfs/tp_update.bin"
#define LCD_AXS15231_FIRMWARE_FATFS_PATH "/sdmmc/tp_update.bin"

#define AXS_QSPI_CMD_LEN                 4

extern void dev_mcu_lcd_reset_set(int val);
extern uint8_t dev_pm_is_goto_hibernation();

//#define lcd_reset(sel, val) sunxi_lcd_gpio_set_value(sel, 0, val)// 复位脚使用 lcd_gpio_0
#define lcd_reset(sel, val) dev_mcu_lcd_reset_set(val)

static int s_axs_spi_port = 0;
static uint8_t *s_axs_buf = NULL;
static uint32_t s_axs_screen_size = 0;
static bool s_axs_set_win = true;
static u32 s_sel = 0;
static int s_init = 0;//初始化标志

static struct disp_panel_para info[LCD_FB_MAX] = {0};

#if (1 == LCD_AXS15231_PARAM_INIT_TYPE)
__xip_rodata static uint8_t s_axs15231_reg_param[] = {
    // data_size cmd/dly data                     4                             9                            14                            19                            24                            29
       8,        0xbb,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0xa5,
       17,       0xa0,   0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x0c, 0x3f, 0x20, 0x05, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00,
       // a2 的 part[0] 由 30 改为 0x21 可进入自检模式
       31,       0xa2,   0x30, 0x3c, 0x20, 0x14, 0x9e, 0x20, 0x9e, 0xe0, 0x40, 0x19, 0x80, 0x80, 0x80, 0x20, 0xf9, 0x10, 0x02, 0xff, 0xff, 0xf0, 0x90, 0x01, 0x32, 0xa0, 0x91, 0xc0, 0x20, 0x7f, 0xff, 0x00, 0x04,
       30,       0xd0,   0xe0, 0x40, 0x51, 0x24, 0x08, 0x05, 0x10, 0x01, 0x90, 0x12, 0xc2, 0x42, 0x22, 0x22, 0xaa, 0x03, 0x10, 0x12, 0x60, 0x14, 0x1e, 0x51, 0x15, 0x00, 0x20, 0x10, 0x00, 0x03, 0x3a, 0x12,
       22,       0xa3,   0xa0, 0x06, 0xaa, 0x00, 0x08, 0x02, 0x0a, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x55, 0x55,
       30,       0xc1,   0x31, 0x04, 0x02, 0x02, 0x71, 0x05, 0x24, 0x55, 0x02, 0x00, 0x41, 0x01, 0x53, 0xff, 0xff, 0xff, 0x4f, 0x52, 0x00, 0x4f, 0x52, 0x00, 0x45, 0x3b, 0x0b, 0x02, 0x0d, 0x00, 0xff, 0x40,
       11,       0xc3,   0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01,
       29,       0xc4,   0x00, 0x24, 0x33, 0x80, 0x00, 0xea, 0x64, 0x32, 0xc8, 0x64, 0xc8, 0x32, 0x90, 0x90, 0x11, 0x06, 0xdc, 0xfa, 0x00, 0x00, 0x80, 0xfe, 0x10, 0x10, 0x00, 0x0a, 0x0a, 0x44, 0x50,
       23,       0xc5,   0x18, 0x00, 0x00, 0x03, 0xfe, 0x36, 0x4b, 0x20, 0x30, 0x10, 0x88, 0xde, 0x0d, 0x08, 0x0f, 0x0f, 0x01, 0x36, 0x4b, 0x20, 0x10, 0x10, 0x00,
       20,       0xc6,   0x05, 0x0a, 0x05, 0x0a, 0x00, 0xe0, 0x2e, 0x0b, 0x12, 0x22, 0x12, 0x22, 0x01, 0x03, 0x00, 0x3f, 0x6a, 0x18, 0xc8, 0x22,
       20,       0xc7,   0x50, 0x32, 0x28, 0x00, 0xa2, 0x80, 0x8f, 0x00, 0x80, 0xff, 0x07, 0x11, 0x9c, 0x67, 0xff, 0x24, 0x0c, 0x0d, 0x0e, 0x0f,
       4,        0xc9,   0x33, 0x44, 0x44, 0x01,
       27,       0xcf,   0x2c, 0x1e, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1e, 0x68, 0x88, 0x00, 0x65, 0x09, 0x22, 0xc4, 0x0c, 0x77, 0x22, 0x44, 0xaa, 0x55, 0x08, 0x08, 0x12, 0xa0, 0x08,
       30,       0xd5,   0x00, 0x30, 0x8d, 0x01, 0x35, 0x04, 0x92, 0x6f, 0x04, 0x92, 0x6f, 0x04, 0x08, 0x6a, 0x04, 0x46, 0x03, 0x03, 0x03, 0x03, 0x82, 0x01, 0x03, 0x00, 0xe0, 0x51, 0xa1, 0x00, 0x00, 0x00,
       30,       0xd6,   0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe, 0x93, 0x00, 0x01, 0x83, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x84, 0x00, 0x20, 0x01, 0x00,
       19,       0xd7,   0x03, 0x01, 0x0b, 0x09, 0x0f, 0x0d, 0x1e, 0x1f, 0x18, 0x1d, 0x1f, 0x19, 0x00, 0x30, 0x04, 0x00, 0x20, 0x49, 0x1f,
       12,       0xd8,   0x02, 0x00, 0x0a, 0x08, 0x0e, 0x0c, 0x1e, 0x1f, 0x18, 0x1d, 0x1f, 0x19,
       12,       0xd9,   0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
       12,       0xdd,   0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
       8,        0xdf,   0x44, 0x73, 0x4b, 0x69, 0x00, 0x0a, 0x02, 0x90,
       17,       0xe0,   0x3b, 0x28, 0x10, 0x16, 0x0c, 0x06, 0x11, 0x28, 0x5c, 0x21, 0x0d, 0x35, 0x13, 0x2c, 0x33, 0x28, 0x0d,
       17,       0xe1,   0x37, 0x28, 0x10, 0x16, 0x0b, 0x06, 0x11, 0x28, 0x5c, 0x21, 0x0d, 0x35, 0x14, 0x2c, 0x33, 0x28, 0x0f,
       17,       0xe2,   0x3b, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x35, 0x44, 0x32, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x2f, 0x0d,
       17,       0xe3,   0x37, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x35, 0x44, 0x32, 0x0c, 0x14, 0x14, 0x36, 0x32, 0x2f, 0x0f,
       17,       0xe4,   0x3b, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x39, 0x44, 0x2e, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x2f, 0x0d,
       17,       0xe5,   0x37, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x39, 0x44, 0x2e, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x2f, 0x0f,
       8,        0xbb,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

       LCD_COMMON_DLY_FLAG, 10,
       0,        0x11, 
       LCD_COMMON_DLY_FLAG, 50,
       0,        0x29,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
};
#elif (2 == LCD_AXS15231_PARAM_INIT_TYPE)
__xip_rodata static uint8_t s_axs15231_reg_param[] = {/*昊宏axs15231初始化参数*/
    // data_size cmd/dly data                     4                             9                            14                            19                            24                            29
       8,        0xbb,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0xa5,
       17,       0xa0,   0xc0, 0x10, 0x00, 0x02, 0x00, 0x00, 0x64, 0x3f, 0x20, 0x05, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00,
       // a2 的 part[0] 由 30 改为 0x21 可进入自检模式
       31,       0xa2,   0x31, 0x3c, 0x29, 0x14, 0xd0, 0x90, 0xff, 0xe0, 0x40, 0x19, 0x80, 0x80, 0x80, 0x20, 0xf9, 0x10, 0x02, 0xff, 0xff, 0xf0, 0x90, 0x01, 0x32, 0xa0, 0x91, 0xc0, 0x20, 0x7f, 0xff, 0x00, 0x54,
       30,       0xd0,   0xe0, 0x40, 0x51, 0x24, 0x08, 0x05, 0x10, 0x01, 0x0d, 0x15, 0xc2, 0x42, 0x22, 0x22, 0xaa, 0x03, 0x10, 0x12, 0x60, 0x14, 0x1e, 0x51, 0x15, 0x00, 0x45, 0x20, 0x00, 0x03, 0x3d, 0x12,
       22,       0xa3,   0xa0, 0x06, 0xaa, 0x00, 0x08, 0x02, 0x0a, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x55, 0x55,
    //    16,       0xa4,   0x85, 0x85, 0x95, 0x82, 0xaf, 0xaa, 0xaa, 0x80, 0x10, 0x30, 0x40, 0x40, 0x20, 0xff, 0x60, 0x30,
    //    4,        0xa4,   0x85, 0x85, 0x95, 0x85,
       30,       0xc1,   0x31, 0x04, 0x02, 0x02, 0x71, 0x05, 0x27, 0x55, 0x02, 0x00, 0x41, 0x00, 0x53, 0xff, 0xff, 0xff, 0x4f, 0x52, 0x00, 0x4f, 0x52, 0x00, 0x45, 0x3b, 0x0b, 0x02, 0x0d, 0x00, 0xff, 0x40,
       11,       0xc3,   0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01,
       29,       0xc4,   0x00, 0x24, 0x33, 0x80, 0x00, 0xea, 0x64, 0x32, 0xc8, 0x64, 0xc8, 0x32, 0x90, 0x90, 0x11, 0x06, 0xdc, 0xfa, 0x00, 0x00, 0x80, 0xfe, 0x10, 0x10, 0x00, 0x0a, 0x0a, 0x44, 0x50,
       // TP
       23,       0xc5,   0x18, 0x00, 0x00, 0x03, 0xfe, 0x50, 0x38, 0x20, 0x30, 0x10, 0x88, 0xde, 0x0d, 0x08, 0x0f, 0x0f, 0x01, 0x50, 0x38, 0x20, 0x10, 0x10, 0x00,
       20,       0xc6,   0x05, 0x0a, 0x05, 0x0a, 0x00, 0xe0, 0x2e, 0x0b, 0x12, 0x22, 0x12, 0x22, 0x01, 0x03, 0x00, 0x02, 0x6a, 0x18, 0xc8, 0x22,
       20,       0xc7,   0x50, 0x32, 0x28, 0x00, 0xa2, 0x80, 0x8f, 0x00, 0x80, 0xff, 0x07, 0x11, 0x9c, 0x67, 0xff, 0x24, 0x0c, 0x0d, 0x0e, 0x0f,
       4,        0xc9,   0x33, 0x44, 0x44, 0x01,
       27,       0xcf,   0x2c, 0x1e, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1e, 0x68, 0x88, 0x00, 0x65, 0x09, 0x22, 0xc4, 0x0c, 0x77, 0x22, 0x44, 0xaa, 0x55, 0x08, 0x08, 0x12, 0xa0, 0x08,
       30,       0xd5,   0x00, 0x3e, 0x89, 0x01, 0x35, 0x04, 0x92, 0x6f, 0x04, 0x92, 0x6f, 0x04, 0x08, 0x6a, 0x04, 0x46, 0x03, 0x03, 0x03, 0x03, 0x00, 0x01, 0x03, 0x00, 0xe0, 0x51, 0xa1, 0x00, 0x00, 0x00,
       30,       0xd6,   0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe, 0x93, 0x00, 0x01, 0x83, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x84, 0x00, 0x20, 0x01, 0x00,
       19,       0xd7,   0x03, 0x01, 0x0b, 0x09, 0x0f, 0x0d, 0x1e, 0x1f, 0x18, 0x1d, 0x1f, 0x19, 0x00, 0x3e, 0x04, 0x00, 0x1d, 0x40, 0x1f,
       12,       0xd8,   0x02, 0x00, 0x0a, 0x08, 0x0e, 0x0c, 0x1e, 0x1f, 0x18, 0x1d, 0x1f, 0x19,
       12,       0xd9,   0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
       12,       0xdd,   0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
       8,        0xdf,   0x44, 0x73, 0x4b, 0x69, 0x00, 0x0a, 0x02, 0x90,
       17,       0xe0,   0x3b, 0x28, 0x0f, 0x14, 0x0c, 0x03, 0x11, 0x26, 0x4b, 0x21, 0x0d, 0x36, 0x13, 0x2a, 0x2f, 0x28, 0x0d,
       17,       0xe1,   0x37, 0x28, 0x0f, 0x14, 0x0b, 0x03, 0x11, 0x26, 0x4b, 0x21, 0x0d, 0x36, 0x13, 0x2a, 0x2d, 0x28, 0x0f,
       17,       0xe2,   0x3b, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x35, 0x44, 0x32, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x0f, 0x0d,
       17,       0xe3,   0x37, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x35, 0x44, 0x32, 0x0c, 0x14, 0x14, 0x36, 0x32, 0x2f, 0x0f,
       17,       0xe4,   0x3b, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x39, 0x44, 0x2e, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x2f, 0x0d,
       17,       0xe5,   0x37, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x39, 0x44, 0x2e, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x2f, 0x0f,
       8,        0xbb,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

       LCD_COMMON_DLY_FLAG, 100,
       0,        0x11, 
       LCD_COMMON_DLY_FLAG, 100,
       0,        0x29,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
};
#elif (3 == LCD_AXS15231_PARAM_INIT_TYPE)
__xip_rodata static uint8_t s_axs15231_reg_param[] = {
    // data_size cmd/dly data                     4                             9                            14                            19                            24                            29
       8,        0xbb,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0xa5,
       17,       0xa0,   0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x0c, 0x3f, 0x20, 0x05, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00,
       // a2 的 part[0] 由 30 改为 0x21 可进入自检模式
       31,       0xa2,   0x30, 0x3c, 0x20, 0x14, 0x9e, 0x20, 0x9e, 0xe0, 0x40, 0x19, 0x80, 0x80, 0x80, 0x20, 0xf9, 0x10, 0x02, 0xff, 0xff, 0xf0, 0x90, 0x01, 0x32, 0xa0, 0x91, 0xc0, 0x20, 0x7f, 0xff, 0x00, 0x04,
       30,       0xd0,   0xe0, 0x40, 0x51, 0x24, 0x08, 0x05, 0x10, 0x01, 0x90, 0x12, 0xc2, 0x42, 0x22, 0x22, 0xaa, 0x03, 0x10, 0x12, 0x60, 0x14, 0x1e, 0x51, 0x15, 0x00, 0x20, 0x10, 0x00, 0x03, 0x3a, 0x12,
       22,       0xa3,   0xa0, 0x06, 0xaa, 0x00, 0x08, 0x02, 0x0a, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x55, 0x55,
       30,       0xc1,   0x31, 0x04, 0x02, 0x02, 0x71, 0x05, 0x24, 0x55, 0x02, 0x00, 0x41, 0x01, 0x53, 0xff, 0xff, 0xff, 0x4f, 0x52, 0x00, 0x4f, 0x52, 0x00, 0x45, 0x3b, 0x0b, 0x02, 0x0d, 0x00, 0xff, 0x40,
       11,       0xc3,   0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01,
       29,       0xc4,   0x00, 0x24, 0x33, 0x80, 0x00, 0xea, 0x64, 0x32, 0xc8, 0x64, 0xc8, 0x32, 0x90, 0x90, 0x11, 0x06, 0xdc, 0xfa, 0x00, 0x00, 0x80, 0xfe, 0x10, 0x10, 0x00, 0x0a, 0x0a, 0x44, 0x50,
       23,       0xc5,   0x18, 0x00, 0x00, 0x03, 0xfe, 0x36, 0x4b, 0x20, 0x30, 0x10, 0x88, 0xde, 0x0d, 0x08, 0x0f, 0x0f, 0x01, 0x36, 0x4b, 0x20, 0x10, 0x10, 0x00,
       20,       0xc6,   0x05, 0x0a, 0x05, 0x0a, 0x00, 0xe0, 0x2e, 0x0b, 0x12, 0x22, 0x12, 0x22, 0x01, 0x03, 0x00, 0x3f, 0x6a, 0x18, 0xc8, 0x22,
       20,       0xc7,   0x50, 0x32, 0x28, 0x00, 0xa2, 0x80, 0x8f, 0x00, 0x80, 0xff, 0x07, 0x11, 0x9c, 0x67, 0xff, 0x24, 0x0c, 0x0d, 0x0e, 0x0f,
       4,        0xc9,   0x33, 0x44, 0x44, 0x01,
       27,       0xcf,   0x2c, 0x1e, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1e, 0x68, 0x88, 0x00, 0x65, 0x09, 0x22, 0xc4, 0x0c, 0x77, 0x22, 0x44, 0xaa, 0x55, 0x08, 0x08, 0x12, 0xa0, 0x08,
       30,       0xd5,   0x00, 0x30, 0x8d, 0x01, 0x35, 0x04, 0x92, 0x6f, 0x04, 0x92, 0x6f, 0x04, 0x08, 0x6a, 0x04, 0x46, 0x03, 0x03, 0x03, 0x03, 0x82, 0x01, 0x03, 0x00, 0xe0, 0x51, 0xa1, 0x00, 0x00, 0x00,
       30,       0xd6,   0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe, 0x93, 0x00, 0x01, 0x83, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x84, 0x00, 0x20, 0x01, 0x00,
       19,       0xd7,   0x03, 0x01, 0x0b, 0x09, 0x0f, 0x0d, 0x1e, 0x1f, 0x18, 0x1d, 0x1f, 0x19, 0x00, 0x30, 0x04, 0x00, 0x20, 0x49, 0x1f,
       12,       0xd8,   0x02, 0x00, 0x0a, 0x08, 0x0e, 0x0c, 0x1e, 0x1f, 0x18, 0x1d, 0x1f, 0x19,
       12,       0xd9,   0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
       12,       0xdd,   0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
       8,        0xdf,   0x44, 0x73, 0x4b, 0x69, 0x00, 0x0a, 0x02, 0x90,
       17,       0xe0,   0x3b, 0x28, 0x10, 0x16, 0x0c, 0x06, 0x11, 0x28, 0x5c, 0x21, 0x0d, 0x35, 0x13, 0x2c, 0x33, 0x28, 0x0d,
       17,       0xe1,   0x37, 0x28, 0x10, 0x16, 0x0b, 0x06, 0x11, 0x28, 0x5c, 0x21, 0x0d, 0x35, 0x14, 0x2c, 0x33, 0x28, 0x0f,
       17,       0xe2,   0x3b, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x35, 0x44, 0x32, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x2f, 0x0d,
       17,       0xe3,   0x37, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x35, 0x44, 0x32, 0x0c, 0x14, 0x14, 0x36, 0x32, 0x2f, 0x0f,
       17,       0xe4,   0x3b, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x39, 0x44, 0x2e, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x2f, 0x0d,
       17,       0xe5,   0x37, 0x07, 0x12, 0x18, 0x0e, 0x0d, 0x17, 0x39, 0x44, 0x2e, 0x0c, 0x14, 0x14, 0x36, 0x3a, 0x2f, 0x0f,
       8,        0xbb,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

       LCD_COMMON_DLY_FLAG, 10,
       0,        0x11, 
       LCD_COMMON_DLY_FLAG, 50,
       0,        0x29,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
       LCD_COMMON_DLY_FLAG, 100,
};
#endif

/*************************
 *   AXS15231 芯片操作函数
 *************************/
#if LCD_AXS15231_FW_UPGRADE
/**
 * @brief  选中 AXS15231 内部 flash
 * @param  sel 选中的 LCD
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_upgrade_select_flash(uint32_t sel)
{
    int ret = -1;

    uint8_t send_buf[5] = {0xa5, 0x5a, 0xb5, 0xab, 0x00};

    lcd_reset(sel, 0);
    sunxi_lcd_delay_ms(2);
    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(2);

    ret = tp_board_config_i2c_write(send_buf, 5);
    sunxi_lcd_delay_ms(15);

    return ret;
}

/**
 * @brief  等待 AXS15231 内部 flash 就绪
 * @param  wait_times 等待的最大次数
 * @return 0:已就绪, -1:未就绪
 */
static int lcd_axs15231_device_upgrade_flash_ready(uint32_t wait_times)
{
    int ret = 0;

    uint8_t send_buf[12] = {0xab, 0xb5, 0xa5, 0x5a, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05};
    uint8_t read_buf = 0xff;

    while (wait_times--) {
        ret = tp_board_config_i2c_write(send_buf, 12);
        ret = tp_board_config_i2c_read(&read_buf, 1);
        if ((0 == ret) && (0x0 == read_buf)) {
            break;
        }
        sunxi_lcd_delay_ms(10);
    }

    if (0x0 != read_buf) {
        ret = -1;
    } else {
        ret = 0;
    }

    return ret;
}

/**
 * @brief  初始化 AXS15231 内部 flash
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_upgrade_flash_init(void)
{
    int ret = -1;

    uint8_t send_buf[13] = {0xab, 0xb5, 0xa5, 0x5a, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06};
    uint8_t read_buf = 0;

    ret = tp_board_config_i2c_write(send_buf, 12);
    sunxi_lcd_delay_ms(1);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }

    send_buf[4] = 0x00;
    send_buf[5] = 0x01;
    send_buf[6] = 0x00;
    send_buf[7] = 0x01;
    send_buf[8] = 0x00;
    send_buf[9] = 0x00;
    send_buf[10] = 0x00;
    send_buf[11] = 0x9f;
    ret = tp_board_config_i2c_write(send_buf, 12);
    sunxi_lcd_delay_ms(1);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }
    ret = tp_board_config_i2c_read(&read_buf, 1);

    send_buf[4] = 0x00;
    send_buf[5] = 0x01;
    send_buf[6] = 0x00;
    send_buf[7] = 0x00;
    send_buf[8] = 0x00;
    send_buf[9] = 0x00;
    send_buf[10] = 0x00;
    send_buf[11] = 0x06;
    ret = tp_board_config_i2c_write(send_buf, 12);
    sunxi_lcd_delay_ms(1);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }

    send_buf[4] = 0x00;
    send_buf[5] = 0x01;
    send_buf[6] = 0x00;
    send_buf[7] = 0x01;
    send_buf[8] = 0x00;
    send_buf[9] = 0x00;
    send_buf[10] = 0x00;
    send_buf[11] = 0x05;
    ret = tp_board_config_i2c_write(send_buf, 12);
    sunxi_lcd_delay_ms(1);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }
    ret = tp_board_config_i2c_read(&read_buf, 1);

    send_buf[4] = 0x00;
    send_buf[5] = 0x02;
    send_buf[6] = 0x00;
    send_buf[7] = 0x00;
    send_buf[8] = 0x00;
    send_buf[9] = 0x00;
    send_buf[10] = 0x00;
    send_buf[11] = 0x01;
    send_buf[12] = 0x02;
    ret = tp_board_config_i2c_write(send_buf, 13);
    sunxi_lcd_delay_ms(1);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }

    ret = lcd_axs15231_device_upgrade_flash_ready(500);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }

    ret = 0;
exit:
    return ret;
}

/**
 * @brief  擦除 AXS15231 内部 flash
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_upgrade_flash_erase(void)
{
    int ret = -1;

    uint8_t erase_cmd_1[12] = {0xab, 0xb5, 0xa5, 0x5a, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06};
    uint8_t erase_cmd_2[12] = {0xab, 0xb5, 0xa5, 0x5a, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc7};

    ret = tp_board_config_i2c_write(erase_cmd_1, 12);
    sunxi_lcd_delay_ms(1);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }

    ret = tp_board_config_i2c_write(erase_cmd_2, 12);
    sunxi_lcd_delay_ms(1);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }

    ret = lcd_axs15231_device_upgrade_flash_ready(500);
    if (ret < 0) {
        ret = -1;
        goto exit;
    }

    ret = 0;
exit:
    return ret;
}

/**
 * @brief  烧写 AXS15231 内部 flash
 * @param  buf 存放升级固件的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_upgrade_flash_write(uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    uint16_t cur_size = 0;
    uint32_t addr = 0;

    uint8_t *p_addr = (uint8_t *)(&addr);

    uint16_t packet_idx = 1;
    uint16_t packet_len = 256;
    uint16_t packet_sum = buf_len / packet_len;

    uint8_t send_buf[256 + 56] = {0xab, 0xb5, 0xa5, 0x5a};

    if ((NULL == buf) || (0 == buf_len)) {
        ret = -1;
        goto exit;
    }

    if (buf_len % packet_len) {
        packet_sum++;
    }

    while (packet_idx <= packet_sum) {
        send_buf[4] = 0x00;
        send_buf[5] = 0x01;
        send_buf[6] = 0x00;
        send_buf[7] = 0x00;
        send_buf[8] = 0x00;
        send_buf[9] = 0x00;
        send_buf[10] = 0x00;
        send_buf[11] = 0x06;

        ret = tp_board_config_i2c_write(send_buf, 12);
        if (ret < 0) {
            goto exit;
        }

        cur_size = (packet_idx == packet_sum) ? (buf_len - (packet_idx - 1) * packet_len) : packet_len;
        addr = (packet_idx - 1) * packet_len;

        send_buf[4] = (cur_size + 4) >> 8;
        send_buf[5] = (cur_size + 4) & 0xff;

        send_buf[6] = 0x00;
        send_buf[7] = 0x00;
        send_buf[8] = 0x00;
        send_buf[9] = 0x00;
        send_buf[10] = 0x00;
        send_buf[11] = 0x02;
        send_buf[12] = *(p_addr + 2);
        send_buf[13] = *(p_addr + 1);
        send_buf[14] = *(p_addr + 0);

        memcpy(&send_buf[15], buf + addr, cur_size);

        ret = tp_board_config_i2c_write(send_buf, cur_size + 15);
        if (ret < 0) {
            goto exit;
        }

        ret = lcd_axs15231_device_upgrade_flash_ready(500);
        if (ret < 0) {
            goto exit;
        }

        packet_idx++;
    }

    ret = 0;
exit:
    return ret;
}

#if LCD_AXS15231_FW_VERIFY
/**
 * @brief  读取 AXS15231 内部 flash
 * @param  buf 存放数据的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_upgrade_flash_read(uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    uint16_t cur_size = 0;
    uint32_t addr = 0;

    uint8_t *p_addr = (uint8_t *)(&addr);

    uint16_t packet_idx = 1;
    uint16_t packet_len = 256;
    uint16_t packet_sum = buf_len / packet_len;

    uint8_t send_buf[15] = {0xab, 0xb5, 0xa5, 0x5a};
    uint8_t read_buf[256 + 1] = {0};

    if ((NULL == buf) || (0 == buf_len)) {
        ret = -1;
        goto exit;
    }

    if (buf_len % packet_len) {
        packet_sum++;
    }

    while (packet_idx <= packet_sum) {
        cur_size = (packet_idx == packet_sum) ? (buf_len - (packet_idx - 1) * packet_len) : packet_len;
        addr = (packet_idx - 1) * packet_len;

        send_buf[4] = 0x00;
        send_buf[5] = 0x04;

        send_buf[6] = (cur_size + 1) >> 8;
        send_buf[7] = (cur_size + 1) & 0xff;

        send_buf[8] = 0x00;
        send_buf[9] = 0x00;
        send_buf[10] = 0x00;
        send_buf[11] = 0x0b;
        send_buf[12] = *(p_addr + 2);
        send_buf[13] = *(p_addr + 1);
        send_buf[14] = *(p_addr + 0);

        ret = tp_board_config_i2c_write(send_buf, 15);
        if (ret < 0) {
            goto exit;
        }

        ret = tp_board_config_i2c_read(read_buf, cur_size + 1);
        if (ret < 0) {
            goto exit;
        }

        memcpy(buf + addr, &read_buf[1], cur_size);

        packet_idx++;
    }

    ret = 0;
exit:
    return ret;
}

/**
 * @brief  AXS15231 升级完成后的数据校验
 * @param  buf 存放数据的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_upgrade_verify(uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    char *read_buf = NULL;

    read_buf = calloc(1, buf_len);
    if (NULL == read_buf) {
        lcd_fb_wrn("malloc buf fail.");
        ret = -1;
        goto exit;
    }

    ret = lcd_axs15231_device_upgrade_flash_read(read_buf, buf_len);
    if (ret < 0) {
        ret = -1;
        lcd_fb_wrn("upgrade flash read fail.");
        goto exit;
    }

    if (0 != memcmp(buf, read_buf, buf_len)) {
        ret = -1;
        lcd_fb_wrn("upgrade verify fail.");
    } else {
        ret = 0;
    }

exit:
    if (NULL != read_buf) {
        free(read_buf);
    }
    return ret;
}
#endif /* LCD_AXS15231_FW_VERIFY */

/**
 * @brief  带有 flash 的 AXS15231 升级流程函数
 * @param  sel 选中的 LCD
 * @param  buf 升级固件缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_upgrade_process(uint32_t sel, uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    ret = lcd_axs15231_device_upgrade_select_flash(sel);
    if (ret < 0) {
        lcd_fb_wrn("upgrade select flash fail.");
        goto exit;
    }

    ret = lcd_axs15231_device_upgrade_flash_init();
    if (ret < 0) {
        lcd_fb_wrn("upgrade flash init fail.");
        goto exit;
    }

    ret = lcd_axs15231_device_upgrade_flash_erase();
    if (ret < 0) {
        lcd_fb_wrn("upgrade flash erase fail.");
        goto exit;
    }

    ret = lcd_axs15231_device_upgrade_flash_write(buf, buf_len);
    if (ret < 0) {
        lcd_fb_wrn("upgrade flash write fail.");
        goto exit;
    }

#if LCD_AXS15231_FW_VERIFY
    ret = lcd_axs15231_device_upgrade_verify(buf, buf_len);
    if (ret < 0) {
        goto exit;
    }
#endif

    printf("[axs15231] upgrade process success.\n");
    ret = 0;
exit:
    return ret;
}

/**
 * @brief  带有 flash 的 AXS15231 升级前检查
 * @param  buf 升级固件缓冲区
 * @param  buf_len 缓冲区长度
 * @return 1:需要升级, 0:无需升级, -1:错误
 */
static int lcd_axs15231_device_upgrade_check(uint8_t *buf, uint32_t buf_len)
{
    int ret = 0;
    uint8_t cur_version = 0;
    uint8_t upgrade_version = 0;

    uint16_t offset = ((buf[0x0b] << 8) | (buf[0x0c])) + 0x5035;
    if (buf_len < offset) {
        ret = -1;
        goto exit;
    }
    upgrade_version = buf[offset];

    uint8_t try_cnt = 3;
    uint8_t read_version_cmd[11] = {0x5a, 0xa5, 0xab, 0xb5, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x89};

    while (try_cnt--) {
        ret = tp_board_config_i2c_write(read_version_cmd, 11);
        ret = tp_board_config_i2c_read(&cur_version, 1);

        if ((ret == 0) && (0 != cur_version)) {
            break;
        }
        XR_OS_MSleep(10);
    }

    if (ret < 0) {
        lcd_fb_wrn("get cur_version fail.");
        ret = -1;
        goto exit;
    }

    printf("[axs15231] cur_version: 0x%.2x, upgrade_version: 0x%.2x.\n", cur_version, upgrade_version);
    if (cur_version == upgrade_version) {
        ret = 0;
    } else {
        ret = 1;
    }

#if LCD_AXS15231_FW_FORCE
    ret = 1;
#endif
exit:
    return ret;
}
#endif /* LCD_AXS15231_FW_UPGRADE */

#if LCD_AXS15231_FW_DOWNLOAD
/**
 * @brief  选中 AXS15231 内部 param
 * @param  sel 选中的 LCD
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_download_select_param(uint32_t sel)
{
    int ret = -1;

    uint8_t send_buf[5] = {0xa5, 0x5a, 0xb5, 0xab, 0x01};

    lcd_reset(sel, 0);
    sunxi_lcd_delay_ms(2);
    
    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(2);

    ret = tp_board_config_i2c_write(send_buf, 5);
    sunxi_lcd_delay_ms(15);

    return ret;
}

/**
 * @brief  烧写 AXS15231 内部 param
 * @param  buf 存放升级固件的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_download_write_param(uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    uint16_t cur_size = 0;
    uint32_t addr = 0;

    uint8_t *p_addr = (uint8_t *)(&addr);

    uint16_t packet_idx = 1;
    uint16_t packet_len = 256;
    uint16_t packet_sum = buf_len / packet_len;

    uint8_t send_buf[256 + 56] = {0xab, 0xb5, 0xa5, 0x5a};

    if ((NULL == buf) || (0 == buf_len)) {
        ret = -1;
        goto exit;
    }

    if (buf_len % packet_len) {
        packet_sum++;
    }

    while (packet_idx <= packet_sum) {
        cur_size = (packet_idx == packet_sum) ? (buf_len - (packet_idx - 1) * packet_len) : packet_len;
        addr = (packet_idx - 1) * packet_len;

        send_buf[4] = (cur_size) >> 8;
        send_buf[5] = (cur_size) & 0xff;

        send_buf[6] = 0x00;
        send_buf[7] = 0x00;
        send_buf[8] = *(p_addr + 2);
        send_buf[9] = *(p_addr + 1);
        send_buf[10] = *(p_addr + 0);

        memcpy(&send_buf[11], buf + addr, cur_size);

        ret = tp_board_config_i2c_write(send_buf, cur_size + 11);
        if (ret < 0) {
            goto exit;
        }

        packet_idx++;
    }

    ret = 0;
exit:
    return ret;
}

/**
 * @brief  AXS15231 内部 param 升级完成
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_download_finish(void)
{
    int ret = -1;

    uint8_t send_buf[8] = {0xab, 0xb5, 0xa5, 0x5a};
    send_buf[4] = 0x00;
    send_buf[5] = 0x00;
    send_buf[6] = 0x00;
    send_buf[7] = 0x00;

    ret = tp_board_config_i2c_write(send_buf, 8);
    sunxi_lcd_delay_ms(15);

    return ret;
}

#if LCD_AXS15231_FW_VERIFY
/**
 * @brief  读取 AXS15231 内部 param
 * @param  buf 存放升级固件的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_download_read_param(uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    uint16_t cur_size = 0;
    uint32_t addr = 0;

    uint8_t *p_addr = (uint8_t *)(&addr);

    uint16_t packet_idx = 1;
    uint16_t packet_len = 256;
    uint16_t packet_sum = buf_len / packet_len;

    uint8_t send_buf[11] = {0xab, 0xb5, 0xa5, 0x5a};
    uint8_t read_buf[256] = {0};

    if ((NULL == buf) || (0 == buf_len)) {
        ret = -1;
        goto exit;
    }

    if (buf_len % packet_len) {
        packet_sum++;
    }

    while (packet_idx <= packet_sum) {
        cur_size = (packet_idx == packet_sum) ? (buf_len - (packet_idx - 1) * packet_len) : packet_len;
        addr = (packet_idx - 1) * packet_len;

        send_buf[4] = 0x00;
        send_buf[5] = 0x03;

        send_buf[6] = (cur_size) >> 8;
        send_buf[7] = (cur_size) & 0xff;
        send_buf[8] = *(p_addr + 2);
        send_buf[9] = *(p_addr + 1);
        send_buf[10] = *(p_addr + 0);

        ret = tp_board_config_i2c_write(send_buf, 11);
        if (ret < 0) {
            goto exit;
        }

        ret = tp_board_config_i2c_read(read_buf, cur_size);
        if (ret < 0) {
            goto exit;
        }

        memcpy(buf + addr, &read_buf[0], cur_size);

        packet_idx++;
    }

    ret = 0;
exit:
    return ret;
}

/**
 * @brief  AXS15231 升级完成后的数据校验
 * @param  buf 存放数据的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_download_verify(uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    char *read_buf = NULL;

    read_buf = calloc(1, buf_len);
    if (NULL == read_buf) {
        lcd_fb_wrn("malloc buf fail.");
        ret = -1;
        goto exit;
    }

    ret = lcd_axs15231_device_download_read_param(read_buf, buf_len);
    if (ret < 0) {
        ret = -1;
        lcd_fb_wrn("download flash read fail.");
        goto exit;
    }

    if (0 != memcmp(buf, read_buf, buf_len)) {
        ret = -1;
        lcd_fb_wrn("download verify fail.");
    } else {
        ret = 0;
    }

exit:
    if (NULL != read_buf) {
        free(read_buf);
    }
    return ret;
}
#endif /* LCD_AXS15231_FW_VERIFY */

/**
 * @brief  不带 flash 的 AXS15231 升级流程函数
 * @param  sel 选中的 LCD
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_download_process(uint32_t sel, uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    ret = lcd_axs15231_device_download_select_param(sel);
    if (ret < 0) {
        lcd_fb_wrn("download select param fail.");
        goto exit;
    }

    ret = lcd_axs15231_device_download_write_param(buf, buf_len);
    if (ret < 0) {
        lcd_fb_wrn("download write param fail.");
        goto exit;
    }

#if LCD_AXS15231_FW_VERIFY
    ret = lcd_axs15231_device_download_verify(buf, buf_len);
    if (ret < 0) {
        goto exit;
    }
#endif

    ret = lcd_axs15231_device_download_finish();
    if (ret < 0) {
        lcd_fb_wrn("download finish fail.");
        goto exit;
    }

    printf("[axs15231] download process success.\n");
    ret = 0;
exit:
    return ret;
}
#endif /* LCD_AXS15231_FW_DOWNLOAD */

/**
 * @brief  AXS15231 升级入口, 不带 flash 走 download, 否则走 upgrade
 * @param  sel 选中的 LCD
 * @param  mode 升级模式, 0:upgrade, 1:download
 * @return 0:成功, -1:失败
 */
static int lcd_axs15231_device_firmware_update(uint32_t sel, uint8_t mode, uint8_t *buf, uint32_t buf_len)
{
    int ret = -1;

    if ((NULL == buf) || (0 == buf_len)) {
        ret = -1;
        goto exit;
    }

    if (0 == mode) {
#if LCD_AXS15231_FW_UPGRADE
        ret = lcd_axs15231_device_upgrade_check(buf, buf_len);
        if (1 == ret) {
            ret = lcd_axs15231_device_upgrade_process(sel, buf, buf_len);
        }
#endif
    } else if (1 == mode) {
#if LCD_AXS15231_FW_DOWNLOAD
        ret = lcd_axs15231_device_download_process(sel, buf, buf_len);
#endif
    }

exit:
    return ret;
}

/*************************
 *   LCD 驱动操作函数
 *************************/
#ifdef CONFIG_SPILCD_SUPPORT_QSPI
static int axs_write_by_spi(int port, unsigned char val, unsigned char *para, unsigned psize)
{
    hal_spi_master_status_t ret;
    hal_spi_master_transfer_t t;
    memset(&t, 0, sizeof(hal_spi_master_transfer_t));

    unsigned q_len = psize + AXS_QSPI_CMD_LEN;
    unsigned char *q_buff = malloc(q_len);
    if (q_buff == NULL) {
        return -1;
    }
    memset(q_buff, 0, q_len);
    q_buff[0] = 0x02;
    q_buff[2] = val;
    if ((psize != 0) && (para != NULL)) {
        memcpy(&q_buff[4], para, psize);
    }

    t.tx_buf = q_buff;
    t.tx_len = q_len;
    t.rx_buf = NULL;
    t.rx_len = 0;
    t.dummy_byte = 0;
    t.tx_single_len = q_len;
    t.tx_nbits = SPI_NBITS_QUAD;

    ret = hal_spi_xfer(port, &t, 1);
    free(q_buff);
    return (int)ret;
}

static unsigned char *axs15231_buf_alloc(unsigned sel, unsigned fb)
{
    unsigned long int screensize = 0;
    struct disp_panel_para info[LCD_FB_MAX];

    if (s_axs_buf != NULL) {
        lcd_fb_wrn("axs buff has been alloced.");
        return NULL;
    }

    if (bsp_disp_get_panel_info(sel, &info[sel])) {
        lcd_fb_wrn("get panel info fail.");
        return NULL;
    }

    screensize = info[sel].lcd_x * info[sel].lcd_y;
    if ((info[sel].lcd_pixel_fmt == LCDFB_FORMAT_RGB_565)
        || (info[sel].lcd_pixel_fmt == LCDFB_FORMAT_BGR_565)) {
        screensize <<= 1;
    } else {
        screensize *= 3;
    }

    if (fb > 0) {
        screensize *= 2;
    }

    screensize += AXS_QSPI_CMD_LEN;
    s_axs_buf = lcd_fb_dma_malloc(screensize);
    if (NULL == s_axs_buf) {
        lcd_fb_wrn("malloc pixel fail.");
        return NULL;
    }
    lcd_fb_inf("pixel_addr = %x, size = %d.", s_axs_buf, screensize);
    memset(s_axs_buf, 0, screensize);

    s_axs_screen_size = screensize;

    // AXS QSPI COMMAND
    s_axs_buf[0] = 0x32;
    s_axs_buf[1] = 0x00;
    s_axs_buf[2] = 0x2c;
    s_axs_buf[3] = 0x00;

    return &s_axs_buf[4];
}

static int axs15231_buf_free(unsigned int sel)
{
    if (s_axs_buf == NULL) {
        lcd_fb_wrn("axs free NULL.");
        return -1;
    }

    lcd_fb_dma_free(s_axs_buf);
    s_axs_buf = NULL;
    s_axs_screen_size = 0;

    return 0;
}

static int axs15231_send(unsigned int sel, void *buf, unsigned int len)
{
    hal_spi_master_status_t ret;
    hal_spi_master_transfer_t t;

    memset(&t, 0, sizeof(hal_spi_master_transfer_t));
    if (s_axs_buf == NULL) {
        lcd_fb_wrn("axs buff NULL.");
        return -1;
    }

    uint8_t *tx_buf = (uint8_t *)buf;
    tx_buf -= AXS_QSPI_CMD_LEN;
    tx_buf[0] = 0x32;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0x2c;
    tx_buf[3] = 0x00;

    t.tx_buf = tx_buf;
    t.tx_len = len + AXS_QSPI_CMD_LEN;
    t.tx_single_len	= AXS_QSPI_CMD_LEN;
    t.bits_per_word	= 16;
    t.dummy_byte = 0;
    t.tx_nbits = SPI_NBITS_QUAD;
    if (ALIGN_DMA_BUF_SIZE < t.tx_len) {
        printf("SORRY, QSPI can only send all data at once! Please increase SPI_ALIGN_DMA_BUF_SIZE.");
        return -1;
    }
    hal_dcache_clean((unsigned long)tx_buf, t.tx_len);

    ret = hal_spi_xfer(s_axs_spi_port, &t, 1);
    return (int)ret;
}

static int lcd_set_addr_win(unsigned int sel, int x, int y, int width, int height);
static void axs15231_lcd_fb_black_screen(unsigned int sel)
{
    unsigned char *pixel = NULL;
    struct disp_panel_para info[LCD_FB_MAX];
    struct fb_info fb_info;

    if (bsp_disp_get_panel_info(sel, &info[sel])) {
        lcd_fb_wrn("get panel info fail!\n");
        return;
    }

    pixel = axs15231_buf_alloc(sel, 0);
    memset(&fb_info, 0, sizeof(struct fb_info));
    fb_info.screen_base = pixel;
    fb_info.var.xres = info[sel].lcd_x;
    fb_info.var.yres = info[sel].lcd_y;
    lcd_set_addr_win(sel, 0, 0, info[sel].lcd_x, info[sel].lcd_y);
    bsp_disp_lcd_set_layer(sel, &fb_info);

    axs15231_buf_free(sel);
}
#endif

static void address(unsigned int sel, int x, int y, int width, int height)
{
    int x_end = 0;
    int y_end = 0;

    x_end = x + width - 1;
    y_end = y + height - 1;

#ifdef CONFIG_SPILCD_SUPPORT_QSPI
    // set coloum address
    unsigned char para[4] = {0};
    para[0] = ((x >> 8) & 0xff);
    para[1] = (x & 0xff);
    para[2] = ((x_end >> 8) & 0xff);
    para[3] = (x_end & 0xff);
    axs_write_by_spi(s_axs_spi_port, 0x2a, para, 4);

    // set row address
    para[0] = ((y >> 8) & 0xff);
    para[1] = (y & 0xff);
    para[2] = ((y_end >> 8) & 0xff);
    para[3] = (y_end & 0xff);
    axs_write_by_spi(s_axs_spi_port, 0x2b, para, 4);
#else
    // set coloum address
    sunxi_lcd_cmd_write(sel, 0x2a);
    sunxi_lcd_para_write(sel, (x >> 8) & 0xff);
    sunxi_lcd_para_write(sel, x & 0xff);
    sunxi_lcd_para_write(sel, (x_end >> 8) & 0xff);
    sunxi_lcd_para_write(sel, x_end & 0xff);

    // set row address
    sunxi_lcd_cmd_write(sel, 0x2b);
    sunxi_lcd_para_write(sel, (y >> 8) & 0xff);
    sunxi_lcd_para_write(sel, y & 0xff);
    sunxi_lcd_para_write(sel, (y_end >> 8) & 0xff);
    sunxi_lcd_para_write(sel, y_end & 0xff);
#endif
}

static void LCD_panel_init(unsigned int sel)
{
    if (bsp_disp_get_panel_info(sel, &info[sel])) {
        lcd_fb_wrn("get panel info fail: %d.", sel);
        return;
    }
    s_axs_spi_port = info[sel].lcd_spi_bus_num;

    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(10);
    lcd_reset(sel, 0);
    sunxi_lcd_delay_ms(10);
    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(100);

    /**
     * 1. 初始化最高只支持 24MHz 速率
     * 2. 初始化完成后, 即发送完 0x11 0x29 后, 才支持 96MHz 速率
     */
    sunxi_lcd_clk_set(sel, 24);

#ifdef CONFIG_SPILCD_SUPPORT_QSPI
    #if (0 == LCD_AXS15231_PARAM_INIT_TYPE)
        sunxi_lcd_delay_ms(10);
        axs_write_by_spi(s_axs_spi_port, 0x11, NULL, 0);
        sunxi_lcd_delay_ms(50);
        axs_write_by_spi(s_axs_spi_port, 0x29, NULL, 0);
        sunxi_lcd_delay_ms(400);
    #elif (3 == LCD_AXS15231_PARAM_INIT_TYPE)
        uint8_t *reg_param = (uint8_t *)s_axs15231_reg_param;
        uint32_t data_size = 0;

        for (uint32_t i = 0; i < sizeof(s_axs15231_reg_param);) {
            data_size = ((lcd_common_reg_para_t *)&reg_param[i])->data_size;
            if (LCD_COMMON_DLY_FLAG == data_size) {
                sunxi_lcd_delay_ms(((lcd_common_reg_para_t *)&reg_param[i])->dly);
                data_size = 0;
            } else {
                axs_write_by_spi(s_axs_spi_port, ((lcd_common_reg_para_t *)&reg_param[i])->cmd, ((lcd_common_reg_para_t *)&reg_param[i])->data, data_size);
            }
            i += 2 + data_size;
            data_size = 0;
        }
    #endif
#else
    #if (0 == LCD_AXS15231_PARAM_INIT_TYPE)
        sunxi_lcd_delay_ms(10);
        sunxi_lcd_cmd_write(sel, 0x11);
        sunxi_lcd_delay_ms(50);
        sunxi_lcd_cmd_write(sel, 0x29);
//        sunxi_lcd_delay_ms(400);
    #elif ((1 == LCD_AXS15231_PARAM_INIT_TYPE) || (2 == LCD_AXS15231_PARAM_INIT_TYPE))
        lcd_common_reg_init(sel, s_axs15231_reg_param, sizeof(s_axs15231_reg_param));
    #endif
#endif

    sunxi_lcd_clk_set(sel, 0);
}

static void LCD_panel_exit(unsigned int sel)
{
#ifdef CONFIG_SPILCD_SUPPORT_QSPI
    // display off
    axs_write_by_spi(s_axs_spi_port, 0x28, NULL, 0);
    sunxi_lcd_delay_ms(20);

    axs_write_by_spi(s_axs_spi_port, 0x10, NULL, 0);
    sunxi_lcd_delay_ms(20);
#else
    // display off
    sunxi_lcd_cmd_write(sel, 0x28);
    sunxi_lcd_delay_ms(20);

    sunxi_lcd_cmd_write(sel, 0x10);
    sunxi_lcd_delay_ms(20);
#endif
}

static int lcd_blank(unsigned int sel, unsigned int en)
{
    if (en) {
        // display off
#ifdef CONFIG_SPILCD_SUPPORT_QSPI
        axs_write_by_spi(s_axs_spi_port, 0x28, NULL, 0);
        sunxi_lcd_delay_ms(50);

        axs_write_by_spi(s_axs_spi_port, 0x10, NULL, 0);
        sunxi_lcd_delay_ms(50);
#else
        sunxi_lcd_cmd_write(sel, 0x28);
        sunxi_lcd_delay_ms(50);

        sunxi_lcd_cmd_write(sel, 0x10);
        sunxi_lcd_delay_ms(50);
        lcd_reset(sel, 0);
#endif
    } else {
        // display on
#ifdef CONFIG_SPILCD_SUPPORT_QSPI
    #if 1
        // 带 flash 版本
        lcd_reset(sel, 1);
        sunxi_lcd_delay_ms(10);
        lcd_reset(sel, 0);
        sunxi_lcd_delay_ms(10);
        lcd_reset(sel, 1);
        sunxi_lcd_delay_ms(100);

        axs_write_by_spi(s_axs_spi_port, 0x11, NULL, 0);
        sunxi_lcd_delay_ms(50);

        axs_write_by_spi(s_axs_spi_port, 0x29, NULL, 0);
        sunxi_lcd_delay_ms(50);
    #else
        // 不带 flash 版本
        axs_write_by_spi(s_axs_spi_port, 0x11, NULL, 0);
        sunxi_lcd_delay_ms(3);

        axs_write_by_spi(s_axs_spi_port, 0x11, NULL, 0);
        sunxi_lcd_delay_ms(50);

        axs_write_by_spi(s_axs_spi_port, 0x29, NULL, 0);
        sunxi_lcd_delay_ms(100);
    #endif
#else
    #if 1
        // 带 flash 版本
        lcd_reset(sel, 1);
        sunxi_lcd_delay_ms(10);
        lcd_reset(sel, 0);
        sunxi_lcd_delay_ms(10);
        lcd_reset(sel, 1);
        sunxi_lcd_delay_ms(100);

        sunxi_lcd_cmd_write(sel, 0x11);
        sunxi_lcd_delay_ms(50);
        
        sunxi_lcd_cmd_write(sel, 0x29);
        sunxi_lcd_delay_ms(50);
        s_axs_set_win = true;
        lcd_fb_black_screen(sel);
        
    #else 
        // 不带 flash 版本
        sunxi_lcd_cmd_write(sel, 0x11);
        sunxi_lcd_delay_ms(3);

        sunxi_lcd_cmd_write(sel, 0x11);
        sunxi_lcd_delay_ms(50);
        
        sunxi_lcd_cmd_write(sel, 0x29);
        sunxi_lcd_delay_ms(100);
    #endif
#endif
    }

    return 0;
}

static void LCD_firmware_update(unsigned int sel)
{
    int ret = -1;
    int fd = -1;

    uint8_t file_type = 0; // 0:tf卡, 1:lfs
    char *buf = NULL;
    struct stat file_stat;

    ret = stat(LCD_AXS15231_FIRMWARE_FATFS_PATH, &file_stat);
    if (0 != ret) {
        lcd_fb_wrn("can not find %s.", LCD_AXS15231_FIRMWARE_FATFS_PATH);
        ret = stat(LCD_AXS15231_FIRMWARE_LFS_PATH, &file_stat);
        if (0 != ret) {
            lcd_fb_wrn("can not find %s.", LCD_AXS15231_FIRMWARE_LFS_PATH);
            return;
        }
        file_type = 1;
    }

    if (0 == file_stat.st_size) {
        return;
    }

    buf = (char *)calloc(1, file_stat.st_size);
    if (NULL == buf) {
        lcd_fb_wrn("buf malloc fail.");
        return;
    }

    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(20);
    lcd_reset(sel, 0);
    sunxi_lcd_delay_ms(20);
    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(130);

    ret = tp_board_config_parse();
    ret = tp_board_config_hw_init();

    if (0 == file_type) {
        fd = open(LCD_AXS15231_FIRMWARE_FATFS_PATH, O_RDONLY);
    } else if (1 == file_type) {
        fd = open(LCD_AXS15231_FIRMWARE_LFS_PATH, O_RDONLY);
    }

    if (fd < 0) {
        free(buf);
        return;
    }
    ret = read(fd, buf, file_stat.st_size);

    ret = lcd_axs15231_device_firmware_update(sel, 0, buf, file_stat.st_size);
    if (ret < 0) {
        lcd_fb_wrn("firmware update fail.");
    }

    close(fd);
    free(buf);
}

static void LCD_panel_resume(u32 sel)
{
	/*目前不做处理可以正常使用，先保留接口，后续有问题更改*/
}

static s32 LCD_open_flow(u32 sel)
{
    lcd_fb_here;

    s_axs_set_win = true;
    s_sel = sel;

    /* open lcd power, and delay 50ms */
    LCD_OPEN_FUNC(sel, LCD_power_on, 10);

    if (!s_init) {
         /* lcd firmware update, and delay 50ms */
        LCD_OPEN_FUNC(sel, LCD_firmware_update, 0);

        /* lcd init, and delay 200ms */
        LCD_OPEN_FUNC(sel, LCD_panel_init, 0);

#ifdef CONFIG_SPILCD_SUPPORT_QSPI
        LCD_OPEN_FUNC(sel, axs15231_lcd_fb_black_screen, 0);
#else
        LCD_OPEN_FUNC(sel, lcd_fb_black_screen, 0);
#endif
        /* open lcd backlight, and delay 0ms */
        LCD_OPEN_FUNC(sel, LCD_bl_open, 0);
    } else {
        LCD_OPEN_FUNC(sel, LCD_panel_resume, 10);
    }
   
    s_init = 1;

    return 0;
}

static s32 LCD_close_flow(u32 sel)
{
    lcd_fb_here;

    /* close lcd backlight, and delay 0ms */
    LCD_CLOSE_FUNC(sel, LCD_bl_close, 50);
    /* close lcd panel, than delay 10ms */
#ifdef CONFIG_COMPONENTS_PM
    if (dev_pm_is_goto_hibernation() == 1) {//进入深休再关闭屏幕
        LCD_CLOSE_FUNC(sel, LCD_panel_exit, 10);
    }
#endif
    /* close lcd power, and delay 500ms */
    LCD_CLOSE_FUNC(sel, LCD_power_off, 10);//这个使用的是扩展io的供电

    return 0;
}

static void LCD_power_on(u32 sel)
{
    /* config lcd_power pin to open lcd power0 */
    lcd_fb_here;

    sunxi_lcd_power_enable(sel, 0);
    sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
    lcd_fb_here;

#ifdef CONFIG_COMPONENTS_PM
    if (dev_pm_is_goto_hibernation()) {
		dev_mcu_lcd_reset_set(0);
	}
#endif

    /* config lcd_power pin to close lcd power0 */
    sunxi_lcd_power_disable(sel, 0);
}

static void LCD_bl_open(u32 sel)
{
    sunxi_lcd_pwm_enable(sel);
    /* config lcd_bl_en pin to open lcd backlight */
    sunxi_lcd_backlight_enable(sel);
    // hal_gpio_set_direction(GPIOA(8), GPIO_DIRECTION_OUTPUT);
    // hal_gpio_pinmux_set_function(GPIOA(8), GPIO_MUXSEL_OUT);
    // hal_gpio_set_pull(GPIOA(8), GPIO_PULL_UP);
    // hal_gpio_set_driving_level(GPIOA(8), GPIO_DRIVING_LEVEL3);

    // hal_gpio_set_data(GPIOA(8), GPIO_DATA_HIGH);
    // printf("LCD_bl_openLCD_bl_openLCD_bl_openLCD_bl_open\n");

    lcd_fb_here;
}

static void LCD_bl_close(u32 sel)
{
    /* config lcd_bl_en pin to close lcd backlight */
    sunxi_lcd_backlight_disable(sel);
    sunxi_lcd_pwm_disable(sel);

    lcd_fb_here;
}

void axs1523_lcd_pwm_bl_enable(int is_enable)
{
	if (is_enable) {
		LCD_bl_open(s_sel);
		// lcd_cs_set(0);
 	} else {
		// lcd_cs_set(1);
		LCD_bl_close(s_sel);
	}
}

/* sel: 0:lcd0; 1:lcd1 */
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
    lcd_fb_here;
    return 0;
}

static int lcd_set_var(unsigned int sel, struct fb_info *p_info)
{
    return 0;
}

static int lcd_set_addr_win(unsigned int sel, int x, int y, int width, int height)
{
    if (s_axs_set_win) {
        address(sel, x, y, width, height);
        s_axs_set_win = false;
    }

#ifdef CONFIG_SPILCD_SUPPORT_QSPI
    axs_write_by_spi(s_axs_spi_port, 0x2c, NULL, 0);
#else
    sunxi_lcd_cmd_write(sel, 0x2c);
#endif

    return 0;
}

struct __lcd_panel axs15231_panel = {
    /**
     * 注意: 结构体 name 成员的名字必须和 sys_config.fex 中的 lcd_driver_name 一致
     */
    .name = "axs15231",
    .func = {
        .cfg_open_flow = LCD_open_flow,
        .cfg_close_flow = LCD_close_flow,
        .lcd_user_defined_func = LCD_user_defined_func,
        .blank = lcd_blank,
        .set_var = lcd_set_var,
        .set_addr_win = lcd_set_addr_win,
#ifdef CONFIG_SPILCD_SUPPORT_QSPI
        .qbuf_alloc = axs15231_buf_alloc,
        .qbuf_free = axs15231_buf_free,
        .qbuf_send = axs15231_send,
#endif
    },
};
