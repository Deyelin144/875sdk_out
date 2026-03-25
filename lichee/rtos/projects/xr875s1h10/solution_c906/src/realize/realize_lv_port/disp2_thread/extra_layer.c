/**
 * @file lv_layer_layer.c
 *
 */

// #if USE_VIDEO_LAYER
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
#include "../g2d_driver.h"
#include <video/sunxi_display2.h>
#include <hal_mutex.h>
#include <hal_mem.h>

#include <hal_lcd_fb.h>

#ifdef USE_DISP2
#define LV_DISP2_SCREEN_INDEX 0
#define LV_DISP2_CHN          1
#define LV_DISP2_LAYER        0
#elif defined USE_SPILCD
#define LV_SPILCD_SCREEN_INFEX 0
#endif

#if (LV_COLOR_DEPTH == 32)
#define LV_DISP_FORMAT DISP_FORMAT_ARGB_8888
#elif (LV_COLOR_DEPTH == 16)
#define LV_DISP_FORMAT DISP_FORMAT_RGB_565
#else
#error PLEASE CHECK "LV_COLOR_DEPTH"
#endif

static uint32_t disp_width;
static uint32_t disp_height;
static uint32_t smem_len = 0;
static char *fbsmem_start = 0;
static uint8_t s_work_ready = 0;
static hal_mutex_t s_layer_mutex = NULL;
static bool s_extra_layer_init = false;

static void buf_copy(char *dst, char *src)
{
    if (src == NULL || dst == NULL) {
        return;
    }

#ifdef USE_SUNXIFB_G2D
    hal_dcache_clean((uintptr_t)dst, smem_len);
    hal_dcache_clean((uintptr_t)src, smem_len);
    sunxifb_g2d_blit_to_fb((uintptr_t)src, disp_width, disp_height, 0, 0, disp_width, disp_height,
                            (uintptr_t)dst, disp_width, disp_height, 0, 0, disp_width, disp_height, G2D_BLT_NONE_H);
    hal_dcache_invalidate((uintptr_t)dst, smem_len);
    hal_dcache_invalidate((uintptr_t)src, smem_len);
#else
    memcpy(dst, src, smem_len);
#endif
}

int extra_layer_init(int len, int w, int h)
{
    int ret = -1;
#if USE_SUNXIFB_G2D
    if (s_extra_layer_init) {
        ret = 0;
        goto exit;
    }

    s_layer_mutex = hal_mutex_create();
    if (s_layer_mutex == NULL) {
        goto exit;
    }

    disp_width = w;
    disp_height = h;
    smem_len = len;

    fbsmem_start = sunxifb_mem_alloc((smem_len * 3), "layer");
    if (fbsmem_start == NULL) {
        goto exit;
    }
    memset(fbsmem_start, 0, smem_len * 3);

    sunxi_g2d_open();

    s_extra_layer_init = true;
    ret = 0;
#endif /* USE_SUNXIFB_G2D */

exit:
    return ret;
}

void extra_layer_exit(void)
{
    if (!s_extra_layer_init) {
        return;
    }
    hal_mutex_lock(s_layer_mutex);
    s_extra_layer_init = false;
    s_work_ready = 0;
    sunxifb_mem_free((void **)&fbsmem_start, "layer");
    hal_mutex_unlock(s_layer_mutex);
    hal_mutex_delete(s_layer_mutex);
    sunxi_g2d_close();
}

// 0：额外图层 1：UIRGB565转ARGB8888的数据，占smem_len * 2
static char *extra_layer_get_buff(int buf_index)
{
    return fbsmem_start + smem_len * buf_index;
}

static uint8_t calc_luma(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);
}

static void rgb565_to_rgb888(uint16_t rgb565, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t r5 = (rgb565 >> 11) & 0x1F;  // 5位红色
    uint8_t g6 = (rgb565 >> 5)  & 0x3F;  // 6位绿色
    uint8_t b5 = rgb565         & 0x1F;  // 5位蓝色

    *r = (r5 << 3) | (r5 >> 2);  // 5 -> 8 bits
    *g = (g6 << 2) | (g6 >> 4);  // 6 -> 8 bits
    *b = (b5 << 3) | (b5 >> 2);  // 5 -> 8 bits
}

