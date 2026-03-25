/*
 * drivers/video/fbdev/sunxi/disp2/disp/lcd/s2003t46/s2003t46g.c
 *
 * Copyright (c) 2007-2018 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
**/
/*
[lcd0]
lcd_used            = 1

lcd_driver_name     = "ili9488_cpu"
lcd_backlight       = 155
lcd_if              = 1
lcd_x               = 320
lcd_y               = 320
lcd_width           = 71
lcd_height          = 70
lcd_rb_swap         = 1
lcd_pwm_used        = 1
lcd_pwm_ch          = 6
lcd_pwm_freq        = 5000
lcd_pwm_pol         = 1
lcd_cpu_mode = 0
lcd_cpu_te = 0

;st7796
;lcd_cpu_if = 14
;lcd_dclk_freq = 24
;lcd_hbp             = 100
;lcd_ht              = 778
;lcd_hspw            = 50
;lcd_vbp             = 8
;lcd_vt              = 496
;lcd_vspw            = 4

lcd_cpu_if = 14
lcd_dclk_freq = 26
;
lcd_hbp             = 100
lcd_ht             = 590
;Width of horizontal synchronization signal
lcd_hspw            = 50
lcd_vbp             = 8
lcd_vt              = 336
lcd_vspw            = 4

lcd_lvds_if         = 0
lcd_lvds_colordepth = 1
lcd_lvds_mode       = 0
lcd_frm             = 2
lcd_io_phase        = 0x0000
lcd_gamma_en        = 0
lcd_bright_curve_en = 0
lcd_cmap_en         = 0

deu_mode            = 0
lcdgamma4iep        = 22
smart_color         = 90

;bl
lcd_bl_en                = port:PA21<1><2><3><1>

;reset 
;lcd_gpio_0               = port:PB3<1><0><3><0>
;cs
lcd_gpio_1               = port:PA0<1><0><3><0>
;data[0:7]
lcd_gpio_2               = port:PA1<8><0><3><0>
lcd_gpio_3               = port:PA2<8><0><3><0>
lcd_gpio_4               = port:PA3<8><0><3><0>
lcd_gpio_5               = port:PA4<8><0><3><0>
lcd_gpio_6               = port:PA5<8><0><3><0>
lcd_gpio_7               = port:PA11<8><0><3><0>
lcd_gpio_8               = port:PA10<8><0><3><0>
lcd_gpio_9               = port:PA8<8><0><3><0>
;WR
lcd_gpio_10              = port:PA6<7><0><3><0>
;RD
lcd_gpio_11              = port:PA7<7><0><3><0>
;RS
lcd_gpio_12              = port:PA9<7><0><3><0>
;TE
;lcd_gpio_13              = port:PB1<1><0><3><0>
 */

#include "ili9488_cpu.h"

//#define CPU_TRI_MODE

#define DBG_INFO(format, args...) (printf("[ILI9488 LCD INFO] LINE:%04d-->%s:"format, __LINE__, __func__, ##args))
#define DBG_ERR(format, args...) (printf("[ILI9488 LCD ERR] LINE:%04d-->%s:"format, __LINE__, __func__, ##args))
// #define panel_reset(val) sunxi_lcd_gpio_set_value(sel, 0, val)
// #define lcd_cs(val)  sunxi_lcd_gpio_set_value(sel, 0, val)

static void lcd_panel_ili9488_init(u32 sel, struct disp_panel_para *info);
static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

extern uint8_t dev_power_is_goto_hibernation();
extern void dev_mcu_lcd_reset_set(int val);

static u32 s_sel = 0;

