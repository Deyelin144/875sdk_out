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
#include <video/sunxi_display2.h>
#include "../sunximem.h"
#include "../sunxig2d.h"
/*********************
 *      DEFINES
 *********************/
#define LV_DISP2_SCREEN_INDEX 0
#define LV_DISP2_CHN          1
#define LV_DISP2_LAYER        0

#if (LV_COLOR_DEPTH == 32)
#define LV_DISP_FORMAT DISP_FORMAT_ARGB_8888
#elif (LV_COLOR_DEPTH == 16)
#define LV_DISP_FORMAT DISP_FORMAT_RGB_565
#else
#error PLEASE CHECK "LV_COLOR_DEPTH"
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      STRUCTURES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static uint32_t disp_width;
static uint32_t disp_height;
static uint32_t smem_len = 0;
static char *fbsmem_start = 0;
static uint8_t fbsmem_offset = 0;

extern int disp_ioctl(int cmd, void *arg);
extern int disp_release(void);
extern int disp_open(void);

static int lv_disp2_init(void);
static int lv_disp2_exit(void);
static int lv_disp2_flush(void);
static int lv_disp2_get_sizes(uint32_t *width, uint32_t *height);
static char *lv_disp_get_buff(void);
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_disp_ops lv_disp2_ops = {
    .init = lv_disp2_init,
    .exit = lv_disp2_exit,
    .flush = lv_disp2_flush,
    .get_sizes = lv_disp2_get_sizes,
    .get_buf = lv_disp_get_buff,
};

/**********************
 *   STATIC FUNCTIONS
 **********************/
static int lv_disp2_init(void)
{
    unsigned long arg[6];

    arg[0] = LV_DISP2_SCREEN_INDEX;
    disp_width = disp_ioctl(DISP_GET_SCN_WIDTH, (void *)arg);
    disp_height = disp_ioctl(DISP_GET_SCN_HEIGHT, (void *)arg);

    smem_len = disp_width * disp_height * LV_COLOR_DEPTH / 8;
    // two buff
    fbsmem_start = sunxifb_mem_alloc((smem_len * 2), "disp2");
    if (fbsmem_start == NULL) {
        return -1;
    }
    memset(fbsmem_start, 0, smem_len);

    return 0;
}

static int lv_disp2_exit(void)
{
    sunxifb_mem_free((void **)&fbsmem_start, "disp2");
}

// use after DISP_WAIT_VSYNC
static void lv_disp2_memory_update(void)
{
    char *disp_idle_buff = NULL;
    char *disp_work_buff = NULL;

    //after DISP_WAIT_VSYNC, working buffer change
    if (fbsmem_offset) {
        disp_idle_buff = fbsmem_start;
        disp_work_buff = fbsmem_start + smem_len;
    } else {
        disp_idle_buff = fbsmem_start + smem_len;
        disp_work_buff = fbsmem_start;
    }
#ifdef USE_SUNXIFB_G2D
    sunxifb_g2d_blit_to_fb((uintptr_t)disp_work_buff, disp_width, disp_height, 0, 0, disp_width,
                           disp_height, (uintptr_t)disp_idle_buff, disp_width, disp_height, 0, 0,
                           disp_width, disp_height, G2D_BLT_NONE_H);
#else
    memcpy(disp_idle_buff, disp_work_buff, smem_len);
#endif /* USE_SUNXIFB_G2D */
    fbsmem_offset ^= 1;
}

static int lv_disp2_flush(void)
{
    struct disp_layer_config config;
    unsigned long arg[3];
    char *disp_idle_buff = NULL;

    // disp_idle_buff = fbsmem_start + smem_len * fbsmem_offset
    disp_idle_buff = ((fbsmem_offset) ? (fbsmem_start + smem_len) : fbsmem_start);

    memset(&config, 0, sizeof(struct disp_layer_config));
    config.channel = LV_DISP2_CHN;
    config.layer_id = LV_DISP2_LAYER;
    config.enable = 1;
    config.info.mode = LAYER_MODE_BUFFER;
    config.info.fb.addr[0] = (unsigned long long)disp_idle_buff;
    config.info.fb.size[0].width = disp_width;
    config.info.fb.size[0].height = disp_height;
    config.info.fb.format = LV_DISP_FORMAT;
    config.info.fb.crop.x = 0;
    config.info.fb.crop.y = 0;
    config.info.fb.crop.width = ((long long)disp_width) << 32;
    config.info.fb.crop.height = ((long long)disp_height) << 32;
    config.info.fb.flags = DISP_BF_NORMAL;
    config.info.fb.scan = DISP_SCAN_PROGRESSIVE;
    config.info.alpha_mode = 0;
    config.info.alpha_value = 0xff;
    config.info.screen_win.width = disp_width;
    config.info.screen_win.height = disp_height;
    config.info.id = 0;

    sunxifb_mem_flush_cache(disp_idle_buff, smem_len);

    arg[0] = LV_DISP2_SCREEN_INDEX;
    arg[1] = (unsigned long)&config;
    arg[2] = 1;
    disp_ioctl(DISP_LAYER_SET_CONFIG, (void *)arg);

    arg[0] = LV_DISP2_SCREEN_INDEX;
    disp_ioctl(DISP_WAIT_VSYNC, (void *)arg);

    lv_disp2_memory_update();
    return 0;
}

static int lv_disp2_get_sizes(uint32_t *width, uint32_t *height)
{
    unsigned long arg[3];
    arg[0] = LV_DISP2_SCREEN_INDEX;
    *width = disp_ioctl(DISP_GET_SCN_WIDTH, (void *)arg);
    *height = disp_ioctl(DISP_GET_SCN_HEIGHT, (void *)arg);
    return 0;
}

static char *lv_disp_get_buff(void)
{
    return ((fbsmem_offset) ? (fbsmem_start + smem_len) : fbsmem_start);
}
