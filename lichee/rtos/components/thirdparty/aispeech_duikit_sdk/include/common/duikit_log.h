#ifndef DUIKIT_LOG_H
#define DUIKIT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "duikit_common.h"


/*
 * 日志组件
 * 特点
 *  支持多线程环境使用
 *  支持动态开关日志输出
 *  支持动态设置全局日志输出等级
 *  支持hexdump
 *  支持DEBUG/INFO/WARN/ERROR等级输出
 *  支持tag过滤及等级设定
 *  支持时间戳（标准时间/精简时间格式）
 */

/*
 * 日志等级
 */
#define DUIKIT_LOG_LEVEL_DEBUG 0
#define DUIKIT_LOG_LEVEL_INFO 1
#define DUIKIT_LOG_LEVEL_WARNING 2
#define DUIKIT_LOG_LEVEL_ERROR 3

/*
 * 默认日志输出等级
 */
#ifndef DUIKIT_LOG_OUTPUT_LEVEL
#define DUIKIT_LOG_OUTPUT_LEVEL DUIKIT_LOG_LEVEL_DEBUG
#endif

/*
 * 自定义日志输出函数
 */
typedef void (*duikit_log_custom_output)(int level, const char *tag, const char *func, int line, const char *format, va_list list);
typedef void (*duikit_log_custom_hexdump)(int level, const char *tag, const char *func, int line, const char *buf, int len, const char *format, va_list list);

/*
 * 日志输出函数
 */
DUIKIT_EXPORT void duikit_log_output(int level, const char *tag, const char *func, int line, const char *format, ...);
DUIKIT_EXPORT void duikit_log_hexdump(int level, const char *tag, const char *func, int line, const char *buf, int len, const char *format, ...);

/*
 * 日志模块配置
 */
typedef struct duikit_log_config {
    int priority;   //日志任务优先级
    int stack_size; //日志任栈大小
    int core_id;    //日志任务运行的核心ID
    int ext_stack;  //日志任务使用扩展栈空间
} duikit_log_config_t;

/*
 * 初始化日志模块
 */
DUIKIT_EXPORT int duikit_log_init(duikit_log_config_t *config);

/*
 * 日志输出开关
 * 默认打开
 */
DUIKIT_EXPORT void duikit_log_set_output(bool enable);

/*
 * 日志输出等级
 * 默认为DUIKIT_LOG_OUTPUT_LEVEL
 */
DUIKIT_EXPORT void duikit_log_set_output_level(int level);

/*
 * 设置特定TAG的输出等级
 *
 * 支持”*“操作为所有TAG设置相同的等级
 *
 * 举例
 * 将“main”设置为DUIKIT_LOG_LEVEL_INFO
 * duikit_log_set_tag_level("main", DUIKIT_LOG_LEVEL_INFO);
 *
 * 所有tag设置为DUIKIT_LOG_LEVEL_WARNING
 * duikit_log_set_tag_level("*", DUIKIT_LOG_LEVEL_WARNING);
 */
DUIKIT_EXPORT void duikit_log_set_tag_level(const char *tag, int level);
DUIKIT_EXPORT void duikit_log_deinit();

/*
 * 可以直接使用DUIKIT_LOG_{D/I/W/E}接口，但是必须在include duikit_log.h之前定义DUIKIT_LOG_TAG和DUIKIT_LOG_LVL宏
 * 例如
 * #define DUIKIT_LOG_TAG "foo"
 * #define DUIKIT_LOG_LVL DUIKIT_LOG_LEVEL_DEBUG
 * DUIKIT_LOG_D("debug");
 * DUIKIT_LOG_I("info");
 * DUIKIT_LOG_W("warning");
 * DUIKIT_LOG_E("error");
 *
 *
 * 如果某个文件包含多个模块，每个模块需要使用不同的TAG，可以使用DUIKIT_LOG_DEBUG/DUIKIT_LOG_INFO等接口实现
 * 举例
 * 需要使用foo
 * #define FOO_D(TAG, format, ...) DUIKIT_LOG_DEBUG(#TAG, __func__, __LINE__, format, ##__VA_ARGS__)
 *
 * FOO_D(foo, "hello")
 */
#if 1
#define DUIKIT_LOG_D(format, ...) DUIKIT_LOG_DEBUG(DUIKIT_LOG_TAG, __func__, __LINE__, format, ##__VA_ARGS__)
#define DUIKIT_LOG_I(format, ...) DUIKIT_LOG_INFO(DUIKIT_LOG_TAG, __func__, __LINE__, format, ##__VA_ARGS__)
#define DUIKIT_LOG_W(format, ...) DUIKIT_LOG_WARN(DUIKIT_LOG_TAG, __func__, __LINE__, format, ##__VA_ARGS__)
#define DUIKIT_LOG_E(format, ...) DUIKIT_LOG_ERROR(DUIKIT_LOG_TAG, __func__, __LINE__, format, ##__VA_ARGS__)
#else
#define DUIKIT_LOG_D(format, args...) printf("[D][" DUIKIT_LOG_TAG "][%s:%d]:" format "\n", __func__, __LINE__, ##args)
#define DUIKIT_LOG_I(format, args...) printf("[I][" DUIKIT_LOG_TAG "][%s:%d]:" format "\n", __func__, __LINE__, ##args)
#define DUIKIT_LOG_W(format, args...) printf("[W][" DUIKIT_LOG_TAG "][%s:%d]:" format "\n", __func__, __LINE__, ##args)
#define DUIKIT_LOG_E(format, args...) printf("[E][" DUIKIT_LOG_TAG "][%s:%d]:" format "\n", __func__, __LINE__, ##args)
#endif
#define DUIKIT_LOG_HEX_D(format, buf, len, ...) DUIKIT_LOG_HEX_DUMP_D(DUIKIT_LOG_TAG, __func__, __LINE__, format, buf, len, ##__VA_ARGS__)
#define DUIKIT_LOG_HEX_I(format, buf, len, ...) DUIKIT_LOG_HEX_DUMP_I(DUIKIT_LOG_TAG, __func__, __LINE__, format, buf, len, ##__VA_ARGS__)
#define DUIKIT_LOG_HEX_W(format, buf, len, ...) DUIKIT_LOG_HEX_DUMP_W(DUIKIT_LOG_TAG, __func__, __LINE__, format, buf, len, ##__VA_ARGS__)
#define DUIKIT_LOG_HEX_E(format, buf, len, ...) DUIKIT_LOG_HEX_DUMP_E(DUIKIT_LOG_TAG, __func__, __LINE__, format, buf, len, ##__VA_ARGS__)