static void LCD_cfg_panel_info(struct panel_extend_para *info)
{
#if 1
	u32 i = 0, j = 0;
	u32 items;
	u8 lcd_gamma_tbl[][2] = {
		//{input value, corrected value}
		{0, 0},     {15, 15},   {30, 30},   {45, 45},   {60, 60},
		{75, 75},   {90, 90},   {105, 105}, {120, 120}, {135, 135},
		{150, 150}, {165, 165}, {180, 180}, {195, 195}, {210, 210},
		{225, 225}, {240, 240}, {255, 255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
		{
		{LCD_CMAP_G0, LCD_CMAP_B1, LCD_CMAP_G2, LCD_CMAP_B3},
		{LCD_CMAP_B0, LCD_CMAP_R1, LCD_CMAP_B2, LCD_CMAP_R3},
		{LCD_CMAP_R0, LCD_CMAP_G1, LCD_CMAP_R2, LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3, LCD_CMAP_G2, LCD_CMAP_B1, LCD_CMAP_G0},
		{LCD_CMAP_R3, LCD_CMAP_B2, LCD_CMAP_R1, LCD_CMAP_B0},
		{LCD_CMAP_G3, LCD_CMAP_R2, LCD_CMAP_G1, LCD_CMAP_R0},
		},
	};
	

	items = sizeof(lcd_gamma_tbl) / 2;
	for (i = 0; i < items - 1; i++) {
		u32 num = lcd_gamma_tbl[i + 1][0] - lcd_gamma_tbl[i][0];

		for (j = 0; j < num; j++) {
			u32 value = 0;

			value =
			    lcd_gamma_tbl[i][1] +
			    ((lcd_gamma_tbl[i + 1][1] - lcd_gamma_tbl[i][1]) *
			     j) /
				num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] =
			    (value << 16) + (value << 8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items - 1][1] << 16) +
				   (lcd_gamma_tbl[items - 1][1] << 8) +
				   lcd_gamma_tbl[items - 1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));
#endif
}

static int s_init = 0;

static void address(unsigned int sel, int x, int y, int width, int height)
{
	sunxi_lcd_cpu_write_index(sel, 0x2B); /* Set row address */
	sunxi_lcd_cpu_write_data(sel, (y >> 8) & 0xff);
	sunxi_lcd_cpu_write_data(sel, y & 0xff);
	sunxi_lcd_cpu_write_data(sel, (height >> 8) & 0xff);
	sunxi_lcd_cpu_write_data(sel, height & 0xff);
	sunxi_lcd_cpu_write_index(sel, 0x2A); /* Set coloum address */
	sunxi_lcd_cpu_write_data(sel, (x >> 8) & 0xff);
	sunxi_lcd_cpu_write_data(sel, x & 0xff);
	sunxi_lcd_cpu_write_data(sel, (width >> 8) & 0xff);
	sunxi_lcd_cpu_write_data(sel, width & 0xff);
}

static void LCD_panel_resume(u32 sel)
{
	lcd_cs_set(0);
	struct disp_panel_para info = {0};
	memset(&info, 0, sizeof(struct disp_panel_para));
	bsp_disp_get_panel_info(sel, &info);

	sunxi_lcd_cpu_write_index(sel, 0x2c); /* Display ON */
	if (info.lcd_cpu_mode == LCD_CPU_AUTO_MODE) {
		sunxi_lcd_cpu_set_auto_mode(sel);
	}
	
}

static s32 LCD_open_flow(u32 sel)
{
	s_sel = sel;
	
	LCD_OPEN_FUNC(sel, LCD_power_on, 120);
#ifdef CPU_TRI_MODE
	LCD_OPEN_FUNC(sel, LCD_panel_init, 100);
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 50);
#else
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);
#endif
	if (!s_init) {
		LCD_OPEN_FUNC(sel, LCD_panel_init, 50);
		LCD_OPEN_FUNC(sel, LCD_bl_open, 0);
	} else {
		LCD_OPEN_FUNC(sel, LCD_panel_resume, 50);
	}
	s_init = 1;
exit:
	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 20);
#ifdef CPU_TRI_MODE
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 10);
	LCD_CLOSE_FUNC(sel, LCD_panel_exit, 50);
#else
	if (dev_power_is_goto_hibernation()) {
		LCD_CLOSE_FUNC(sel, LCD_panel_exit, 10);
	}
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 10);
#endif
	LCD_CLOSE_FUNC(sel, LCD_power_off, 0);

	return 0;
}

