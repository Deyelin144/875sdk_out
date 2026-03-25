#ifndef __DEV_TOUCH_AREA_H__
#define __DEV_TOUCH_AREA_H__

typedef enum {
    TOUCH_AREA_1 = 0,
    TOUCH_AREA_2,
    TOUCH_AREA_3,
    TOUCH_AREA_4,
    TOUCH_AREA_5,
    TOUCH_AREA_6,
    TOUCH_AREA_7,
    TOUCH_AREA_8,
    TOUCH_AREA_9,
    TOUCH_AREA_10,
    TOUCH_AREA_11,
    TOUCH_AREA_MAX
} touch_area_index_t;

typedef enum {
    TOUCH_AREA_EVENT_DOWN = 1,    // 按下
    TOUCH_AREA_EVENT_UP,          // 抬起
    TOUCH_AREA_EVENT_LONG,        // 长按
    TOUCH_AREA_EVENT_DOUBLE,      // 双击
    TOUCH_AREA_EVENT_SLIDER,      // 滑动
} touch_area_event_t;

typedef struct point {
    uint16_t x;
    uint16_t y;
} touch_area_point_t;

typedef struct {
    char type[16];
    char event[16];
    touch_area_point_t start;
    touch_area_point_t end;
} touch_area_pos_t;

int dev_touch_area_init(void);
int dev_touch_area_deinit(void);
int dev_touch_area_config(touch_area_pos_t *pos, int num);
void dev_touch_area_update_state(int x, int y, int pressed);

#endif /* __DEV_TOUCH_AREA_H__ */
