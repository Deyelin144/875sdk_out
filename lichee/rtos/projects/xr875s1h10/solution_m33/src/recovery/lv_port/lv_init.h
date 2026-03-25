
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "../src/upgrade_logic/upgrade_logic.h"

#define lv_lock() lv_lock_ex(__func__)
#define lv_unlock() lv_unlock_ex(__func__)

void lv_lock_ex(const char* func);
void lv_unlock_ex(const char *func);
int lv_task(upgrade_handle_t *handle);
lv_font_t *lv_get_ft_font();
// lv_font_t *lv_get_bin_font();