#if (DUIKIT_LOG_LVL > DUIKIT_LOG_LEVEL_DEBUG) || (DUIKIT_LOG_OUTPUT_LEVEL > DUIKIT_LOG_LEVEL_DEBUG)                
    #define DUIKIT_LOG_DEBUG(TAG, func, line, format, ...)
#else                                                                           
    #define DUIKIT_LOG_DEBUG(TAG, func, line, format, ...) duikit_log_output(DUIKIT_LOG_LEVEL_DEBUG, TAG, func, line, format, ##__VA_ARGS__)
#endif

#if (DUIKIT_LOG_LVL > DUIKIT_LOG_LEVEL_INFO) || (DUIKIT_LOG_OUTPUT_LEVEL > DUIKIT_LOG_LEVEL_INFO)                
    #define DUIKIT_LOG_INFO(TAG, func, line, format, ...)
#else                                                                           
    #define DUIKIT_LOG_INFO(TAG, func, line, format, ...) duikit_log_output(DUIKIT_LOG_LEVEL_INFO, TAG, func, line, format, ##__VA_ARGS__)
#endif

#if (DUIKIT_LOG_LVL > DUIKIT_LOG_LEVEL_WARNING) || (DUIKIT_LOG_OUTPUT_LEVEL > DUIKIT_LOG_LEVEL_WARNING)                
    #define DUIKIT_LOG_WARN(TAG, func, line, format, ...)
#else                                                                           
    #define DUIKIT_LOG_WARN(TAG, func, line, format, ...) duikit_log_output(DUIKIT_LOG_LEVEL_WARNING, TAG, func, line, format, ##__VA_ARGS__)
#endif

#if (DUIKIT_LOG_LVL > DUIKIT_LOG_LEVEL_ERROR) || (DUIKIT_LOG_OUTPUT_LEVEL > DUIKIT_LOG_LEVEL_ERROR)                
    #define DUIKIT_LOG_ERROR(TAG, func, line, format, ...)
#else                                                                           
    #define DUIKIT_LOG_ERROR(TAG, func, line, format, ...) duikit_log_output(DUIKIT_LOG_LEVEL_ERROR, TAG, func, line, format, ##__VA_ARGS__)
#endif
                                                                                   
#if (DUIKIT_LOG_LVL > DUIKIT_LOG_LEVEL_DEBUG) || (DUIKIT_LOG_OUTPUT_LEVEL > DUIKIT_LOG_LEVEL_DEBUG)                
    #define DUIKIT_LOG_HEX_DUMP_D(TAG, func, line, format, buf, len, ...)
#else                                                                           
    #define DUIKIT_LOG_HEX_DUMP_D(TAG, func, line, format, buf, len, ...) duikit_log_hexdump(DUIKIT_LOG_LEVEL_DEBUG, TAG, func, line, buf, len, format, ##__VA_ARGS__)
#endif

#if (DUIKIT_LOG_LVL > DUIKIT_LOG_LEVEL_INFO) || (DUIKIT_LOG_OUTPUT_LEVEL > DUIKIT_LOG_LEVEL_INFO)                
    #define DUIKIT_LOG_HEX_DUMP_I(TAG, func, line, format, buf, len, ...)
#else                                                                           
    #define DUIKIT_LOG_HEX_DUMP_I(TAG, func, line, format, buf, len, ...) duikit_log_hexdump(DUIKIT_LOG_LEVEL_INFO, TAG, func, line, buf, len, format, ##__VA_ARGS__)
#endif

#if (DUIKIT_LOG_LVL > DUIKIT_LOG_LEVEL_WARNING) || (DUIKIT_LOG_OUTPUT_LEVEL > DUIKIT_LOG_LEVEL_WARNING)                
    #define DUIKIT_LOG_HEX_DUMP_W(TAG, func, line, format, buf, len, ...)
#else                                                                           
    #define DUIKIT_LOG_HEX_DUMP_W(TAG, func, line, format, buf, len, ...) duikit_log_hexdump(DUIKIT_LOG_LEVEL_WARNING, TAG, func, line, buf, len, format, ##__VA_ARGS__)
#endif

#if (DUIKIT_LOG_LVL > DUIKIT_LOG_LEVEL_ERROR) || (DUIKIT_LOG_OUTPUT_LEVEL > DUIKIT_LOG_LEVEL_ERROR)                
    #define DUIKIT_LOG_HEX_DUMP_E(TAG, func, line, format, buf, len, ...)
#else                                                                           
    #define DUIKIT_LOG_HEX_DUMP_E(TAG, func, line, format, buf, len, ...) duikit_log_hexdump(DUIKIT_LOG_LEVEL_ERROR, TAG, func, line, buf, len, format, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