static void LCD_power_on(u32 sel)
{
	/*config lcd_power pin to open lcd power0 */
	sunxi_lcd_power_enable(sel, 0);
	sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
	/*lcd_cs, active low */
	// lcd_cs(1);
	lcd_cs_set(1);
	// sunxi_lcd_delay_ms(10);
	/*lcd_rst, active hight */

	if (dev_power_is_goto_hibernation()) {
		// lcd_cs_set(0);
		dev_mcu_lcd_reset_set(0);
	}

	sunxi_lcd_pin_cfg(sel, 0);
	/*config lcd_power pin to close lcd power0 */
	sunxi_lcd_power_disable(sel, 0);
}

static void LCD_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
	/*config lcd_bl_en pin to open lcd backlight */
	sunxi_lcd_backlight_enable(sel);
}
// extern int dev_extio_set_bl(int is_enable);
static void LCD_bl_close(u32 sel)
{
	/*config lcd_bl_en pin to close lcd backlight */
	sunxi_lcd_backlight_disable(sel);
	sunxi_lcd_pwm_disable(sel);
}

void ili9488_lcd_pwm_bl_enable(int is_enable)
{
	if (is_enable) {
		LCD_bl_open(s_sel);
		// lcd_cs_set(0);
 	} else {
		// lcd_cs_set(1);
		LCD_bl_close(s_sel);
	}
}

