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


#ifndef __PANEL_H__
#define __PANEL_H__
#include "../include.h"
#include "lcd_source.h"
#include "../disp_display.h"
#include "../disp_lcd.h"

struct __lcd_panel {
	char name[32];
	struct disp_lcd_panel_fun func;
};

extern struct __lcd_panel *panel_array[];

struct sunxi_lcd_drv {
	struct sunxi_disp_source_ops src_ops;
};

extern int sunxi_disp_get_source_ops(struct sunxi_disp_source_ops *src_ops);
int lcd_init(void);
#ifdef CONFIG_LCD_SUPPORT_KLD35512
extern struct __lcd_panel kld35512_panel;
#endif

#ifdef CONFIG_LCD_SUPPORT_KLD39501
extern struct __lcd_panel kld39501_panel;
#endif

#ifdef CONFIG_LCD_SUPPORT_KLD2844B
extern struct __lcd_panel kld2844b_panel;
#endif

#ifdef CONFIG_LCD_SUPPORT_NV3029S
extern struct __lcd_panel nv3029s_panel;
#endif

#ifdef CONFIG_LCD_SUPPORT_JLT35031C
extern struct __lcd_panel jlt35031c_panel;
#endif

#ifdef CONFIG_LCD_SUPPORT_ST7796_SPI
extern struct __lcd_panel st7796_spi_panel;
#endif

#ifdef CONFIG_LCD_SUPPORT_ST7789V3
extern struct __lcd_panel st7789v3_panel;
#endif

#ifdef CONFIG_LCD_SUPPORT_AXS15231
extern struct __lcd_panel axs15231_panel;
#endif

#endif
