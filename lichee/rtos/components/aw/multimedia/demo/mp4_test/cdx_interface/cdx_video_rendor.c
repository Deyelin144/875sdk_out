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

#include <string.h>
#include <stdlib.h>
#include "hal_thread.h"
#include "cdx_video_dec.h"
#include "sunxi_hal_common.h"
#include "hal_lcd_fb.h"
#include "hal_cache.h"
#include "g2d_driver.h"
#include "sunxi_display2.h"
#include "video_debug/vd_log.h"

// 要使用原测试用例的播放，则将此处打开，sunxifb_extra_layer_set_buf是G2D混合播放的接口
#define USE_QUANZHI_TEST_DEMO 0

#ifdef CONFIG_DRIVERS_SPILCD
#define FB_NUM 1
static struct fb_info g_fb_info = { 0 };
#else
#define FB_NUM 2
#endif

#define DISPLAY_PRIV   0
#define DISPLAY_FORMAT LCDFB_FORMAT_RGB_565 //LCDFB_FORMAT_ARGB_8888
#define DISPLAY_BPP    2                    //4

#define UINT32_UP_ALIGN8(x) (((x & 0x07) == 0) ? x : (x + (8 - (x & 0x07))))

typedef struct {
    unsigned width;
    unsigned height;
} image_t;

typedef struct {
    unsigned step;
    unsigned char *buff_temp;
    g2d_blt_h *blit_para;
} g2d_convert;

typedef struct {
    g2d_convert g_g2d_convert;
    image_t yuv_input;
    image_t ratio;
    image_t rotate_ratio;
    image_t dis_output;
    image_t win_size;
    image_t win_position;
    unsigned char fb_p;
    unsigned char direction;
    unsigned char auto_size;
    unsigned char set_win;
    unsigned char *rotate_buff;
    unsigned char *display_point[FB_NUM];
} video_rendor_context;
static video_rendor_context *video_rendor = NULL;

extern int sunxi_g2d_control(int cmd, void *arg);
extern int sunxi_g2d_close(void);
extern int sunxi_g2d_open(void);
extern int disp_ioctl(int cmd, void *arg);

#ifdef CONFIG_DRIVERS_SPILCD
static void spi_fb_init(void *buff_addr, unsigned width, unsigned height)
{
    g_fb_info.screen_base = buff_addr;
    g_fb_info.var.xres = width;
    g_fb_info.var.yres = height;
    g_fb_info.var.xoffset = 0;
    g_fb_info.var.yoffset = 0;
    g_fb_info.var.lcd_pixel_fmt = DISPLAY_FORMAT;
    g_fb_info.fix.line_length = (width * DISPLAY_BPP);
}

static void swap_endianess_16(uint16_t *data, unsigned length)
{
    for (unsigned i = 0; i < length; i++) {
        data[i] = __builtin_bswap16(data[i]);
    }
}

static void Mp4SpiLcdGetSizes(uint32_t *width, uint32_t *height)
{
    *width = bsp_disp_get_screen_width(DISPLAY_PRIV);
    *height = bsp_disp_get_screen_height(DISPLAY_PRIV);
}

static void video_display_clear(void)
{
    if (g_fb_info.screen_base == NULL)
        return;

    memset(g_fb_info.screen_base, 0, g_fb_info.fix.line_length * g_fb_info.var.yres);
    bsp_disp_lcd_set_layer(0, &g_fb_info);
}
#else
static void bsp_rgb_display(void *buff_addr, unsigned width, unsigned height,
                            __disp_pixel_fmt_t format)
{
    struct disp_layer_config config;
    unsigned long arg[3];

    memset(&config, 0, sizeof(struct disp_layer_config));
    config.channel = 0;
    config.layer_id = 0;
    config.enable = 1;
    config.info.mode = LAYER_MODE_BUFFER;
    config.info.fb.addr[0] = (unsigned long long)buff_addr;
    config.info.fb.size[0].width = width;
    config.info.fb.size[0].height = height;
    config.info.fb.format = format;
    config.info.screen_win.width = width;
    config.info.screen_win.height = height;
    config.info.screen_win.x = 0;
    config.info.screen_win.y = 0;

    config.info.fb.crop.x = 0;
    config.info.fb.crop.y = 0;
    config.info.fb.crop.width = ((long long)width) << 32;
    config.info.fb.crop.height = ((long long)height) << 32;
    config.info.fb.flags = DISP_BF_NORMAL;
    config.info.fb.scan = DISP_SCAN_PROGRESSIVE;
    config.info.alpha_mode = 0;
    config.info.alpha_value = 0xff;
    config.info.screen_win.width = width;
    config.info.screen_win.height = height;
    config.info.id = 0;

    arg[0] = 0;
    arg[1] = (unsigned long)&config;
    arg[2] = 1;
    disp_ioctl(DISP_LAYER_SET_CONFIG, (void *)arg);
    disp_ioctl(DISP_WAIT_VSYNC, (void *)arg);
}

