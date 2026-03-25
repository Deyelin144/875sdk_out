/**
 * @file disp2.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../sunxifb.h"

#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../sunximem.h"
#include "../sunxig2d.h"
#include <hal_lcd_fb.h>
/*********************
 *      DEFINES
 *********************/
#define LV_SPILCD_SCREEN_INFEX 0
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      STRUCTURES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int lv_spilcd_init(void);
static int lv_spilcd_exit(void);
static int lv_spilcd_flush(void);
static int lv_spilcd_get_sizes(uint32_t *width, uint32_t *height);
static char *lv_spilcd_get_buff(void);
/**********************
 *  STATIC VARIABLES
 **********************/
static unsigned spilcd_wight = 0;
static unsigned spilcd_height = 0;
static char *fbsmem_start = 0;
static unsigned smem_len = 0;
static struct fb_info spilcd_info;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_disp_ops lv_spilcd_ops = {
    .init = lv_spilcd_init,
    .exit = lv_spilcd_exit,
    .flush = lv_spilcd_flush,
    .get_sizes = lv_spilcd_get_sizes,
    .get_buf = lv_spilcd_get_buff,
};
/**********************
 *   STATIC FUNCTIONS
 **********************/
static int lv_spilcd_init(void)
{
    spilcd_wight = bsp_disp_get_screen_width(LV_SPILCD_SCREEN_INFEX);
    spilcd_height = bsp_disp_get_screen_height(LV_SPILCD_SCREEN_INFEX);

    smem_len = spilcd_wight * spilcd_height * LV_COLOR_DEPTH / 8;

    fbsmem_start = sunxifb_mem_alloc(smem_len * 2, "spilcd_buff");
    if (fbsmem_start == NULL) {
        return -1;
    }
    memset(fbsmem_start, 0, smem_len * 2);

    memset(&spilcd_info, 0, sizeof(struct fb_info));

    spilcd_info.screen_base = fbsmem_start + smem_len;
    spilcd_info.var.xres = spilcd_wight;
    spilcd_info.var.yres = spilcd_height;
    spilcd_info.var.xoffset = 0;
    spilcd_info.var.yoffset = 0;
    spilcd_info.fix.line_length = spilcd_wight * LV_COLOR_DEPTH / 8;

    return 0;
}

static int lv_spilcd_exit(void)
{
    sunxifb_mem_free((void **)&fbsmem_start, "spilcd_buff");
    fbsmem_start = NULL;
}

static int lv_spilcd_flush(void)
{
    sunxifb_mem_flush_cache(fbsmem_start + smem_len, smem_len);
    // If it is not dbi, please enable config DRIVERS_SPI_DMA_MALLOC_SELF
    bsp_disp_lcd_set_layer(LV_SPILCD_SCREEN_INFEX, &spilcd_info);
    // spilcd has DRAM, and the completion of sending means that the buff is free
    // bsp_disp_lcd_wait_for_vsync(LV_SPILCD_SCREEN_INFEX);
    return 0;
}

static int lv_spilcd_get_sizes(uint32_t *width, uint32_t *height)
{
    *width = bsp_disp_get_screen_width(LV_SPILCD_SCREEN_INFEX);
    *height = bsp_disp_get_screen_height(LV_SPILCD_SCREEN_INFEX);
    return 0;
}

static char *lv_spilcd_get_buff(void)
{
    return fbsmem_start;
}

// I know it's stupid, but it makes ococci happy
void bsp_flush_screen(char* ptr)
{
    struct fb_info lcd_info;
    lcd_info.var.xres = 240;
    lcd_info.var.yres = 320;
    lcd_info.var.xoffset = 0;
    lcd_info.var.yoffset = 0;
    lcd_info.screen_base = ptr;
    bsp_disp_lcd_set_layer(LV_SPILCD_SCREEN_INFEX, &lcd_info);
}
