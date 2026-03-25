#include "lv_init.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "FreeRTOS.h"
#include "lvgl.h"
#include "src/extra/libs/freetype/lv_freetype.h"
#include <stdio.h>
#include "kernel/os/os.h"
#include <stdio.h>
#include <task.h>
#include "../src/fs_adapter/fs_adapter.h"
#include "../src/os_adapter/os_adapter.h"
#include "recovery_main.h"
#include "drivers/key/dev_key.h"
#include "ota_msg.h"


static void *s_lv_task_tid = NULL;
static void *s_lv_mutex = NULL;
static int s_lv_task_stop = 0;

#if LV_USE_FREETYPE
#define TTF_USE_MEM

// #define FONT_BIN_PATH "P:"APP_DIR"/ota_font.bin"
#define FONT_FT_PATH APP_DIR"/font.ttf"
#define FONT_SIZE 22

static lv_font_t *s_font_ft = NULL;
// static lv_font_t *s_font_bin = NULL;

lv_font_t *lv_get_ft_font()
{
    return (lv_font_t *)s_font_ft;
}

// lv_font_t *lv_get_bin_font()
// {
//     return (lv_font_t *)s_font_ft;
// }

lv_font_t *lv_get_freetype_font(char *path, int font_size, int style)
{
	lv_font_t *font = NULL;
	lv_ft_info_t ft_info = { 0 }; 
	char font_path[128] = {0};
#ifdef TTF_USE_MEM
	unsigned int len = 0;
    FILE *font_file = NULL;
#endif
    unsigned char *font_buffer = NULL;
    unsigned int size = 0;

#ifdef TTF_USE_MEM
    font_file = fs_adapter()->fs_open(path, UNIT_FS_RDONLY);
    if (!font_file) {
        LOG_ERR("open file %s failed\n", path);
        goto exit;
    }

    fs_adapter()->fs_seek(font_file, 0, UNIT_FS_SEEK_END);
    fs_adapter()->fs_tell(font_file, &size);
    printf("file %s size %u\n", path, size);
    fs_adapter()->fs_seek(font_file, 0, UNIT_FS_SEEK_SET);

    font_buffer = (unsigned char*)os_adapter()->calloc(1, size);
    if (!font_buffer) {
        LOG_ERR("malloc err %u\n", size);
        goto exit;
    }

    fs_adapter()->fs_read(font_file, font_buffer, size, &len);
    if (len != size) {
        LOG_ERR("font file read err %u %u\n", len, size);
        goto exit;
    }
#endif

#ifdef TTF_USE_MEM
	ft_info.name = NULL;
#else
	ft_info.name = path;
#endif
    ft_info.weight = font_size;
    ft_info.style = style;
    ft_info.mem = font_buffer;
	ft_info.mem_size = size;

    if(!lv_ft_font_init(&ft_info)) {
        LOG_ERR("font init failed.\n");
		goto exit;
    }
    printf("load font %s suc!\n", path);
	font = ft_info.font;

exit:
    // psram_free(font_buffer);
#ifdef TTF_USE_MEM
    fs_adapter()->fs_close(font_file);
#endif
	return font;
}
#endif


// lv_font_t *lv_load_font_bin(char *path)
// {
// 	lv_font_t *font = NULL;

//     font = lv_font_load(path);
//     if (font == NULL) {
//         LOG_ERR("load %s failed.\n", path);
//     } else {
//         printf("load %s suc!\n", path);
//     }

// 	return font;
// }

void lv_mutex_init(void)
{
    if (NULL == s_lv_mutex) {
        void *mutex = NULL;
        if ((mutex = os_adapter()->mutex_create()) == NULL) {
            LOG_ERR("[%d], %s failed\n", __LINE__, __func__);
        }
	    s_lv_mutex = mutex;
    }
}

void lv_lock_ex(const char *func)
{
    // printf("lv_lock:%s\n", func);
    if (NULL != s_lv_mutex) {
        if (0 != os_adapter()->mutex_lock(s_lv_mutex, os_adapter()->get_forever_time())) {
            LOG_ERR("[%d], %s failed.\n", __LINE__, __func__);
        }
    }
}

void lv_unlock_ex(const char *func)
{
    // printf("lv_unlock:%s\n", func);
    if (NULL != s_lv_mutex) {
        if (0 != os_adapter()->mutex_unlock(s_lv_mutex)) {
            LOG_ERR("[%d], %s failed.\n", __LINE__, __func__);
        }
    }
}

void _lv_task(void *arg)
{
    int ret = 50;
    printf("lv_task start\n");
    while (0 == s_lv_task_stop) {
        lv_lock();
        // printf("ret = %d\n", ret);
        ret = lv_task_handler();
        lv_unlock();
        // if (ret < 20) {
        //     ret = 20;
        // }
        // os_adapter()->msleep(ret);
        os_adapter()->msleep(40);
    
    }

    os_adapter()->thread_delete(&s_lv_task_tid);
}

int lv_task(upgrade_handle_t *handle)
{
    int ret = -1;

    if (NULL != s_lv_task_tid) {
        goto exit;
    }

    dev_key_config_info_t dev_para = {0};
    if (handle->key.enable) {
        dev_para.type = handle->key.type == -1 ? 0 : handle->key.type;
        ret = dev_key_init(&dev_para);
        if (0 != ret) {
            LOG_ERR("dev key init error!\n");
            goto exit;
        }
    }
    printf("lv_init\n");
    lv_init();

    printf("lv_mutex_init\n");
    lv_mutex_init();

    printf("lv_disp_init\n");
    lv_port_disp_init(handle);

    printf("lv_indev_init\n");
	lv_port_indev_init(handle);

    lv_task_handler();

#if LV_USE_FREETYPE
    s_font_ft = lv_get_freetype_font(FONT_FT_PATH, FONT_SIZE, FT_FONT_STYLE_NORMAL);
#endif

    // s_font_bin = lv_load_font_bin(FONT_BIN_PATH);

    ret = os_adapter()->thread_create(&s_lv_task_tid,
                                      _lv_task,
                                      NULL,
                                      "_lv_task",
                                      OS_ADAPTER_PRIORITY_ABOVE_NORMAL,
                                      1024 * 20);
    if (ret != 0) {
        printf("create lv thread failed ret = %d.\n", ret);
        s_lv_task_tid = NULL;
    }

exit:
    return ret;
}