void Mp4SpiLcdGetSizes(uint32_t *width, uint32_t *height)
{
    unsigned long arg[6];

    // disp_open();
    arg[0] = 0;
    *width = disp_ioctl(DISP_GET_SCN_WIDTH, (void *)arg);
    *height = disp_ioctl(DISP_GET_SCN_HEIGHT, (void *)arg);
    // disp_release();
}

static void video_display_clear(void)
{
    struct disp_layer_config config;
    unsigned long arg[3];

    memset(&config, 0, sizeof(struct disp_layer_config));
    config.enable = false;
    config.channel = 0;
    config.layer_id = 0;

    arg[0] = 0;
    arg[1] = (unsigned long)&config;
    arg[2] = 1;
    disp_ioctl(DISP_LAYER_SET_CONFIG, (void *)arg);
    disp_ioctl(DISP_WAIT_VSYNC, (void *)arg);
}
#endif

static int g2d_yuv_format_convert(unsigned char *src_buf, unsigned char *dst_buf)
{
    if (video_rendor == NULL) {
        return -1;
    }

    int ret = -1;
    unsigned char *g2d_inbuff = NULL;
    unsigned char *g2d_outbuff = NULL;
    unsigned roi_x = 0;
    unsigned roi_y = 0;
    unsigned ratio_width = 0;
    unsigned ratio_height = 0;
    unsigned cache_size64 = 0;
    image_t g2d_src = { 0 };
    image_t g2d_dst = { 0 };
    g2d_blt_h blit_para = { 0 };

    g2d_inbuff = src_buf;
    g2d_outbuff = dst_buf;
    g2d_src.width = video_rendor->yuv_input.width;
    g2d_src.height = video_rendor->yuv_input.height;
    g2d_dst.width = video_rendor->dis_output.width;
    g2d_dst.height = video_rendor->dis_output.height;
    // adaptive screen?
    if (video_rendor->auto_size == 1) {
        ratio_width = video_rendor->ratio.width;
        ratio_height = video_rendor->ratio.height;
    } else {
        ratio_width = g2d_src.width;
        ratio_height = g2d_src.height;
    }

    // blit_para.dst_image_h.color = 0xee8899;
    // blit_para.dst_image_h.mode = G2D_PIXEL_ALPHA;
    // blit_para.dst_image_h.alpha = 255;
    // blit_para.src_image_h.color = 0xee8899;
    // blit_para.src_image_h.mode = G2D_PIXEL_ALPHA;
    // blit_para.src_image_h.alpha = 255;

    sunxi_g2d_open();

    // rotate
    if (video_rendor->direction == 1) {
        g2d_inbuff = src_buf;
        g2d_outbuff = video_rendor->rotate_buff;
        g2d_src.width = video_rendor->yuv_input.width;
        g2d_src.height = video_rendor->yuv_input.height;
        g2d_dst.width = UINT32_UP_ALIGN8(g2d_src.height);
        g2d_dst.height = UINT32_UP_ALIGN8(g2d_src.width);

        blit_para.src_image_h.laddr[0] = ((uint32_t)(uintptr_t)g2d_inbuff);
        blit_para.src_image_h.laddr[1] =
                (int)(blit_para.src_image_h.laddr[0] + g2d_src.width * g2d_src.height);
        blit_para.src_image_h.laddr[2] =
                (int)(blit_para.src_image_h.laddr[0] + g2d_src.width * g2d_src.height * 5 / 4);
        blit_para.src_image_h.use_phy_addr = 1;

        blit_para.src_image_h.clip_rect.x = 0;
        blit_para.src_image_h.clip_rect.y = 0;
        blit_para.src_image_h.clip_rect.w = g2d_src.width;
        blit_para.src_image_h.clip_rect.h = g2d_src.height;

        blit_para.src_image_h.format = G2D_FORMAT_YUV420_PLANAR;
        blit_para.src_image_h.width = g2d_src.width;
        blit_para.src_image_h.height = g2d_src.height;

        blit_para.dst_image_h.laddr[0] = ((uint32_t)(uintptr_t)g2d_outbuff);
        blit_para.dst_image_h.laddr[1] =
                (int)(blit_para.dst_image_h.laddr[0] + g2d_dst.width * g2d_dst.height);
        blit_para.dst_image_h.laddr[2] =
                (int)(blit_para.dst_image_h.laddr[0] + g2d_dst.width * g2d_dst.height * 5 / 4);
        blit_para.dst_image_h.use_phy_addr = 1;

        blit_para.dst_image_h.clip_rect.x = 0;
        blit_para.dst_image_h.clip_rect.y = 0;
        blit_para.dst_image_h.clip_rect.w = g2d_dst.width;
        blit_para.dst_image_h.clip_rect.h = g2d_dst.height;

        blit_para.dst_image_h.format = G2D_FORMAT_YUV420_PLANAR;
        blit_para.dst_image_h.width = g2d_dst.width;
        blit_para.dst_image_h.height = g2d_dst.height;

        blit_para.flag_h = G2D_ROT_90;

        cache_size64 = g2d_src.width * g2d_src.height * 3 / 2;
        cache_size64 +=
                (cache_size64 % CACHELINE_LEN) ? (CACHELINE_LEN - cache_size64 % CACHELINE_LEN) : 0;
        hal_dcache_clean((unsigned long)g2d_inbuff, cache_size64);
        cache_size64 = g2d_dst.width * g2d_dst.height * 3 / 2;
        cache_size64 +=
                (cache_size64 % CACHELINE_LEN) ? (CACHELINE_LEN - cache_size64 % CACHELINE_LEN) : 0;
        hal_dcache_clean((unsigned long)g2d_outbuff, cache_size64);
        if (sunxi_g2d_control(G2D_CMD_BITBLT_H, &blit_para) < 0) {
            vlog_error("g2d G2D_CMD_BITBLT_H fail.");
            goto g2d_yuv_format_convert_exit;
        }
        hal_dcache_invalidate((unsigned long)g2d_outbuff, cache_size64);

        g2d_inbuff = video_rendor->rotate_buff;
        g2d_outbuff = dst_buf;
        g2d_src.width = g2d_dst.width;
        g2d_src.height = g2d_dst.height;
        g2d_dst.width = video_rendor->dis_output.width;
        g2d_dst.height = video_rendor->dis_output.height;
        if (video_rendor->auto_size == 1) {
            ratio_width = video_rendor->rotate_ratio.width;
            ratio_height = video_rendor->rotate_ratio.height;
        } else {
            ratio_width = g2d_src.width;
            ratio_height = g2d_src.height;
        }

        if (video_rendor->set_win) {
            ratio_width = video_rendor->win_size.height;
            ratio_height = video_rendor->win_size.width;
        }
    } else {
        if (video_rendor->set_win) {
            ratio_width = video_rendor->win_size.width;
            ratio_height = video_rendor->win_size.height;
        }
    }

    if (video_rendor->auto_size == 0) {
        if ((ratio_width > g2d_dst.width) || (ratio_height > g2d_dst.height)) {
            vlog_error("target size is out of range of screen, please enable auto size");
            vlog_error("screen width(%d) x higth(%d)", g2d_dst.width, g2d_dst.height);
            vlog_error("video width(%d) x height(%d)", ratio_width, ratio_height);
            goto g2d_yuv_format_convert_exit;
        }
    }


    if (video_rendor->set_win) {
        unsigned x = video_rendor->dis_output.width - ratio_width;
        unsigned y = video_rendor->dis_output.height - ratio_height;
        if (video_rendor->direction) {
            roi_x = x - video_rendor->win_position.height > 0? x - video_rendor->win_position.height : 0;
            roi_y = video_rendor->win_position.width > y? y : video_rendor->win_position.width;
        } else {
            roi_x = video_rendor->win_position.width > x? x : video_rendor->win_position.width;
            roi_y = video_rendor->win_position.height > y? y : video_rendor->win_position.height;
        }
    } else {
        roi_x = ((g2d_dst.width - ratio_width) >> 1);
        roi_y = ((g2d_dst.height - ratio_height) >> 1);
    }
    // printf("win size:%d %d %d %d\n", ratio_width, ratio_height, roi_x, roi_y);

    memset(&blit_para, 0, sizeof(g2d_blt_h));
    blit_para.src_image_h.laddr[0] = ((uint32_t)(uintptr_t)g2d_inbuff);
    blit_para.src_image_h.laddr[1] =
            (int)(blit_para.src_image_h.laddr[0] + g2d_src.width * g2d_src.height);
    blit_para.src_image_h.laddr[2] =
            (int)(blit_para.src_image_h.laddr[0] + g2d_src.width * g2d_src.height * 5 / 4);
    blit_para.src_image_h.use_phy_addr = 1;

    blit_para.src_image_h.clip_rect.x = 0;
    blit_para.src_image_h.clip_rect.y = 0;
    if (video_rendor->direction == 1) {
        blit_para.src_image_h.clip_rect.w = video_rendor->yuv_input.height;
        blit_para.src_image_h.clip_rect.h = video_rendor->yuv_input.width;
    } else {
        blit_para.src_image_h.clip_rect.w = video_rendor->yuv_input.width;
        blit_para.src_image_h.clip_rect.h = video_rendor->yuv_input.height;
    }

    blit_para.src_image_h.format = G2D_FORMAT_YUV420_PLANAR;
    blit_para.src_image_h.width = g2d_src.width;
    blit_para.src_image_h.height = g2d_src.height;

    blit_para.dst_image_h.laddr[0] = ((uint32_t)(uintptr_t)g2d_outbuff);
    // blit_para.dst_image_h.laddr[1] = (int)(blit_para.dst_image_h.laddr[0] + g2d_dst.width * g2d_dst.height);
    // blit_para.dst_image_h.laddr[2] = (int)(blit_para.dst_image_h.laddr[0] + g2d_dst.width * g2d_dst.height * 5 / 4);
    blit_para.dst_image_h.use_phy_addr = 1;

    blit_para.dst_image_h.clip_rect.x = roi_x;
    blit_para.dst_image_h.clip_rect.y = roi_y;
    blit_para.dst_image_h.clip_rect.w = ratio_width;
    blit_para.dst_image_h.clip_rect.h = ratio_height;

    blit_para.dst_image_h.format = G2D_FORMAT_RGB565;
    blit_para.dst_image_h.width = g2d_dst.width;
    blit_para.dst_image_h.height = g2d_dst.height;

    blit_para.flag_h = G2D_BLT_NONE;

    memset(g2d_outbuff, 0, g2d_dst.width * g2d_dst.height * 2);
    hal_dcache_clean((unsigned long)g2d_outbuff, g2d_dst.width * g2d_dst.height * 2);
    cache_size64 = g2d_src.width * g2d_src.height * 3 / 2;
    cache_size64 +=
            (cache_size64 % CACHELINE_LEN) ? (CACHELINE_LEN - cache_size64 % CACHELINE_LEN) : 0;
    hal_dcache_clean((unsigned long)g2d_inbuff, cache_size64);
    if (sunxi_g2d_control(G2D_CMD_BITBLT_H, &blit_para) < 0) {
        vlog_error("g2d G2D_CMD_BITBLT_H fail.");
        goto g2d_yuv_format_convert_exit;
    }
    hal_dcache_invalidate((unsigned long)g2d_outbuff, g2d_dst.width * g2d_dst.height * 2);

    ret = 0;

g2d_yuv_format_convert_exit:
    sunxi_g2d_close();
    return ret;
}
extern int sunxifb_extra_layer_set_buf(char *data, int width, int height);