static int extra_layer_convert(void *src, int src_w, int src_h, void * dst, int dst_w, int dst_h)
{
    int ret = -1;
    int idx = 0;
    int num = 0;
    bool no_trans = false;
    uint8_t opa = 0;
    uint8_t r_bg = 0;
    uint8_t g_bg = 0;
    uint8_t b_bg = 0;
    uint8_t *pdst = NULL;
    uint8_t *next = NULL;
    uint16_t pixel = 0;

    if (src == NULL || dst == NULL || src_w == 0 || src_h == 0) {
        printf("g2d convert param err\n");
        goto exit;
    }

    pdst = (uint8_t *)dst;
    next = (uint8_t *)src;
    for (int y =0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            idx = (y * dst_w + x) * 2;
            pixel = next[idx] | (next[idx + 1] << 8);
            if (pixel == 0) {
                num++;
            }
        }
    }

    if (num <= dst_h * dst_w / 2) {
        no_trans = true;
    }

    hal_dcache_clean((unsigned long)src, src_w * src_h * 2);
	hal_dcache_clean((unsigned long)dst, dst_w * dst_h * 4);
    ret = sunxifb_g2d_convert((uintptr_t)src, src_w, src_h, G2D_FORMAT_RGB565,
                        (uintptr_t)dst, dst_w, dst_h, G2D_FORMAT_ARGB8888);
    hal_dcache_invalidate((unsigned long)src, src_w * src_h * 2);
	hal_dcache_invalidate((unsigned long)dst, dst_w * dst_h * 4);
    if (ret < 0) {
        printf("g2d convert fail\n");
        goto exit;
    }

    pdst = (uint8_t *)dst;
    next = (uint8_t *)src;
    for (int y =0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            opa = 0;
            idx = (y * dst_w + x) * 2;
            if (no_trans) {
                pdst[2 * idx + 3] = 0xFF; // 设置透明度为255
                continue;
            }
            pixel = next[idx] | (next[idx + 1] << 8);
            rgb565_to_rgb888(pixel, &r_bg, &g_bg, &b_bg);
            opa = calc_luma(r_bg, g_bg, b_bg);
            opa = (opa == 0)? 0 : 180;
            pdst[2 * idx + 3] = opa;
        }
    }

exit:
    // printf("g2d convert ok\n");
    return ret;
}

