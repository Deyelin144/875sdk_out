/**
 * @file sunxifb.c
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
#if USE_SUNXIFB_G2D
#include "../sunxig2d.h"
#endif
// #define LV_USE_SUNXIFB_DEBUG
#include <hal_sem.h>
#include <hal_mutex.h>
#include <hal_queue.h>
#include <hal_thread.h>
#include <hal_time.h>

/*********************
 *      DEFINES
 *********************/
// #define LV_USE_SUNXIFB_DEBUG
#ifdef LV_USE_SUNXIFB_DEBUG
#include <sys/time.h>
#endif /* LV_USE_SUNXIFB_DEBUG */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      STRUCTURES
 **********************/
struct lv_queue_context {
    lv_disp_drv_t *drv;
    const lv_area_t *area;
    lv_color_t *color_p;
    bool last_area;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_disp_ops *lv_disp = NULL;
static hal_sem_t sunxifb_sem = NULL;
// static hal_sem_t sunxifb_ready_sem = NULL;
static hal_mutex_t sunxifb_mux = NULL;
static hal_thread_t sunxifb_flush_thread = NULL;
#ifdef LV_USE_SUNXIFB_DEBUG
static uint64_t cur_time = 0;// = hal_gettime_ns();
static uint64_t old_time = 0;
static uint64_t disp_time = 0;
static uint64_t lock_time = 0;
#endif /* LV_USE_SUNXIFB_DEBUG */

static int lv_disp_thread_create(void);
static void lv_disp_thread_destory(void);
static int sunxifb_area_update(char *dst_buff, struct lv_queue_context *new_area);
extern void hal_dcache_invalidate(unsigned long vaddr_start, unsigned long size);
extern void hal_dcache_clean(unsigned long vaddr_start, unsigned long size);
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void sunxifb_get_sizes(uint32_t *width, uint32_t *height)
{
    if (lv_disp != NULL)
        lv_disp->get_sizes(width, height);
}

void sunxifb_init(uint32_t rotated)
{
#ifdef USE_DISP2
    lv_disp = &lv_disp2_ops;
#elif defined USE_SPILCD
    lv_disp = &lv_spilcd_ops;
#endif

    if (lv_disp->init() < 0) {
        printf("lv_disp init fail!\n");
        return;
    }

    if (lv_disp_thread_create() < 0) {
        lv_disp->exit();
        printf("lv_disp_thread create fail!\n");
    }
#ifdef USE_SUNXIFB_G2D
    sunxifb_g2d_init(LV_COLOR_DEPTH);
#endif

    return;
}

void sunxifb_exit(void)
{
    lv_disp_thread_destory();
    lv_disp->exit();
    lv_disp = NULL;
#ifdef USE_SUNXIFB_G2D
    sunxifb_g2d_deinit();
#endif
}

/**
 * Flush a buffer to the marked area
 * @param drv pointer to driver where this function belongs
 * @param area an area where to copy `color_p`
 * @param color_p an array of pixel to copy to the `area` part of the screen
 */
void sunxifb_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    // printf("flush cb\n");
    struct lv_queue_context lv_update_data;
    char *refresh_buff = NULL;

    lv_update_data.drv = drv;
    lv_update_data.area = area;
    lv_update_data.color_p = color_p;
    lv_update_data.last_area = lv_disp_flush_is_last(drv);
#ifdef LV_USE_SUNXIFB_DEBUG
    cur_time = hal_gettime_ns();
#endif

    hal_mutex_lock(sunxifb_mux);
#ifdef LV_USE_SUNXIFB_DEBUG
   lock_time = hal_gettime_ns();
#endif
    refresh_buff = lv_disp->get_buf();
    if (!refresh_buff) {
        printf("buffer is null\n");
    }
    sunxifb_area_update(refresh_buff, &lv_update_data);
    hal_mutex_unlock(sunxifb_mux);

    if (lv_update_data.last_area != 0) {
        if (hal_sem_post(sunxifb_sem) < 0) {
            printf("sunxifb_flush send error\n");
            return;
        }
    }
#ifdef LV_USE_SUNXIFB_DEBUG
    disp_time = hal_gettime_ns();
    printf("lvgl use %lluus, lock use: %lluus, disp use %lluus\n",
        (cur_time - old_time)/1000, (lock_time - cur_time)/1000, (disp_time - cur_time)/1000);
    old_time = disp_time;
#endif
    lv_disp_flush_ready(drv);

    return;
}

void *sunxifb_alloc(size_t size, char *label)
{
    return sunxifb_mem_alloc(size, label);
}

void sunxifb_free(void **data, char *label)
{
    sunxifb_mem_free(data, label);
}

