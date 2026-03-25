/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY'S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS'SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY'S TECHNOLOGY.
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <hal_cache.h>
#include <hal_mem.h>
#include <hal_log.h>
#include <hal_cmd.h>
#include <hal_lcd_fb.h>

/* Color Depth: 16 (RGB565), 24 (RGB888), 32 (ARGB8888) */
#define LCD_COLOR_DEPTH 16
#define LCD_BPP         (LCD_COLOR_DEPTH / 8)

static uint32_t width;
static uint32_t height;
static long int screensize = 0;
static char *fbsmem_start  = 0;

extern int hal_msleep(unsigned int msecs);

static void lcdfb_fb_init(uint32_t yoffset, struct fb_info *p_info)
{
	p_info->screen_base = fbsmem_start;
	p_info->var.xres    = width;
	p_info->var.yres    = height;
	p_info->var.xoffset = 0;
	p_info->var.yoffset = yoffset;
}

int show_rgb(unsigned int sel, uint8_t color)
{
	int i = 0, ret = -1;
	struct fb_info fb_info;

#if defined(LCD_COLOR_DEPTH) && (LCD_COLOR_DEPTH == 16)
	//R5G6B5 -> G3(l)B5,G3(H)R5 (little endian)
	unsigned char color_yellow[2] = { 0x0, 0xff };
	unsigned char color_red[2]    = { 0x0, 0xf8 };
	unsigned char color_green[2]  = { 0xe0, 0x07 };
	unsigned char color_blue[2]   = { 0x1f, 0x0 };
#else
	// A8R8G8B8->B8G8R8A8 (little endian)
	unsigned char color_yellow[4] = { 0x0, 0xff, 0xff, 0x0 };
	unsigned char color_red[4]    = { 0x0, 0x0, 0xff, 0x0 };
	unsigned char color_green[4]  = { 0x0, 0xff, 0x0, 0x0 };
	unsigned char color_blue[4]   = { 0xff, 0x0, 0x0, 0x0 };
#endif

	width  = bsp_disp_get_screen_width(sel);
	height = bsp_disp_get_screen_height(sel);

	screensize   = width * LCD_BPP * height;
	fbsmem_start = hal_malloc_coherent(screensize);

	// hal_log_info("width = %d, height = %d, screensize = %d, fbsmem_start = %x\n",
	// 		width, height, screensize, fbsmem_start);

	memset(fbsmem_start, 0, screensize);
	for (i = 0; i < screensize / LCD_BPP; ++i) {
		if (1 == color) {
			memcpy(fbsmem_start + i * LCD_BPP, color_red, LCD_BPP);
		} else if (2 == color) {
			memcpy(fbsmem_start + i * LCD_BPP, color_green, LCD_BPP);
		} else if (3 == color) {
			memcpy(fbsmem_start + i * LCD_BPP, color_blue, LCD_BPP);
		} else {
			memcpy(fbsmem_start + i * LCD_BPP, color_yellow, LCD_BPP);
		}
	}

	bsp_disp_lcd_set_bright(sel, 0);

	memset(&fb_info, 0, sizeof(struct fb_info));
	lcdfb_fb_init(0, &fb_info);
	hal_dcache_clean((unsigned long)fbsmem_start, screensize);
	bsp_disp_lcd_set_layer(sel, &fb_info);

	hal_free_coherent(fbsmem_start);
	return ret;
}

static int cmd_test_spilcd(int argc, char **argv)
{
	uint8_t ret;
	uint8_t color = 0;

	hal_log_info("Run spilcd hal layer test case\n");

	if (2 == argc) {
		color = atoi(argv[1]);
	}

	do {
		if (1 == color) {
			printf(">>>>>>>>>>>>>>>> red <<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		} else if (2 == color) {
			printf(">>>>>>>>>>>>>>>> green <<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		} else if (3 == color) {
			printf(">>>>>>>>>>>>>>>> blue <<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		} else {
			printf(">>>>>>>>>>>>>>>> yellow <<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		}

		ret = show_rgb(0, color);
		color++;
		hal_msleep(2000);
	} while (color < 4);

	hal_log_info("spilcd test finish\n");

	return ret;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_test_spilcd, test_spilcd, spilcd hal APIs tests)