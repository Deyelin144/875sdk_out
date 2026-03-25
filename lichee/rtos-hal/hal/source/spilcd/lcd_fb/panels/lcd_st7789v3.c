#include "compiler.h"
#include "lcd_common_func.h"
#include "lcd_st7789v3.h"

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);
static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

extern void dev_mcu_lcd_reset_set(int val);
// 复位脚使用 lcd_gpio_0s
// #define lcd_reset(sel, val) sunxi_lcd_gpio_set_value(sel, 0, val)
#define lcd_reset(sel, val) dev_mcu_lcd_reset_set(val)
static struct disp_panel_para info[LCD_FB_MAX] = {0};
static u32 s_sel = 0;
static int s_init = 0;  //初始化标志
static bool s_axs_set_win = true;
__xip_rodata static uint8_t s_st7789v3_reg_param[] = {
    LCD_COMMON_DLY_FLAG,  120,
    0,        0x11,
    LCD_COMMON_DLY_FLAG,  120,
    1,        0x35, 0x00, // TE on
    // 1,        0x36, 0x60, //硬件旋转
    1,        0x3A, 0x55,
    5,        0xB2, 0x0c, 0x0c, 0x00, 0x33, 0x33,
    1,        0xB7, 0x46,
    1,        0xBB, 0x1B,
    1,        0xC0, 0x2C,
    1,        0xC2, 0x01,
    1,        0xC3, 0x00,
    1,        0xC4, 0x20,
    1,        0xC6, 0x15, // 50帧
    2,        0xD0, 0xA4, 0xA1,
    1,        0xD6, 0xA1,
    14,       0xE0, 0xF0, 0x00, 0x06, 0x06, 0x07, 0x05, 0x30, 0x44, 0x48, 0x38, 0x11, 0x10, 0x2e, 0x34,
    14,       0xE1, 0xF0, 0x0a, 0x0e, 0x0d, 0x0b, 0x27, 0x2f, 0x44, 0x47, 0x35, 0x12, 0x12, 0x2C, 0x32,
    0,        0x21,
    0,        0x29,
    0,        0x2C,
};

static void address(unsigned int sel, int x, int y, int width, int height)
{
    // set coloum address
    sunxi_lcd_cmd_write(sel, 0x2a);
    sunxi_lcd_para_write(sel, (x >> 8) & 0xff);
    sunxi_lcd_para_write(sel, x & 0xff);
    sunxi_lcd_para_write(sel, (width >> 8) & 0xff);
    sunxi_lcd_para_write(sel, width & 0xff);

    // set row address
    sunxi_lcd_cmd_write(sel, 0x2b);
    sunxi_lcd_para_write(sel, (y >> 8) & 0xff);
    sunxi_lcd_para_write(sel, y & 0xff);
    sunxi_lcd_para_write(sel, (height >> 8) & 0xff);
    sunxi_lcd_para_write(sel, height & 0xff);
}

static void LCD_panel_init(unsigned int sel)
{
    if (bsp_disp_get_panel_info(sel, &info[sel])) {
        lcd_fb_wrn("get panel info fail: %d.", sel);
        return;
    }

    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(20);
    lcd_reset(sel, 0);
    sunxi_lcd_delay_ms(20);
    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(120);

    lcd_common_reg_init(sel, s_st7789v3_reg_param, sizeof(s_st7789v3_reg_param));
}

static void LCD_panel_exit(unsigned int sel)
{
    // display off
    sunxi_lcd_cmd_write(sel, 0x28);
    sunxi_lcd_delay_ms(20);

    sunxi_lcd_cmd_write(sel, 0x10);
    sunxi_lcd_delay_ms(20);
}

static int lcd_blank(unsigned int sel, unsigned int en)
{
    if (en) {
        // display off
        sunxi_lcd_cmd_write(sel, 0x28);
    } else {
        // display on
        sunxi_lcd_cmd_write(sel, 0x29);
        s_axs_set_win = true;
    }

    return 0;
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
    LCD_OPEN_FUNC(sel, LCD_power_on, 50);

    if (!s_init) {
        /* open lcd power, than delay 200ms */
        // LCD_OPEN_FUNC(sel, LCD_panel_init, 200);  //由于在uboot中已经复位过屏幕,这里不再复位,否则会看到屏幕闪一下

        // LCD_OPEN_FUNC(sel, lcd_fb_black_screen, 50);   // lcd_fb_black_screen函数中会写一屏空白数据，导致屏幕看起来会感觉闪一下，这里注释掉
        /* open lcd backlight, and delay 0ms */
        LCD_OPEN_FUNC(sel, LCD_bl_open, 0);
    }else {
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
    /* open lcd power, than delay 200ms */
    LCD_CLOSE_FUNC(sel, LCD_panel_exit, 10);
    /* close lcd power, and delay 500ms */
    LCD_CLOSE_FUNC(sel, LCD_power_off, 10);

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

    /* config lcd_power pin to close lcd power0 */
    sunxi_lcd_power_disable(sel, 0);
}

static void LCD_bl_open(u32 sel)
{
    sunxi_lcd_pwm_enable(sel);
    /* config lcd_bl_en pin to open lcd backlight */
    sunxi_lcd_backlight_enable(sel);

    lcd_fb_here;
}

static void LCD_bl_close(u32 sel)
{
    /* config lcd_bl_en pin to close lcd backlight */
    sunxi_lcd_backlight_disable(sel);
    sunxi_lcd_pwm_disable(sel);

    lcd_fb_here;
}

void st7789v3_lcd_pwm_bl_enable(int is_enable)
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
    sunxi_lcd_cmd_write(sel, 0x2c);
    return 0;
}

struct __lcd_panel st7789v3_panel = {
    /**
     * 注意: 结构体 name 成员的名字必须和 sys_config.fex 中的 lcd_driver_name 一致
     */
    .name = "st7789v3",
    .func = {
        .cfg_open_flow = LCD_open_flow,
        .cfg_close_flow = LCD_close_flow,
        .lcd_user_defined_func = LCD_user_defined_func,
        .blank = lcd_blank,
        .set_var = lcd_set_var,
        .set_addr_win = lcd_set_addr_win,
    },
};
