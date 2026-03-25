#ifndef __REALIZE_UNIT_SYSYTIMER_H__
#define __REALIZE_UNIT_SYSYTIMER_H__

#include <stdbool.h>
#include "../realize_unit_hash/realize_unit_hash.h"

typedef struct timer_wrap {
    unsigned int key;
    void *timer_id;
    void *user_data;
    uint32_t remain_time;    // 剩余时间，单位ms
} unit_timer_wrap_t;

typedef enum {
    TIMER_STOP_CMD,
    TIMER_START_CMD
} unit_timer_cmd_e;

int realize_unit_systimer_init(void);
hash_t *realize_unit_systimer_get_timer_hash_table();
unsigned int realize_unit_systimer_create_timer(void *user_data, bool repeat, unsigned int ms, void *func);
int realize_unit_systimer_clear_timer(unsigned int id, void *free_userdata_cb);
int realize_unit_systimer_set_all_timer(unsigned char cmd);
int realize_unit_systimer_marker_clear_timer(unsigned int id);
int realize_unit_systimer_get_new_id(void);

int realize_unit_systimer_stop_timer(unit_timer_wrap_t *timer);
int realize_unit_systimer_start_timer(unit_timer_wrap_t *timer);
int realize_unit_systimer_get_remaining_time(unit_timer_wrap_t *timer);

int realize_unit_systimer_is_need_clean(void *ptr);
int realize_unit_systimer_is_repeat(void *ptr);
#endif