int Mp4VideoPlay(unsigned char *dec_data)
{
    if (g2d_yuv_format_convert(dec_data, video_rendor->display_point[video_rendor->fb_p]) < 0) {
        return -1;
    }

    // char name[20];
    // sprintf(name, "sdmmc/test/%d.bin", testnum);
    // testnum ++;
    // V_write_buffer2file(name, video_rendor->display_point[fb_p], 307200);
    // printf("finish %d\n", testnum);

#if USE_QUANZHI_TEST_DEMO
#ifdef CONFIG_DRIVERS_SPILCD
#ifdef CONFIG_SPILCD_SUPPORT_QSPI
    swap_endianess_16((uint16_t *)video_rendor->display_point[fb_p],
                      dis_output.width * dis_output.height);
#endif
    bsp_disp_lcd_set_layer(0, &g_fb_info);
#else
    bsp_rgb_display(video_rendor->display_point[video_rendor->fb_p], video_rendor->dis_output.width,
                    video_rendor->dis_output.height, DISPLAY_FORMAT);
#endif
#else
    sunxifb_extra_layer_set_buf((char *)video_rendor->display_point[video_rendor->fb_p], video_rendor->dis_output.width, video_rendor->dis_output.height);
#endif
    if (video_rendor->fb_p < (FB_NUM - 1)) {
        video_rendor->fb_p += 1;
    } else {
        video_rendor->fb_p = 0;
    }
    return 0;
}