#ifdef LV_USE_DIRECT_MODE
#error can not support direct mode
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void lv_disp_thread(void *arg)
{
    while (lv_disp == NULL) {
        printf("please init lv_disp at first\n");
        hal_msleep(100);
    }

    struct lv_queue_context new_area;

    while (1) {
        // 1. wait queue
        hal_sem_wait(sunxifb_sem);
        hal_mutex_lock(sunxifb_mux);
        // 2. flush data
        lv_disp->flush();
        hal_mutex_unlock(sunxifb_mux);

#ifdef LV_USE_SUNXIFB_DEBUG
        static struct timeval new, old;
        static uint32_t cur_fps, avg_fps, max_fps, min_fps = 60, fps_cnt, first;
        gettimeofday(&new, NULL);
        if (new.tv_sec * 1000 - old.tv_sec * 1000 >= 1000) {
            if (first > 4) {
                if (first > 64) {
                    fps_cnt = 0;
                    avg_fps = 0;
                    max_fps = 0;
                    min_fps = 60;
                    first = 4;
                }
                fps_cnt++;
                avg_fps += cur_fps;
                if (max_fps < cur_fps)
                    max_fps = cur_fps;
                if (min_fps > cur_fps)
                    min_fps = cur_fps;
            }

            if (fps_cnt > 0)
                printf("sunxifb_flush fps_cnt=%u cur_fps=%u, max_fps=%u, "
                       "min_fps=%u, avg_fps=%.2f\n",
                       fps_cnt, cur_fps, max_fps, min_fps, (float)avg_fps / (float)fps_cnt);

            first++;
            old = new;
            cur_fps = 0;
        } else {
            cur_fps++;
        }
#endif /* LV_USE_SUNXIFB_DEBUG */
    }
}

static int lv_disp_thread_create(void)
{
    sunxifb_sem = hal_sem_create(0);
    if (sunxifb_sem == NULL) {
        return -1;
    }

    sunxifb_mux = hal_mutex_create();
    if (sunxifb_mux == NULL) {
        hal_sem_delete(sunxifb_sem);
        return -1;
    }

    sunxifb_flush_thread = hal_thread_create(lv_disp_thread, NULL, "lv_disp_thread", 1024,
                                             (HAL_THREAD_PRIORITY_APP + 2));
    if (sunxifb_flush_thread == NULL) {
        hal_sem_delete(sunxifb_sem);
        hal_mutex_delete(sunxifb_mux);
        return -1;
    }

    return 0;
}

static void lv_disp_thread_destory(void)
{
    hal_thread_stop(sunxifb_flush_thread);
    hal_mutex_delete(sunxifb_mux);
    hal_sem_delete(sunxifb_sem);
}

/*
* note : don't call this function in your app,
*        unless you what you do
*/
// static char* sunxifb_get_buf(void)
// {
//     if (lv_disp != 0) {
//         return lv_disp->get_buf();
//     }

//     return NULL;
// }

static int sunxifb_area_update(char *dst_buff, struct lv_queue_context *new_area)
{
    lv_disp_drv_t *drv = new_area->drv;
    lv_area_t *area = (lv_area_t *)new_area->area;
    uint8_t *color_p = (uint8_t *)new_area->color_p;

    uint32_t fbp_w = drv->hor_res;
    uint32_t fbp_h = drv->ver_res;

    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > (int32_t)fbp_w - 1 ? (int32_t)fbp_w - 1 : area->x2;
    int32_t act_y2 = area->y2 > (int32_t)fbp_h - 1 ? (int32_t)fbp_h - 1 : area->y2;

    int32_t color_byte = 0;
    if ((LV_COLOR_DEPTH == 32) || (LV_COLOR_DEPTH == 24)) {
        color_byte = 4;
    } else if (LV_COLOR_DEPTH == 16) {
        color_byte = 2;
    } else {
        printf("unsupport LV_COLOR_DEPTH %d\n", LV_COLOR_DEPTH);
        return -1;
    }

    uint32_t smem_len = fbp_w * fbp_h * color_byte;
    int flag_blit = -1;

#if USE_SUNXIFB_G2D
    // 添加g2d旋转后，需要修改的数据量一定大于设置的限制，直接忽略该条件判断
    // if (lv_area_get_size(area) >= sunxifb_g2d_get_limit(SUNXI_G2D_LIMIT_BLIT)) {
        /* clean cache */
        hal_dcache_clean((uintptr_t)color_p,
                         UINT32_UP_ALIGN64((area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1) *
                                           color_byte));
        hal_dcache_clean((uintptr_t)dst_buff, smem_len);
        hal_dcache_clean((uintptr_t)dst_buff + smem_len, smem_len);
        flag_blit = sunxifb_g2d_blit_to_fb((uintptr_t)color_p, (area->x2 - area->x1 + 1),
                                           (area->y2 - area->y1 + 1), 0, 0, (act_x2 - act_x1 + 1),
                                           (act_y2 - act_y1 + 1), (uintptr_t)dst_buff, fbp_w, fbp_h,
                                           area->x1, area->y1, (act_x2 - act_x1 + 1),
                                           (act_y2 - act_y1 + 1), G2D_BLT_NONE_H);
        // g2d旋转代码
        flag_blit = sunxifb_g2d_blit_to_fb((uintptr_t)dst_buff, fbp_w, fbp_h, 0, 0, fbp_w, fbp_h,
                                            (uintptr_t)dst_buff + smem_len, fbp_h, fbp_w, 0, 0, fbp_h, fbp_w,  G2D_ROT_90);
        hal_dcache_invalidate((uintptr_t)dst_buff, smem_len);
        hal_dcache_invalidate((uintptr_t)dst_buff + smem_len, smem_len);
    // }
#endif /* CONFIG_LVGL8_USE_SUNXIFB_G2D */

    if (flag_blit < 0) {
        lv_coord_t w = (act_x2 - act_x1 + 1);
        unsigned int line_lenght = (w * color_byte);
        long int location = 0;
        for (int32_t y = act_y1; y <= act_y2; y++) {
            location = (((fbp_w * y) + act_x1) * color_byte);
            memcpy(&dst_buff[location], color_p, line_lenght);
            color_p += line_lenght;
        }
    }
    return 0;
}