void lcd_cs_init()
{
	hal_gpio_set_pull(GPIOA(0), GPIO_PULL_UP);
    hal_gpio_set_direction(GPIOA(0), GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_data(GPIOA(0), GPIO_DATA_HIGH);
    hal_gpio_pinmux_set_function(GPIOA(0),GPIO_MUXSEL_OUT);
}

void lcd_cs_set(int val)
{
	hal_gpio_set_data(GPIOA(0), val);
}

/*static int bootup_flag = 0;*/
static void LCD_panel_init(u32 sel)
{
	struct disp_panel_para *info =
	    malloc(sizeof(struct disp_panel_para));

	DBG_INFO("\n");
	
	bsp_disp_get_panel_info(sel, info);
	lcd_panel_ili9488_init(sel, info);

	disp_sys_free(info);
	return;
}

static void LCD_panel_exit(u32 sel)
{
	sunxi_lcd_cpu_write_index(0, 0x28);
	sunxi_lcd_cpu_write_index(0, 0x10);
}


// __xip_rodata static uint8_t s_ili9488_reg_param[] = {
//        1,        0x36,   0x48,   //*
//        1,        0x3a,   0x55,      //*
//        1,        0xb0,   0x00,          //*
//        2,        0xb1,   0xb0, 0x11,  //*
//        1,        0xb4,   0x02,   //*
//        2,        0xb6,   0x02, 0x02,//*
//        1,        0xb7,   0xc6,
//         2,       0xbe,   0x00, 0x04,//*
//        2,        0xc0,   0x09, 0x09,  //*
//        1,        0xc1,   0x45,      //*
//        3,        0xc5,   0x00, 0x55, 0x80,  //*
//        15,       0xe0,   0x00, 0x07, 0x0f, 0x04, 0x11, 0x06, 0x39, 0x67, 0x4E, 0x02, 0x0A, 0x09, 0x2D, 0x33, 0x0f,  //*
//        15,       0xe1,   0x00, 0x0F, 0x14, 0x03, 0x10, 0x06, 0x33, 0x34, 0x45, 0x06, 0x0E, 0x0C, 0x2A, 0x30, 0x0f,   //*
//         1,       0xe9,  0x00,
//         4,       0xf7, 0xa9, 0x51, 0x2c, 0x82,
//         0,        0x21,
//        0,        0x11,
//        LCD_COMMON_DLY_FLAG, 120,
//        0,        0x29,
// };

static void lcd_panel_ili9488_init(u32 sel, struct disp_panel_para *info)
{
	unsigned int rotate;
	DBG_INFO("\n");
	/*lcd_cs, active low */
	lcd_cs_init();
	// lcd_cs(0);
	lcd_cs_set(0);

	//扩展io的复位
	//extern void dev_mcu_lcd_reset(void);
	//dev_mcu_lcd_reset();

	// sunxi_lcd_cpu_write_index(sel, 0x35);
	// sunxi_lcd_cpu_write_data(sel, 0x00);

	sunxi_lcd_cpu_write_index(0, 0x28);
	sunxi_lcd_cpu_write_index(0, 0x10);
	sunxi_lcd_delay_ms(120);
	sunxi_lcd_cpu_write_index(sel, 0x11);
	sunxi_lcd_delay_ms(120);

	sunxi_lcd_cpu_write_index(sel, 0x36);
	sunxi_lcd_cpu_write_data(sel, 0x40);

	sunxi_lcd_cpu_write_index(sel, 0x3A);
	if (info->lcd_cpu_if == 14)
		sunxi_lcd_cpu_write_data(sel, 0x55);
	else
		sunxi_lcd_cpu_write_data(sel, 0x66);

	sunxi_lcd_cpu_write_index(sel, 0xB0);
	sunxi_lcd_cpu_write_data(sel, 0x00);

	sunxi_lcd_cpu_write_index(sel, 0xB1);
	sunxi_lcd_cpu_write_data(sel, 0xb0);
	sunxi_lcd_cpu_write_data(sel, 0x11);

	sunxi_lcd_cpu_write_index(sel, 0xB4); /* Display Inversion Control */
	sunxi_lcd_cpu_write_data(sel, 0x02);  /* 1-dot */

	sunxi_lcd_cpu_write_index(sel, 0xB6);
	sunxi_lcd_cpu_write_data(sel, 0x02);
	sunxi_lcd_cpu_write_data(sel, 0x02);

	sunxi_lcd_cpu_write_index(sel, 0xB7);
	sunxi_lcd_cpu_write_data(sel, 0xC6);

	sunxi_lcd_cpu_write_index(sel, 0xBe);
	sunxi_lcd_cpu_write_data(sel, 0x00);
	sunxi_lcd_cpu_write_data(sel, 0x04);

	sunxi_lcd_cpu_write_index(sel, 0xC0);
	sunxi_lcd_cpu_write_data(sel, 0x09);
	sunxi_lcd_cpu_write_data(sel, 0x09);

	sunxi_lcd_cpu_write_index(sel, 0xC1);
	sunxi_lcd_cpu_write_data(sel, 0x45);

	sunxi_lcd_cpu_write_index(sel, 0xC5);
	sunxi_lcd_cpu_write_data(sel, 0x00);
	sunxi_lcd_cpu_write_data(sel, 0x55);
	sunxi_lcd_cpu_write_data(sel, 0x80);

	sunxi_lcd_cpu_write_index(sel, 0xE0);
	sunxi_lcd_cpu_write_data(sel, 0x00);
	sunxi_lcd_cpu_write_data(sel, 0x07);
	sunxi_lcd_cpu_write_data(sel, 0x0F);
	sunxi_lcd_cpu_write_data(sel, 0x04);
	sunxi_lcd_cpu_write_data(sel, 0x11);
	sunxi_lcd_cpu_write_data(sel, 0x06);
	sunxi_lcd_cpu_write_data(sel, 0x39);
	sunxi_lcd_cpu_write_data(sel, 0x67);
	sunxi_lcd_cpu_write_data(sel, 0x4E);
	sunxi_lcd_cpu_write_data(sel, 0x02);
	sunxi_lcd_cpu_write_data(sel, 0x0A);
	sunxi_lcd_cpu_write_data(sel, 0x09);
	sunxi_lcd_cpu_write_data(sel, 0x2D);
	sunxi_lcd_cpu_write_data(sel, 0x33);
	sunxi_lcd_cpu_write_data(sel, 0x0F);

	sunxi_lcd_cpu_write_index(sel, 0xE1);
	sunxi_lcd_cpu_write_data(sel, 0x00);
	sunxi_lcd_cpu_write_data(sel, 0x0F);
	sunxi_lcd_cpu_write_data(sel, 0x14);
	sunxi_lcd_cpu_write_data(sel, 0x03);
	sunxi_lcd_cpu_write_data(sel, 0x10);
	sunxi_lcd_cpu_write_data(sel, 0x06);
	sunxi_lcd_cpu_write_data(sel, 0x33);
	sunxi_lcd_cpu_write_data(sel, 0x34);
	sunxi_lcd_cpu_write_data(sel, 0x45);
	sunxi_lcd_cpu_write_data(sel, 0x06);
	sunxi_lcd_cpu_write_data(sel, 0x0E);
	sunxi_lcd_cpu_write_data(sel, 0x0C);
	sunxi_lcd_cpu_write_data(sel, 0x2A);
	sunxi_lcd_cpu_write_data(sel, 0x30);
	sunxi_lcd_cpu_write_data(sel, 0x0F);

	sunxi_lcd_cpu_write_index(sel, 0xE9);
	sunxi_lcd_cpu_write_data(sel, 0x00);

	sunxi_lcd_cpu_write_index(sel, 0xF7);
	sunxi_lcd_cpu_write_data(sel, 0xa9);
	sunxi_lcd_cpu_write_data(sel, 0x51);
	sunxi_lcd_cpu_write_data(sel, 0x2c);
	sunxi_lcd_cpu_write_data(sel, 0x82);

	sunxi_lcd_cpu_write_index(sel, 0x21);
	sunxi_lcd_cpu_write_index(sel, 0x11);
	sunxi_lcd_delay_ms(120);

#if defined(CPU_TRI_MODE)
	/* enable te, mode 0 */
	sunxi_lcd_cpu_write_index(0, 0x35);
	sunxi_lcd_cpu_write_data(0, 0x00);

	sunxi_lcd_cpu_write_index(0, 0x44);
	sunxi_lcd_cpu_write_data(0, 0x00);
	sunxi_lcd_cpu_write_data(0, 0x80);
#endif

	sunxi_lcd_cpu_write_index(sel, 0x29);

	address(sel, 0, 0, info->lcd_x - 1, info->lcd_y - 1);
	sunxi_lcd_cpu_write_index(sel, 0x2c); /* Display ON */
	

	if (info->lcd_cpu_mode == LCD_CPU_AUTO_MODE)
		sunxi_lcd_cpu_set_auto_mode(sel);
}


static int lcd_set_esd_info(struct disp_lcd_esd_info *p_info)
{
	if (!p_info)
		return -1;
	p_info->level = 1;
	p_info->freq = 60;
	p_info->esd_check_func_pos = 1;
	return 0;
}


/**
 * @name       :lcd_esd_check
 * @brief      :check if panel is ok
 * @param[IN]  :sel:index of dsi
 * @param[OUT] :none
 * @return     :0 if ok, else not ok
 */
static s32 lcd_esd_check(u32 sel)
{
	// tcon_reset(sel);
	// u8 data[5] = {0};
	// sunxi_lcd_cpu_read_index(sel, 0x09, data, 5);
	// u32 param = 0;
	// param = ((data[1] << 24) | (data[2] << 16) | (data[3] << 8) | (data[4]));
	// printf("read 09H = %x\n", param);
	
	return 0;
}

/**
 * @name       :lcd_reset_panel
 * @brief      :reset panel step
 * @param[IN]  :sel:index of dsi
 * @param[OUT] :none
 * @return     :0
 */
static s32 lcd_reset_panel(u32 sel)
{
	/*reset tcon*/
	LCD_panel_exit(sel);
	sunxi_lcd_delay_ms(120);
	LCD_power_off(sel);
	sunxi_lcd_delay_ms(200);
	LCD_panel_init(sel);
	sunxi_lcd_delay_ms(20);
	return 0;
}

/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
struct __lcd_panel ili9488_cpu_panel = {
	.name = "ili9488_cpu",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.esd_check = lcd_esd_check,
		.reset_panel = lcd_reset_panel,
		.set_esd_info = lcd_set_esd_info,
	},
};