#ifndef LV_HZ_STROKE_H
#define LV_HZ_STROKE_H

#ifdef __cplusplus
extern "C" {
#endif



/*********************
 *      INCLUDES
 *********************/
#include "lv_conf_internal.h"
#include "core/lv_obj.h"

typedef enum{
    HZ_STROKE_STATUS_STOP = 0,
    HZ_STROKE_STATUS_PLAY,
    HZ_STROKE_STATUS_PAUSE,
    HZ_STROKE_STATUS_RESET,
    HZ_STROKE_STATUS_RESUME,
}HZ_STROKE_STATUS;

typedef struct
{
    lv_point_t *point;
    short count;
} HZ_POINTS;

typedef struct
{
    lv_area_t coords;
    lv_opa_t *mask_buf;
}HZ_ONE_STROKE;

#define MAX_POINTS 1024  
typedef struct 
{
    HZ_POINTS *strokes_points;
    HZ_ONE_STROKE *strokes;
    HZ_POINTS *path;
    int strokes_count;
}HZ_STROKE;

typedef struct {
    lv_obj_t obj;
    HZ_STROKE strokes;
    lv_timer_t * timer;    
    int current_stroke;
    int current_path_point;
    bool repeat;                    //是否循环播放
    HZ_STROKE_STATUS status;        //描红状态
    int speed;                      //描红速度
    lv_color_t line_color;          //米字格颜色     
    lv_color_t stroke_bg_color;     //描红的背景色
    lv_color_t stroke_color;        //描红的颜色
    lv_color_t cur_stroke_color;        //描红的颜色

} lv_hzstroke_t;

extern const lv_obj_class_t lv_hzstroke_class;


lv_obj_t * lv_hzstroke_create(lv_obj_t * parent);
void lv_hzstroke_set_src(lv_obj_t * obj, char * src);
void lv_hzstroke_set_speed(lv_obj_t * obj, int speed);
void lv_hzstroke_set_status(lv_obj_t * obj, HZ_STROKE_STATUS status);
void lv_hzstroke_set_line_color(lv_obj_t * obj, lv_color_t color);
void lv_hzstroke_set_stroke_bg_color(lv_obj_t * obj, lv_color_t color);
void lv_hzstroke_set_stroke_color(lv_obj_t * obj, lv_color_t color);
void lv_hzstroke_set_cur_stroke_color(lv_obj_t * obj, lv_color_t color);
void lv_hzstroke_set_repeat(lv_obj_t * obj, bool repeat);
#endif /*LV_HZ_STROKE_H*/

#ifdef __cplusplus
} /*extern "C"*/
#endif
