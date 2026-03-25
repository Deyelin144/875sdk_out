#include "sdk_entry.h"

#include "FreeRTOS.h"
#include "lvgl.h"
#include "drivers/pm/dev_pm.h"
#include "kernel/os/os.h"
#include "realize/realize_lv_port/lv_port_disp.h"
#include "realize/realize_lv_port/lv_port_indev.h"
#include "sdk_runtime.h"

static XR_OS_Thread_t s_sdk_entry_thread;

static void ui_demo_show(void)
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_t *title = lv_label_create(screen);

    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_label_set_text(title, "gurobot");
    lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);
}

__attribute__((weak)) void sdk_user_app_init(void)
{
    ui_demo_show();

    if (fs_copy_demo() != 0) {
        printf("[fs_demo] app_config copy failed.\n");
    }
}

static void sdk_entry_thread(void *arg)
{
    (void)arg;

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    sdk_runtime_set_backlight_percent(sdk_runtime_get_backlight_percent());
    sdk_user_app_init();

    while (1) {
        lv_timer_handler();
        XR_OS_MSleep(20);
    }
}

int sdk_entry_init(void)
{
    if (XR_OS_ThreadIsValid(&s_sdk_entry_thread)) {
        return 0;
    }

    if (XR_OS_OK != XR_OS_ThreadCreate(&s_sdk_entry_thread,
                                       "sdk_entry",
                                       sdk_entry_thread,
                                       NULL,
                                       XR_OS_PRIORITY_NORMAL,
                                       (20 * 1024))) {
        printf("[sdk_entry] init task create fail.\n");
        return -1;
    }

    return 0;
}