static int blend_two_layers(void *ui, void *ext, void *dst, unsigned int width, unsigned int height)
{
	g2d_bld info;
	int ret;

	memset(&info, 0, sizeof(g2d_bld));

	info.src_image[0].laddr[0] = __va_to_pa((uint32_t)ext);
	info.src_image[1].laddr[0] = __va_to_pa((uint32_t)ui);
	info.dst_image.laddr[0] = __va_to_pa((uint32_t)dst);

	info.src_image[0].laddr[1] = 0;
	info.src_image[0].laddr[2] = 0;
	info.src_image[0].use_phy_addr = 1;

	info.src_image[1].laddr[1] = 0;
	info.src_image[1].laddr[2] = 0;
	info.src_image[1].use_phy_addr = 1;

	info.dst_image.laddr[1] = 0;
	info.dst_image.laddr[2] = 0;
	info.dst_image.use_phy_addr = 1;

	info.bld_cmd = G2D_BLD_SRCOVER;

    info.src_image[0].format = LV_DISP_FORMAT;
    info.src_image[0].mode = G2D_PIXEL_ALPHA;
    info.src_image[0].width = disp_width;
    info.src_image[0].height = disp_height;
    info.src_image[0].clip_rect.x = 0;
    info.src_image[0].clip_rect.y = 0;
    info.src_image[0].clip_rect.w = disp_width;
    info.src_image[0].clip_rect.h = disp_height;
    info.src_image[0].coor.x = 0;
    info.src_image[0].coor.y = 0;
    info.src_image[0].alpha = 0xff;

    info.src_image[1].format = DISP_FORMAT_ARGB_8888;
    info.src_image[1].mode = G2D_PIXEL_ALPHA;
    info.src_image[1].width = disp_width;
    info.src_image[1].height = disp_height;
    info.src_image[1].clip_rect.x = 0;
    info.src_image[1].clip_rect.y = 0;
    info.src_image[1].clip_rect.w = disp_width;
    info.src_image[1].clip_rect.h = disp_height;
    info.src_image[1].coor.x = 0;
    info.src_image[1].coor.y = 0;
    // info.src_image[1].alpha = 0xff;

	info.dst_image.format = LV_DISP_FORMAT;
	info.dst_image.mode = G2D_GLOBAL_ALPHA;
	info.dst_image.alpha = 0xff;
	info.dst_image.width = disp_width;
	info.dst_image.height = disp_height;
	info.dst_image.clip_rect.x = 0;
	info.dst_image.clip_rect.y = 0;
	info.dst_image.clip_rect.w = disp_width;
	info.dst_image.clip_rect.h = disp_height;

	hal_dcache_clean((unsigned long)ui, disp_width * height * 4);
	hal_dcache_clean((unsigned long)ext, disp_width * height * 2);
	hal_dcache_clean((unsigned long)dst, disp_width * height * 2);

	ret = sunxi_g2d_control(G2D_CMD_BLD_H, &info);
	if (ret) {
		printf("g2d G2D_CMD_BLD_H fail\n");
	}

    hal_dcache_invalidate((unsigned long)ui, disp_width * height * 4);
	hal_dcache_invalidate((unsigned long)ext, disp_width * height * 2);
	hal_dcache_invalidate((unsigned long)dst, disp_width * height * 2);
	// printf("G2D_CMD_BLD_H ok\n");
	return ret;
}

static int extra_layer_mix(void *buf, void *data, int is_extra)
{
    int ret = -1;
    char *buf_ext = NULL;
    char *buf_argb = NULL;

    buf_ext = extra_layer_get_buff(0); // 保存的额外图层数据
    buf_argb = extra_layer_get_buff(1); // 保存的UI图层转ARGB888后的数据

    if (s_work_ready) {
        if (is_extra == 0) {
            // 传入的UI图层数据，则更新RGB565转ARGB888
            if (0 != extra_layer_convert(data, disp_width, disp_height, buf_argb, disp_width, disp_height)) {
                goto exit;
            }
        }
        // 混合图层并保存到buf送显
        if (0 != blend_two_layers(buf_argb, buf_ext, buf, disp_width, disp_height)) {
            goto exit;
        }
    }

    ret = 0;

exit:
    return ret;
}

/**
 * @description: 设置图层数据
 * @param final_buf 如果需要混合，则将混合数据保存其中
 * @param data 设置进来的图层数据，额外的需要保存，正常UI的则直接转ARGB888数据
 * @param width 图像数据宽度
 * @param height 图像数据高度
 * @param is_extra 是否是额外图层，由此决定对data的操作
 * @return 成功:0，否则不成功
 */
int extra_layer_set_data(void *final_buf, void *data, int width, int height, int is_extra)
{
    int ret = -1;
#if USE_SUNXIFB_G2D
    if (!s_extra_layer_init) {
        goto exit;
    }

    if (data == NULL || width != disp_width || height != disp_height) {
        printf("layer set data error, data = %p, width = %d, height = %d\n", data, width, height);
        goto exit;
    }

    hal_mutex_lock(s_layer_mutex);
    // 额外图层的数据需要保存，保证下次刷UI时有数据可以混合
    if (is_extra) {
        buf_copy(extra_layer_get_buff(0), data);
    }

    // 额外图层设置了数据，则可以混合
    if (is_extra && 0 == s_work_ready) {
        s_work_ready = 1;
    }
    ret = extra_layer_mix(final_buf, data, is_extra);
    hal_mutex_unlock(s_layer_mutex);
#endif /* USE_SUNXIFB_G2D */

exit:
    return ret;
}

// #endif /* USE_LV_VIDEO_LAYER */