/**
 * @video_size : dimensions of the image
 * @lcd_size ： Screen size or target output size
 * @ratio_size ： thr size after adjust
*/
static int Mp4VideoAdjustSize(image_t *video_size, image_t *lcd_size, image_t *ratio_size)
{
    unsigned ratio_width_factor = video_size->height * lcd_size->width;
    unsigned ratio_height_factor = video_size->width * lcd_size->height;

    if ((video_size->width < lcd_size->width) && (video_size->height < lcd_size->height)) {
        if (ratio_width_factor > ratio_height_factor) {
            ratio_size->width = video_size->width * lcd_size->height / video_size->height;
            ratio_size->height = video_size->height * lcd_size->height / video_size->height;
        } else {
            ratio_size->width = video_size->width * lcd_size->width / video_size->width;
            // ratio_size->height = lcd_size->height * (lcd_size->width / video_size->width);
            ratio_size->height = video_size->height * lcd_size->width / video_size->width;
        }
    } else if ((video_size->width > lcd_size->width) || (video_size->height > lcd_size->height)) {
        if (ratio_width_factor < ratio_height_factor) {
            // ratio_size->width = video_size->height * lcd_size->width / video_size->width;
            ratio_size->width = lcd_size->width;
            ratio_size->height = video_size->height * lcd_size->width / video_size->width;
        } else {
            ratio_size->width = video_size->width * lcd_size->height / video_size->height;
            // ratio_size->height = video_size->height * lcd_size->height / video_size->height;
            ratio_size->height = lcd_size->height;
        }
    } else {
        ratio_size->width = video_size->width;
        ratio_size->height = video_size->height;
    }
    return 0;
}

