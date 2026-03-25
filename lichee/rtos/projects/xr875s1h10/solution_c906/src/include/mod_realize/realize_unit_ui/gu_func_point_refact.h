/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.13
  *Description:  函数指针反射
  *History:  
**********************************************************************************/

#include "lvgl.h"
#include "gu_attr_type.h"
#include <stdarg.h>
#ifndef GU_FUNC_POINT_REFACT_H
#define GU_FUNC_POINT_REFACT_H

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*funcp_in_p_int)(void *,  int);
typedef void (*funcp_in_p_int_int)(void *, int, int);
typedef void *(*funcp_in_p_p_out_p)(void *, void*);
typedef void (*funcp_in_p_p_int)(void *, void *, int);
typedef void (*funcp_in_p_p_int_p)(void *, void *, int, void *);
typedef void (*funcp_in_p_p)(void *, void *);
typedef void (*funcp_in_p_c)(void *, lv_color_t);
typedef void (*funcp_in_p_p_int_int_int)(void *, void *, int,int, int);
typedef void (*funcp_in_p_int_int_int)(void *, int,int, int);
typedef int (*funcp_in_p_out_int)(void *);
typedef void *(*funcp_in_p_out_p)(const void *);
typedef void *(*funcp_in_p_int_c_c_out_p)(void *, int, lv_color_t, lv_color_t);
typedef void (*funcp_in_p)(void *);
typedef void (*funcp_in_p_c_int)(void *, lv_color_t, int);

//函数指针类别
typedef enum {
    FUNCP_IN_P_INT, //(*funcp_in_point_int)(vvoid *, int)
    FUNCP_IN_P_INT_INT, //void (*funcp_in_point_int_int)(void *, int,int)
    FUNCP_IN_P_P_OUT_P,  //void *(*funcp_in_p_p_out_p)(void *, void*);
    FUNCP_IN_P_P_INT, // void (*funcp_in_p_p_int)(void *, void *, int);
    FUNCP_IN_P_P,// void (*funcp_in_p_p)(void *, void *);
    FUNCP_IN_P_OUT_P, // void *(*funcp_in_p_out_p)(void *);
    FUNCP_IN_P_OUT_INT, // int (*funcp_in_p_out_p)(void *);
    FUNCP_IN_P_P_INT_P, //void (*funcp_in_p_p_int_p)(void *, void *, int, void *);
    FUNCP_IN_P_C,  //void (*funcp_in_p_c)(void *, lv_color_t);   
    FUNCP_IN_P_P_INT_INT_INT,
    FUNCP_IN_P_INT_INT_INT,
    FUNCP_IN_P_INT_C_C_OUT_P,
    FUNCP_IN_P,    
    FUNCP_IN_P_C_INT
}fun_point_type;

typedef struct gu_func_point_refact
{
  int funIndex; //函数的索引编号 //要和数组下标一致  
  char *domname;   //dom名称
  char *attrname;  //attr名称
  void *funcP; //设置函数指针
  char *attrValueType; //属性值类型
  void *funcPConver; //属性转化函数指针
  char *funcName; //要删除，只是用于调试
  fun_point_type type; //函数的类别
  void *funcGetP; //获取属性函数指针，用于双向绑定
  fun_point_type get_func_type; //获取指针函数的类别
  int attr_order; //attr属性的排序，用于某些dom的一些属性必须要优先设置
}guFuncPoint_t;

int call_func_p(guFuncPoint_t *func, void **out, void *obj, va_list ap);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_CSS_H*/