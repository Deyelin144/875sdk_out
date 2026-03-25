#ifndef _REALIZE_UNIT_LOG_H_
#define _REALIZE_UNIT_LOG_H_

#include "stdio.h"
#include <stdbool.h>
#include "../../platform/gulitesf_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	REALIZE_UNIT_LOG_NOR,
	REALIZE_UNIT_LOG_JSAPP,
} realize_unit_log_type_t;

typedef enum {
    REALIZE_UNIT_LOG_CB_TYPE_ERROR = -1,
    REALIZE_UNIT_LOG_CB_TYPE_INFO = 0,  // 用于回调日志上传开始结束等信息
    REALIZE_UNIT_LOG_CB_TYPE_START= 1,
    REALIZE_UNIT_LOG_CB_TYPE_DATA,
    REALIZE_UNIT_LOG_CB_TYPE_END,
    REALIZE_UNIT_LOG_CB_TYPE_DELETE,
    REALIZE_UNIT_LOG_CB_TYPE_DESTROY,
} log_cb_type_t;

typedef enum {
    REALIZE_UNIT_LOG_CB_UPLOAD_CLOSED = 0,
    REALIZE_UNIT_LOG_CB_UPLOAD_OPEN = 1,
} log_info_t;

typedef enum {
    REALIZE_UNIT_LOG_ERROR_NO_ERR,
    REALIZE_UNIT_LOG_ERROR_FILE_OPEN,
    REALIZE_UNIT_LOG_ERROR_FILE_READ,
    REALIZE_UNIT_LOG_ERROR_FILE_BUFFER_FULL,
    REALIZE_UNIT_LOG_ERROR_FILE_FLUSH,
    REALIZE_UNIT_LOG_ERROR_QUEUE_BUSY,
    REALIZE_UNIT_LOG_ERROR_MALLOC,
    REALIZE_UNIT_LOG_ERROR_UPLOAD_INIT,    // 初始化失败
    REALIZE_UNIT_LOG_ERROR_UPLOAD_ABORT,   // 上传被中断
    REALIZE_UNIT_LOG_ERROR_UPLOAD_PARAM,   // 参数异常
    REALIZE_UNIT_LOG_ERROR_UPLOAD_REQUEST, // 请求失败
} log_error_type_t;

typedef struct realize_unit_log_ctx {
    void *user_data;
    int (*log_cb)(void *user_data, log_cb_type_t type, void *data, int len);
    int (*log_getdata_cb)(void *, log_cb_type_t type, void *data, int len);
} realize_unit_log_ctx_t;

typedef struct {
    void (*print)(char *buffer); // 因为从printf函数接管了sdk的日志，所以在日志系统内某些地方不能再用printf打印，直接使用底层输出
    void (*flush_to_buffer)(void); // 将sdk的日志buffer flush到引擎日志系统缓冲区，方便上传或者写文件
    unsigned char (*is_critical_ctx)(void); // 是否关键上下文（IRQ、FIQ、ISR、Scheduler），关键上下文内不使用互斥锁
} unit_log_hook_t;

typedef enum {
    REALIZE_UNIT_LOG_LEVEL_PRINT = -1,
	REALIZE_UNIT_LOG_LEVEL_VERBOSE = 0,
	REALIZE_UNIT_LOG_LEVEL_DEBUG,
	REALIZE_UNIT_LOG_LEVEL_INFO,
	REALIZE_UNIT_LOG_LEVEL_WARN,
	REALIZE_UNIT_LOG_LEVEL_ERROR,
	REALIZE_UNIT_LOG_LEVEL_JS_DEBUG,
	REALIZE_UNIT_LOG_LEVEL_JS_INFO,
	REALIZE_UNIT_LOG_LEVEL_JS_WARN,
	REALIZE_UNIT_LOG_LEVEL_JS_ERROR,
} realize_unit_log_level_t;

#define REALIZE_UNIT_LOG_CLEAN_UPLOAD (1 << 0)
#define REALIZE_UNIT_LOG_CLEAN_SAVE (1 << 1)

#ifdef CONFIG_USE_EASYLOGGER
void realize_unit_log_raw(const char *format, ...);
void _realize_unit_log_add(realize_unit_log_type_t type, realize_unit_log_level_t level, const char * file, int line, const char * func, const char * format, ...);
#define realize_unit_log_verbose(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_VERBOSE, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define realize_unit_log_debug(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define realize_unit_log_info(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define realize_unit_log_warn(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define realize_unit_log_error(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define realize_unit_log_print(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_PRINT, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define realize_unit_log_js_debug(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_JSAPP, REALIZE_UNIT_LOG_LEVEL_JS_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define realize_unit_log_js_info(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_JSAPP, REALIZE_UNIT_LOG_LEVEL_JS_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define realize_unit_log_js_warn(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_JSAPP, REALIZE_UNIT_LOG_LEVEL_JS_WARN, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define realize_unit_log_js_error(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_JSAPP, REALIZE_UNIT_LOG_LEVEL_JS_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#else
void _realize_unit_log_add(realize_unit_log_type_t type, realize_unit_log_level_t level, const char * file, int line, const char * func, const char * format, ...);

#define REALIZE_UNIT_LOG_COLOR_BLACK_BG   "40"
#define REALIZE_UNIT_LOG_COLOR_RED_BG     "41"
#define REALIZE_UNIT_LOG_COLOR_GREEN_BG   "42"
#define REALIZE_UNIT_LOG_COLOR_BROWN_BG   "43"
#define REALIZE_UNIT_LOG_COLOR_BLUE_BG    "44"
#define REALIZE_UNIT_LOG_COLOR_PURPLE_BG  "45"
#define REALIZE_UNIT_LOG_COLOR_CYAN_BG    "46"
#define REALIZE_UNIT_LOG_COLOR_WRITE_BG   "47"