int VideoPlayInit(unsigned width, unsigned higth)
{
    unsigned display_size = 0;
    image_t rotate_trmp = { 0 };

    video_rendor = malloc(sizeof(video_rendor_context));
    if (video_rendor == NULL) {
        return -1;
    }
    memset(video_rendor, 0, sizeof(video_rendor_context));

    Mp4SpiLcdGetSizes(&video_rendor->dis_output.width, &video_rendor->dis_output.height);
    display_size = video_rendor->dis_output.width * video_rendor->dis_output.height * DISPLAY_BPP;

#ifdef CONFIG_DRIVERS_SPILCD
#ifdef CONFIG_SPILCD_SUPPORT_QSPI
    video_rendor->display_point[0] = bsp_disp_qspi_malloc(DISPLAY_PRIV, 0);
#else
    video_rendor->display_point[0] = hal_malloc_coherent(display_size);
#endif // CONFIG_SPILCD_SUPPORT_QSPI
#else
    video_rendor->display_point[0] = hal_malloc_coherent(display_size * FB_NUM);
#endif

    if (video_rendor->display_point[0] == NULL) {
        return -1;
    }
    memset(video_rendor->display_point[0], 0, (display_size * FB_NUM));

#ifdef CONFIG_DRIVERS_SPILCD
    spi_fb_init(video_rendor->display_point[0], video_rendor->dis_output.width,
                video_rendor->dis_output.height);
#else
    for (int i = 1; i < FB_NUM; i++) {
        video_rendor->display_point[i] = (video_rendor->display_point[0] + display_size * i);
    }
#endif

    video_rendor->yuv_input.width = width;
    video_rendor->yuv_input.height = higth;

    rotate_trmp.width = higth;
    rotate_trmp.height = width;
    Mp4VideoAdjustSize(&video_rendor->yuv_input, &video_rendor->dis_output, &video_rendor->ratio);
    Mp4VideoAdjustSize(&rotate_trmp, &video_rendor->dis_output, &video_rendor->rotate_ratio);

    vlog_debug("video size : %d * %d\n", video_rendor->yuv_input.width,
               video_rendor->yuv_input.height);
    vlog_debug("lcd size : %d * %d\n", video_rendor->dis_output.width,
               video_rendor->dis_output.height);
    vlog_debug("rotate : %d * %d\n", video_rendor->ratio.width, video_rendor->ratio.height);
    vlog_debug("rotate_ratio : %d * %d\n", video_rendor->rotate_ratio.width,
               video_rendor->rotate_ratio.height);

    return 0;
}

