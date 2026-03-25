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

#include "spilcd.h"
#ifdef CONFIG_DRIVERS_SPILCD

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <hal_lcd_fb.h>

static u32 g_screen_index = 0;

extern int bsp_disp_lcd_set_layer(unsigned int disp, struct fb_info *p_info);
extern int bsp_disp_get_screen_width(unsigned int disp);
extern int bsp_disp_get_screen_height(unsigned int disp);

static void fb_init(struct fb_info *p_info, struct camera_preview_buf dst_buf,
		uint32_t yoffset, uint32_t line_length)
{
    p_info->screen_base = dst_buf.addr;
    p_info->var.xres = dst_buf.width;
    p_info->var.yres = dst_buf.height;
    p_info->var.xoffset = 0;
    p_info->var.yoffset = yoffset;
    p_info->fix.line_length = line_length;
}

void cp_spilcd_get_sizes(uint32_t *width, uint32_t *height)
{
    *width = bsp_disp_get_screen_width(g_screen_index);
    *height = bsp_disp_get_screen_height(g_screen_index);
}

int cp_spilcd_pan_display(struct camera_preview_buf dst_buf, int fbindex)
{
    struct fb_info info;
    uint32_t yoffset, line_length;

	yoffset = dst_buf.height * fbindex;
	line_length = dst_buf.size / dst_buf.height;

    memset(&info, 0, sizeof(struct fb_info));
    fb_init(&info, dst_buf, yoffset, line_length);

    hal_dcache_clean((unsigned long)(dst_buf.addr + yoffset * line_length), dst_buf.size);
    bsp_disp_lcd_set_layer(g_screen_index, &info);
 
	bsp_disp_lcd_wait_for_vsync(g_screen_index);

    return 0;
}

#endif