#define REALIZE_UNIT_LOG_COLOR_BLACK   "30"
#define REALIZE_UNIT_LOG_COLOR_RED     "31"
#define REALIZE_UNIT_LOG_COLOR_GREEN   "32"
#define REALIZE_UNIT_LOG_COLOR_BROWN   "33"
#define REALIZE_UNIT_LOG_COLOR_BLUE    "34"
#define REALIZE_UNIT_LOG_COLOR_PURPLE  "35"
#define REALIZE_UNIT_LOG_COLOR_CYAN    "36"
#define REALIZE_UNIT_LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define REALIZE_UNIT_LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define REALIZE_UNIT_LOG_RESET_COLOR   "\033[0m"

#define REALIZE_UNIT_LOG_COLOR_E_JA		REALIZE_UNIT_LOG_COLOR(REALIZE_UNIT_LOG_COLOR_RED_BG";"REALIZE_UNIT_LOG_COLOR_BLACK)
#define REALIZE_UNIT_LOG_COLOR_W_JA		REALIZE_UNIT_LOG_COLOR(REALIZE_UNIT_LOG_COLOR_BROWN_BG";"REALIZE_UNIT_LOG_COLOR_BLACK)
#define REALIZE_UNIT_LOG_COLOR_I_JA		REALIZE_UNIT_LOG_COLOR(REALIZE_UNIT_LOG_COLOR_GREEN_BG";"REALIZE_UNIT_LOG_COLOR_BLACK)
#define REALIZE_UNIT_LOG_COLOR_D_JA		REALIZE_UNIT_LOG_COLOR(REALIZE_UNIT_LOG_COLOR_WRITE_BG";"REALIZE_UNIT_LOG_COLOR_BLACK)

#define REALIZE_UNIT_LOG_COLOR_E		REALIZE_UNIT_LOG_COLOR(REALIZE_UNIT_LOG_COLOR_RED)
#define REALIZE_UNIT_LOG_COLOR_W		REALIZE_UNIT_LOG_COLOR(REALIZE_UNIT_LOG_COLOR_BROWN)
#define REALIZE_UNIT_LOG_COLOR_I		REALIZE_UNIT_LOG_COLOR(REALIZE_UNIT_LOG_COLOR_GREEN)
#define REALIZE_UNIT_LOG_COLOR_D		//REALIZE_UNIT_LOG_COLOR(REALIZE_UNIT_LOG_COLOR_CYAN)
#define REALIZE_UNIT_LOG_COLOR_V

#define REALIZE_UNIT_LOG_FORMAT(letter, format)  REALIZE_UNIT_LOG_COLOR_ ## letter format REALIZE_UNIT_LOG_RESET_COLOR "\n"

#define realize_unit_log_verbose(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_VERBOSE, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(V, format), ##__VA_ARGS__)
#define realize_unit_log_debug(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_DEBUG, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(D, format), ##__VA_ARGS__)
#define realize_unit_log_info(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_INFO, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(I, format), ##__VA_ARGS__)
#define realize_unit_log_warn(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_WARN, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(W, format), ##__VA_ARGS__)
#define realize_unit_log_error(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_NOR, REALIZE_UNIT_LOG_LEVEL_ERROR, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(E, format), ##__VA_ARGS__)
#define realize_unit_log_print(format, ...) printf(format"\n", ##__VA_ARGS__)
#define realize_unit_log_raw(format, ...) printf(format, ##__VA_ARGS__)

#define realize_unit_log_js_debug(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_JSAPP, REALIZE_UNIT_LOG_LEVEL_JS_DEBUG, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(D_JA, format), ##__VA_ARGS__)
#define realize_unit_log_js_info(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_JSAPP, REALIZE_UNIT_LOG_LEVEL_JS_INFO, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(I_JA, format), ##__VA_ARGS__)
#define realize_unit_log_js_warn(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_JSAPP, REALIZE_UNIT_LOG_LEVEL_JS_WARN, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(W_JA, format), ##__VA_ARGS__)
#define realize_unit_log_js_error(format, ...) _realize_unit_log_add(REALIZE_UNIT_LOG_JSAPP, REALIZE_UNIT_LOG_LEVEL_JS_ERROR, NULL, __LINE__, __func__, REALIZE_UNIT_LOG_FORMAT(E_JA, format), ##__VA_ARGS__)
#endif

void realize_unit_log_ctx_init(realize_unit_log_ctx_t *ctx);
realize_unit_log_ctx_t *realize_unit_log_get_ctx();
void realize_unit_log_ctx_deinit(void);
void realize_unit_log_set_output_to_terminal(bool enable);
int realize_unit_log_flush_buffer(void);
int realize_unit_log_clean_file(const char *dir, int clean_type);
void realize_unit_log_set_level(int type, int level);
int realize_unit_log_get_level(int type);
unsigned char realize_unit_log_is_inited(void);
void realize_unit_log_set_hook(unit_log_hook_t *hook);
unit_log_hook_t *realize_unit_log_get_hook(void);
unsigned char realize_unit_log_get_write_status(void);
void _realize_unit_log_suspend(const char *func, unsigned char suspend);
#define realize_unit_log_suspend(suspend) _realize_unit_log_suspend(__func__, suspend);
void realize_unit_log_init();

#ifdef __cplusplus
}
#endif

#endif
