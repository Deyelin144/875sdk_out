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


#ifndef CONFIG_DRIVER_SYSCONFIG
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <hal_clk.h>
#include <hal_gpio.h>
#include "../lcd_fb/lcd_fb_intf.h"
#include "disp_board_config.h"

struct property_t g_lcd0_config[] = {
	{
		.name = "lcd_used",
		.type = PROPERTY_INTGER,
		.v.value = 1,
	},
	{
		.name = "lcd_driver_name",
		.type = PROPERTY_STRING,
		.v.str = "kld2844b",
	},
	{
		.name = "lcd_backlight",
		.type = PROPERTY_INTGER,
		.v.value = 100,
	},
	{
		.name = "lcd_if",
		.type = PROPERTY_INTGER,
		.v.value = 1,
	},
	{
		.name = "lcd_dbi_if",
		.type = PROPERTY_INTGER,
		.v.value = 4,
	},
	{
		.name = "lcd_data_speed",
		.type = PROPERTY_INTGER,
		.v.value = 60,
	},
	{
		.name = "lcd_spi_bus_num",
		.type = PROPERTY_INTGER,
		.v.value = 1,
	},
	{
		.name = "lcd_x",
		.type = PROPERTY_INTGER,
		.v.value = 240,
	},
	{
		.name = "lcd_y",
		.type = PROPERTY_INTGER,
		.v.value = 320,
	},
	{
		.name = "lcd_width",
		.type = PROPERTY_INTGER,
		.v.value = 60,
	},
	{
		.name = "lcd_height",
		.type = PROPERTY_INTGER,
		.v.value = 95,
	},
	{
		.name = "lcd_pwm_used",
		.type = PROPERTY_INTGER,
		.v.value = 1,
	},
	{
		.name = "lcd_pwm_ch",
		.type = PROPERTY_INTGER,
		.v.value = 6,
	},
	{
		.name = "lcd_pwm_freq",
		.type = PROPERTY_INTGER,
		.v.value = 5000,
	},
	{
		.name = "lcd_pwm_pol",
		.type = PROPERTY_INTGER,
		.v.value = 0,
	},
	{
		.name = "lcd_pixel_fmt",
		.type = PROPERTY_INTGER,
		.v.value = 0,
	},
	{
		.name = "lcd_dbi_fmt",
		.type = PROPERTY_INTGER,
		.v.value = 3,
	},
	{
		.name = "lcd_rgb_order",
		.type = PROPERTY_INTGER,
		.v.value = 0,
	},
	{
		.name = "lcd_frm",
		.type = PROPERTY_INTGER,
		.v.value = 1,
	},
	{
		.name = "lcd_gamma_en",
		.type = PROPERTY_INTGER,
		.v.value = 1,
	},
	{
		.name = "lcd_buffer_num",
		.type = PROPERTY_INTGER,
		.v.value = 2,
	},
	{
		.name = "lcd_dbi_te",
		.type = PROPERTY_INTGER,
		.v.value = 1,
	},
	{
		.name = "lcd_fps",
		.type = PROPERTY_INTGER,
		.v.value = 60,
	},

//gpio
	// dc
	{
		.name = "lcd_spi_dc_pin",
		.type = PROPERTY_GPIO,
		.v.gpio_list = {
			.gpio = GPIOA(19),
			.mul_sel = GPIO_DIRECTION_OUTPUT,
			.pull = 0,
			.drv_level = 3,
			.data = 1,
		},
	},

};

struct property_t g_lcd1_config[] = {
	{
		.name = "lcd_used",
		.type = PROPERTY_INTGER,
		.v.value = 0,
	},
};

u32 g_lcd0_config_len = sizeof(g_lcd0_config) / sizeof(struct property_t);
u32 g_lcd1_config_len = sizeof(g_lcd1_config) / sizeof(struct property_t);
#endif
