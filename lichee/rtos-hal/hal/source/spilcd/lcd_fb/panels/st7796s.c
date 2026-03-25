/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/*
[lcd_fb0]
lcd_used            = 1
lcd_model_name      = "spilcd"
lcd_driver_name     = "st7796_spi"
lcd_x               = 320
lcd_y               = 480
lcd_width           = 60
lcd_height          = 95
lcd_data_speed      = 96
lcd_pwm_used        = 1
lcd_pwm_ch          = 0
lcd_pwm_freq        = 5000
lcd_pwm_pol         = 1
lcd_if              = 1
lcd_pixel_fmt       = 10
lcd_dbi_fmt         = 2
lcd_dbi_clk_mode    = 1
lcd_dbi_te          = 0
fb_buffer_num       = 2
lcd_dbi_if          = 2
lcd_rgb_order       = 0
lcd_fps             = 60
lcd_spi_bus_num     = 1
lcd_frm             = 1
lcd_gamma_en        = 1
lcd_backlight       = 100

lcd_power_num       = 0
lcd_gpio_regu_num   = 0
lcd_bl_percent_num  = 0

lcd_gpio_0          = port:PA13<1><0><2><0>
*/

#include "st7796s.h"

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);
static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);
static struct disp_panel_para info[LCD_FB_MAX];

// RESET PIN lcd_gpio_0
#define lcd_reset(sel, val) sunxi_lcd_gpio_set_value(sel, 0, val)

static void address(unsigned int sel, int x, int y, int width, int height)
{
	sunxi_lcd_cmd_write(sel, 0x2B); /* Set row address */
	sunxi_lcd_para_write(sel, (y >> 8) & 0xff);
	sunxi_lcd_para_write(sel, y & 0xff);
	sunxi_lcd_para_write(sel, (height >> 8) & 0xff);
	sunxi_lcd_para_write(sel, height & 0xff);
	sunxi_lcd_cmd_write(sel, 0x2A); /* Set coloum address */
	sunxi_lcd_para_write(sel, (x >> 8) & 0xff);
	sunxi_lcd_para_write(sel, x & 0xff);
	sunxi_lcd_para_write(sel, (width >> 8) & 0xff);
	sunxi_lcd_para_write(sel, width & 0xff);
}

