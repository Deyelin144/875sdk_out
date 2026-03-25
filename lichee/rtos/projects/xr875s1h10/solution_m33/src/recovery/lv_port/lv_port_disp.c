/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "kernel/os/os.h"
#include "lv_port_disp.h"

#ifdef USE_DISP2
// #include "disp2.h"
// #include "disp_cfg_layer.h"
#elif defined USE_SPILCD
#include "spilcd.h"
#endif
#include "sunxifb.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
struct sunxifb_var_screeninfo {
    uint32_t xres;
    uint32_t yres;
    uint32_t xoffset;
    uint32_t yoffset;
    uint32_t bits_per_pixel;
};

struct sunxifb_fix_screeninfo {
    char *smem_start;
    long int smem_len;
    uint32_t line_length;
};

struct sunxifb_info {
    char *screenfbp[2];
    uint32_t fbnum;
    uint32_t fbindex;
    volatile bool dbuf_en;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(disp_t *disp);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static void disp_rounder(struct _lv_disp_drv_t * disp_drv, lv_area_t * area);
void disp_render_start(struct _lv_disp_drv_t * disp_drv);
void disp_rotate_r_angle(struct _lv_disp_drv_t * disp_drv, lv_coord_t * max_row);
void disp_monitor(struct _lv_disp_drv_t * disp_drv, uint32_t time, uint32_t px);
static void disp_wait(struct _lv_disp_drv_t * disp_drv);

//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);
//#define DISP_BUD_SIZE (MY_DISP_VER_RES * MY_DISP_HOR_RES * sizeof(lv_color_t))

//__psram_text static lv_color_t disp_buf[MY_DISP_VER_RES][MY_DISP_HOR_RES];			/*A screen sized buffer*/
//__psram_text static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*A screen sized buffer*/
//__psram_text static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*An other screen sized buffer*/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_port_disp_init(upgrade_handle_t *handle)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init(&handle->disp);

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */
    static lv_color_t *buf = NULL;
    static uint32_t width;
    static uint32_t height;
    sunxifb_get_sizes(&width, &height);

#ifndef LV_USE_DIRECT_MODE
    buf = (lv_color_t *)sunxifb_alloc(width * height * sizeof(lv_color_t), "lv_disp");
#else
    buf = (lv_color_t *)sunxifb_get_buf();
#endif /* LV_USE_DIRECT_MODE */

    if (buf == NULL) {
        sunxifb_exit();
        printf("malloc draw buffer fail.\n");
        return;
    }

    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, width * height);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

#ifdef USE_SUNXIFB_G2D
    disp_drv.hor_res      = height;
    disp_drv.ver_res      = width;
#else
    disp_drv.hor_res      = width;
    disp_drv.ver_res      = height;
#endif
    printf("height = %d width = %d\n", height, width);
    disp_drv.flush_cb     = sunxifb_flush;
    disp_drv.sw_rotate    = 1;
#ifndef USE_SUNXIFB_G2D
    disp_drv.rotated = LV_DISP_ROT_270;
#endif
    disp_drv.antialiasing = 1;

    disp_drv.wait_cb      = disp_wait;
    disp_drv.rounder_cb   = disp_rounder;

#ifdef LV_USE_DIRECT_MODE
    disp_drv.direct_mode = 1;
#endif /* LV_USE_DIRECT_MODE */

    disp_drv.draw_buf     = &disp_buf;
    lv_disp_drv_register(&disp_drv);

}

/**********************
 *   STATIC FUNCTIONS
//  **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(disp_t *disp)
{
    sunxifb_init(LV_DISP_ROT_NONE);
    return;
}


/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    // TODO
}

static void disp_wait(struct _lv_disp_drv_t * disp_drv)
{
	os_adapter()->msleep(1);
	// printf("==================================================================================================================.\n");
}

static void disp_rounder(struct _lv_disp_drv_t * disp_drv, lv_area_t * area)
{
    // printf("%s %d\n", __func__, __LINE__);
    if (area->x1 % 4 != 0) {
		area->x1 -= (area->x1 % 4);
	}

	if (area->y1 % 4 != 0) {
		area->y1 -= (area->y1 % 4);
	}

	if (area->x2 % 4 == 0) {
		area->x2 += (area->x2 % 4);
	}

	if (area->y2 % 4 == 0) {
		area->y2 += (area->y2 % 4);
	}
}

void disp_render_start(struct _lv_disp_drv_t * disp_drv)
{
	//drv_led_get_te_sem();
}

void disp_rotate_r_angle(struct _lv_disp_drv_t * disp_drv, lv_coord_t * max_row)
{
    if (*max_row % 4 != 0) {
		*max_row -= (*max_row % 4);
	}
}

void disp_monitor(struct _lv_disp_drv_t * disp_drv, uint32_t time, uint32_t px)
{
//	printf("========>time = %u, px = %u.\n", time, px);
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
