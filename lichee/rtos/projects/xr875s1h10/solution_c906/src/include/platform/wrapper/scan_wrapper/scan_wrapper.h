#ifndef _SCAN_WRAPPER_H_
#define _SCAN_WRAPPER_H_

#include "../../gulitesf_config.h"
#include "../../../mod_realize/realize_unit_scan/realize_unit_scan.h"
#ifdef CONFIG_FUNC_SCAN_READ

/**
 * @brief 扫描模块初始化
 * @param cb：回调
 * @return 成功则返回0，失败返回-1
 */
typedef int (*scan_init_t)(scan_cb_t *cb);

/**
 * @brief 扫描模块反初始化
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*scan_deinit_t)(void);

/**
 * @brief nlp查询
 * @param content：查询的内容
 * @param opt：需要查询的操作
 * @return 成功则返回0，失败返回-1
 */
typedef int (*start_nlp_t)(const char *content, int opt);

/**
 * @brief stop nlp查询
 * @param void
 * @return void
 */

typedef int (*stop_nlp_t)(void);

/**
 * @brief 设置tts语速
 * @param speed 语速 [0.7,1.2]
 * @return 成功则返回0，失败返回-1
 */

typedef int (*set_speed_t)(float speed);

typedef int (*scan_control_t)(char *json_str);

/**
 * @brief 设置摄像头扫描的左右手模式
 * @param mode 左手：0 ； 右手：1
 * @return 成功则返回0，失败返回-1
 */

typedef int (*set_handedness_t)(int mode);

typedef struct {
	scan_init_t init;
	scan_deinit_t deinit;
	start_nlp_t start_nlp;
	stop_nlp_t stop_nlp;
	set_speed_t set_speed;
	scan_control_t control;
	set_handedness_t set_handedness;
} scan_wrapper_t;

#endif
#endif