static void LCD_panel_init(unsigned int sel)
{
	unsigned int rotate;


	if (bsp_disp_get_panel_info(sel, &info[sel])) {
		lcd_fb_wrn("get panel info fail!\n");
		return;
	}

    lcd_reset(sel, 0);
    sunxi_lcd_delay_ms(100);
    lcd_reset(sel, 1);
    sunxi_lcd_delay_ms(50);

	sunxi_lcd_delay_ms(120);
	sunxi_lcd_cmd_write(sel, 0x11);
	sunxi_lcd_delay_ms(120);

	sunxi_lcd_cmd_write(sel, 0xF0);
	sunxi_lcd_para_write(sel, 0xC3);

	sunxi_lcd_cmd_write(sel, 0xF0);
	sunxi_lcd_para_write(sel, 0x96);

	sunxi_lcd_cmd_write(sel, 0x36);
	sunxi_lcd_para_write(sel, 0x48);

	sunxi_lcd_cmd_write(sel, 0x3A); /* Interface Pixel Format */
	/* 55----RGB565;66---RGB666 */
	if (info[sel].lcd_pixel_fmt == LCDFB_FORMAT_RGB_565 ||
	    info[sel].lcd_pixel_fmt == LCDFB_FORMAT_BGR_565) {
		sunxi_lcd_para_write(sel, 0x55);
	} else if (info[sel].lcd_pixel_fmt < LCDFB_FORMAT_RGB_888) {
		sunxi_lcd_para_write(sel, 0x66);
		if (info[sel].lcd_pixel_fmt == LCDFB_FORMAT_BGRA_8888 ||
		    info[sel].lcd_pixel_fmt == LCDFB_FORMAT_BGRX_8888 ||
		    info[sel].lcd_pixel_fmt == LCDFB_FORMAT_ABGR_8888 ||
		    info[sel].lcd_pixel_fmt == LCDFB_FORMAT_XBGR_8888) {
		}
	} else {
		sunxi_lcd_para_write(sel, 0x66);
	}

	sunxi_lcd_cmd_write(sel, 0xB4); /* Display Inversion Control */
	sunxi_lcd_para_write(sel, 0x01);  /* 1-dot */

	sunxi_lcd_cmd_write(sel, 0xB7);
	sunxi_lcd_para_write(sel, 0xC6);

	sunxi_lcd_cmd_write(sel, 0xE8);
	sunxi_lcd_para_write(sel, 0x40);
	sunxi_lcd_para_write(sel, 0x8A);
	sunxi_lcd_para_write(sel, 0x00);
	sunxi_lcd_para_write(sel, 0x00);
	sunxi_lcd_para_write(sel, 0x29);
	sunxi_lcd_para_write(sel, 0x19);
	sunxi_lcd_para_write(sel, 0xA5);
	sunxi_lcd_para_write(sel, 0x33);

	sunxi_lcd_cmd_write(sel, 0xC1);
	sunxi_lcd_para_write(sel, 0x06);

	sunxi_lcd_cmd_write(sel, 0xC2);
	sunxi_lcd_para_write(sel, 0xa7);

	sunxi_lcd_cmd_write(sel, 0xC5);
	sunxi_lcd_para_write(sel, 0x18);

	sunxi_lcd_cmd_write(sel, 0xE0);
	sunxi_lcd_para_write(sel, 0xF0);
	sunxi_lcd_para_write(sel, 0x09);
	sunxi_lcd_para_write(sel, 0x0B);
	sunxi_lcd_para_write(sel, 0x06);
	sunxi_lcd_para_write(sel, 0x04);
	sunxi_lcd_para_write(sel, 0x15);
	sunxi_lcd_para_write(sel, 0x2F);
	sunxi_lcd_para_write(sel, 0x54);
	sunxi_lcd_para_write(sel, 0x42);
	sunxi_lcd_para_write(sel, 0x3C);
	sunxi_lcd_para_write(sel, 0x17);
	sunxi_lcd_para_write(sel, 0x14);
	sunxi_lcd_para_write(sel, 0x18);
	sunxi_lcd_para_write(sel, 0x1B);

	sunxi_lcd_cmd_write(sel, 0xE1);
	sunxi_lcd_para_write(sel, 0xF0);
	sunxi_lcd_para_write(sel, 0x09);
	sunxi_lcd_para_write(sel, 0x0B);
	sunxi_lcd_para_write(sel, 0x06);
	sunxi_lcd_para_write(sel, 0x04);
	sunxi_lcd_para_write(sel, 0x03);
	sunxi_lcd_para_write(sel, 0x2D);
	sunxi_lcd_para_write(sel, 0x43);
	sunxi_lcd_para_write(sel, 0x42);
	sunxi_lcd_para_write(sel, 0x3B);
	sunxi_lcd_para_write(sel, 0x16);
	sunxi_lcd_para_write(sel, 0x14);
	sunxi_lcd_para_write(sel, 0x17);
	sunxi_lcd_para_write(sel, 0x1B);

	sunxi_lcd_cmd_write(sel, 0xF0);
	sunxi_lcd_para_write(sel, 0x3C);

	sunxi_lcd_cmd_write(sel, 0xF0);
	sunxi_lcd_para_write(sel, 0x69);
	sunxi_lcd_delay_ms(120);

#if defined(CPU_TRI_MODE)
	/* enable te, mode 0 */
	sunxi_lcd_cmd_write(0, 0x35);
	sunxi_lcd_para_write(0, 0x00);

	sunxi_lcd_cmd_write(0, 0x44);
	sunxi_lcd_para_write(0, 0x00);
	sunxi_lcd_para_write(0, 0x80);
#endif

	sunxi_lcd_cmd_write(sel, 0x29);

	sunxi_lcd_cmd_write(sel, 0x36);
	/*MY MX MV ML RGB MH 0 0*/
	if (info->lcd_x > info->lcd_y)
		rotate = 0x20;
	else
		rotate = 0x48;
	sunxi_lcd_para_write(sel, rotate |= 0x08); /*horizon scrren*/

	address(sel, 0, 0, info->lcd_x - 1, info->lcd_y - 1);
	sunxi_lcd_cmd_write(sel, 0x2c); /* Display ON */
}

static void LCD_panel_exit(unsigned int sel)
{
	sunxi_lcd_cmd_write(sel, 0x28);
	sunxi_lcd_delay_ms(20);
	sunxi_lcd_cmd_write(sel, 0x10);
	sunxi_lcd_delay_ms(20);
	sunxi_lcd_pin_cfg(sel, 0);
}

static s32 LCD_open_flow(u32 sel)
{
	lcd_fb_here;
	/* open lcd power, and delay 50ms */
	LCD_OPEN_FUNC(sel, LCD_power_on, 50);
	/* open lcd power, than delay 200ms */
	LCD_OPEN_FUNC(sel, LCD_panel_init, 20);

	LCD_OPEN_FUNC(sel, lcd_fb_black_screen, 50);
	/* open lcd backlight, and delay 0ms */
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);

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

static int lcd_set_var(unsigned int sel, struct fb_info *p_info)
{
	return 0;
}

static int lcd_blank(unsigned int sel, unsigned int en)
{
	if (en)
		sunxi_lcd_cmd_write(sel, 0x28);
	else
		sunxi_lcd_cmd_write(sel, 0x29);

	sunxi_lcd_cmd_write(sel, 0x2c); /* Display ON */
	return 0;
}

static int lcd_set_addr_win(unsigned int sel, int x, int y, int width, int height)
{
	address(sel, x, y, width, height);
	sunxi_lcd_cmd_write(sel, 0x2c); /* Display ON */
	return 0;
}

struct __lcd_panel st7796_spi_panel = {
    /* panel driver name, must mach the name of lcd_drv_name in sys_config.fex
       */
	.name = "st7796_spi",
	.func = {
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.blank = lcd_blank,
		.set_var = lcd_set_var,
		.set_addr_win = lcd_set_addr_win,
		},
};
