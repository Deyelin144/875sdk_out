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



#include "include.h"
#include "lcd_fb_intf.h"
#include "dev_lcd_fb.h"
#ifndef __DISP_LCD_H__
#define __DISP_LCD_H__


#define SUPPORT_MAX_LCD LCD_FB_MAX
#define LCD_GPIO_NUM 8
#define LCD_POWER_NUM 4
#define LCD_POWER_STR_LEN 32
#define LCD_GPIO_REGU_NUM 3
#define LCD_GPIO_SCL (LCD_GPIO_NUM-2)
#define LCD_GPIO_SDA (LCD_GPIO_NUM-1)

struct disp_lcd_cfg {
	bool lcd_used;

	bool lcd_bl_en_used;
	struct disp_gpio_set_t lcd_bl_en;
	int lcd_bl_gpio_hdl;
	char lcd_bl_en_power[LCD_POWER_STR_LEN];

	u32 lcd_power_used[LCD_POWER_NUM];
	char lcd_power[LCD_POWER_NUM][LCD_POWER_STR_LEN];

	/* 0: invalid, 1: gpio, 2: regulator */
	u32 lcd_fix_power_used[LCD_POWER_NUM];
	char lcd_fix_power[LCD_POWER_NUM][LCD_POWER_STR_LEN];

	bool lcd_gpio_used[LCD_GPIO_NUM];
	struct disp_gpio_set_t lcd_gpio[LCD_GPIO_NUM];
	int gpio_hdl[LCD_GPIO_NUM];
	char lcd_gpio_power[LCD_GPIO_REGU_NUM][LCD_POWER_STR_LEN];

	char lcd_pin_power[LCD_GPIO_REGU_NUM][LCD_POWER_STR_LEN];
	struct disp_gpio_set_t lcd_spi_dc_pin;
	int spi_dc_pin_hdl;

	u32 backlight_bright;
	/*
	 * IEP-drc backlight dimming rate:
	 * 0 -256 (256: no dimming; 0: the most dimming)
	 */
	u32 backlight_dimming;
	u32 backlight_curve_adjust[101];

	u32 lcd_bright;
	u32 lcd_contrast;
	u32 lcd_saturation;
	u32 lcd_hue;
};

s32 disp_init_lcd(struct dev_lcd_fb_t *p_info);
s32 disp_exit_lcd(void);
s32 disp_lcd_set_bright(struct lcd_fb_device *lcd, u32 bright);
s32 disp_lcd_get_bright(struct lcd_fb_device *lcd);
s32 disp_lcd_gpio_init(struct lcd_fb_device *lcd);
s32 disp_lcd_gpio_exit(struct lcd_fb_device *lcd);
s32 disp_lcd_gpio_set_direction(struct lcd_fb_device *lcd, u32 io_index,
				u32 direction);
s32 disp_lcd_gpio_get_value(struct lcd_fb_device *lcd, u32 io_index);
s32 disp_lcd_gpio_set_value(struct lcd_fb_device *lcd, u32 io_index, u32 data);
s32 disp_lcd_is_enabled(struct lcd_fb_device *lcd);
struct lcd_fb_device *disp_get_lcd(u32 disp);
void lcd_fb_black_screen(unsigned int sel);

#endif
