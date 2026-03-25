#ifndef _MODULE_NATIVE_START_H_
#define _MODULE_NATIVE_START_H_

#include "../../mod_realize/realize_unit_input/realize_unit_input.h"
#include "../../mod_realize/realize_unit_wifi/realize_unit_wifi.h"
#include "../../mod_realize/realize_unit_sleep/realize_unit_sleep.h"
#include "../../mod_realize/realize_unit_battery/realize_unit_battery.h"
#include "../../mod_realize/realize_unit_device/realize_unit_device.h"
#include "../../mod_realize/realize_unit_kv/realize_unit_kv.h"
#include "../../mod_realize/realize_unit_fs/realize_unit_fs.h"
#include "../../mod_realize/realize_unit_mem/realize_unit_mem.h"
#include "../../mod_realize/realize_unit_log/realize_unit_log.h"
#include "../../mod_realize/realize_unit_player/realize_unit_player.h"
#include "../../mod_realize/realize_unit_gui_swipe.h"
#include "../../platform/wrapper/wrapper.h"

// #include "module_native_sync_notify.h"
// #include "../../GMP/gmp_module_logic/module_js_sync_notify.h"
#include "../../native/native_app/native_app_input/native_app_input.h"

typedef enum {
	NATIVE_NORMAL_MODE,
	NATIVE_TEST_MODE,
	NATIVE_AGING_MODE,
	NATIVE_NONE_MODE,
} module_native_mode_t;

typedef enum {
    ALLOW_TEST_PASS,
    ALLOW_TEST_FAIL,
    ALLOW_TEST_NOT_COMP,
} allow_test_res_t;

#define NATIVE_APP_MAX_SIZE 20
#define NATIVE_APP_SUPPORT_MAX_SIZE 10
#define NATIVE_INPUT_MAX_SIZE NATIVE_APP_SUPPORT_MAX_SIZE

typedef struct {
	char name[NATIVE_APP_MAX_SIZE];
	void (*native_app_enter)(void *);				//原生app进入函数句柄
	void (*native_app_exit)(void *);				//原生app退出函数句柄
} native_app_t;

typedef struct {
	native_app_t native_app[NATIVE_APP_SUPPORT_MAX_SIZE];
	native_input_t native_input[NATIVE_INPUT_MAX_SIZE];
	player_play_cb_t native_player_play_cb;		//原生app 播放回调
	wifi_state_cb_t native_wifi_state_cb;		//原生app wifi状态变更回调
	unit_sleep_param_t native_sleep_param;
	unit_mem_conf_t mem_conf;
} module_func_cb_t;

typedef struct {
	int boot_start;
    allow_test_res_t allow_test;
	module_native_mode_t boot_curr_mode;
	void *native_mode_mutex;
	module_func_cb_t module_func_cb;
} native_start_t;

void module_native_start(module_func_cb_t *func_cb, wrapper_t *wrapper_cb, const char *auth_path);
void module_native_pause(void);
void module_native_resume(void);
native_start_t *module_native_start_get_param(void);
void module_native_set_allow_test(allow_test_res_t res);

#endif
