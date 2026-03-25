#ifndef _REALIZE_UNIT_GUI_SWIPE_H_
#define _REALIZE_UNIT_GUI_SWIPE_H_

#include "realize_unit_ui/gu_lvgl.h"
#include "../components/GUI/lvgl/lvgl.h"

#define FONT_SYMBOL_GU  "\xee\x9b\x88"

typedef enum {
    GUI_SET_PRIORITY_UP,
    GUI_SET_PRIORITY_DOWN,
    GUI_SET_PRIORITY_UP_HIGH
} gui_prior_conf_t;

typedef void (*lvgl_start)(void *);
void realize_unit_gui_swipe_init(lvgl_start start_cb);
void realize_unit_gui_swipe_deinit(void);
#ifdef CONFIG_LV_USE_FREETYPE
lv_font_t *realize_unit_gui_swipe_get_freetype_font(char *path, int font_size, int style, int is_use_mem);
#endif
lv_font_t *realize_unit_gui_swipe_get_font();
void realize_unit_gui_swipe_set_prior(int cmd);

#endif