int Mp4VideoRotate(unsigned char rotate)
{
    if (video_rendor == NULL) {
        return -1;
    }

    unsigned max_size =
            video_rendor->dis_output.width * video_rendor->dis_output.height * DISPLAY_BPP;

    video_rendor->direction = rotate;
    if (video_rendor->direction == 1) {
        if (video_rendor->rotate_buff == NULL) {
            video_rendor->rotate_buff = hal_malloc_coherent(max_size);
            if (video_rendor->rotate_buff == NULL) {
                vlog_error("is no enough buff to rotate!\n");
                video_rendor->direction = 0;
                return -1;
            }
        }
    } else {
        if (video_rendor->rotate_buff != NULL) {
            hal_free_coherent(video_rendor->rotate_buff);
            video_rendor->rotate_buff = NULL;
        }
    }
    return 0;
}

int Mp4VideoAutoSize(unsigned char autos)
{
    if (video_rendor == NULL) {
        return -1;
    }

    video_rendor->auto_size = autos;
    return 0;
}

int Mp4VideoSetWin(unsigned char set, int x, int y, int w, int h)
{
    if (video_rendor == NULL) {
        return -1;
    }

    video_rendor->set_win = set;
    if (set) {
        video_rendor->win_size.width = w;
        video_rendor->win_size.height = h;
        video_rendor->win_position.width = x;
        video_rendor->win_position.height = y;
    }

    return 0;
}

int VideoPlayDeinit(void)
{
    if (video_rendor == NULL) {
        return -1;
    }

    if (video_rendor->rotate_buff != NULL) {
        hal_free_coherent(video_rendor->rotate_buff);
        video_rendor->rotate_buff = NULL;
    }

#if USE_QUANZHI_TEST_DEMO
    video_display_clear();
#endif
#ifdef CONFIG_SPILCD_SUPPORT_QSPI
    bsp_disp_qspi_free(DISPLAY_PRIV);
#else
    hal_free_coherent(video_rendor->display_point[0]);
#endif
    free(video_rendor);
    video_rendor = NULL;
    return 0;
}
