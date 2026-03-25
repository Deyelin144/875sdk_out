
#ifndef __REALIZE_UNIT_PERF_TEST_H__
#define __REALIZE_UNIT_PERF_TEST_H__

#include "../../platform/gulitesf_config.h"

/*
各模块包含该头文件，请在.c文件使用下列形式
#undef MODULE_PTEST
// #define MODULE_PTEST //默认不定义，需要单独打开该模块开关时定义
#include "xxx.../realize_unit_perf_test.h"
*/

#ifdef CONFIG_USE_PERF_TEST
#include "../../platform/wrapper/wrapper.h"

void realize_unit_perf_test_setdbg(char debug);
long realize_unit_perf_test_get_start_time(void);
int realize_unit_perf_test_start(char *save_dir, char *test_name, int cache_len, int identify_num, const char *func);
int realize_unit_perf_test_flush_buffer(void);
void realize_unit_perf_test_end(const char *func);
int realize_unit_perf_test_entry(char *identify, long data);
int realize_unit_perf_test_exit(char *identify, long data);
void realize_unit_perf_test_separate(void);

#define PTEST_LOG_ERR(fmt, ...) realize_unit_log_error(fmt, ##__VA_ARGS__)
#define PTEST_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define PTEST_GET_TIME_MS wrapper_get_handle()->os.get_time_ms()
// 距离start_time的时间
#define PTEST_GET_TIME_MS_INNER ((int)(PTEST_GET_TIME_MS - realize_unit_perf_test_get_start_time()))

/*
当前在module logic层添加了测试打点函数，
若不定义该宏，会在引擎初始化内存、文件之后开始运行测试代码，直接在串口打印测试数据
若定义该宏，则认为是要跑自己的测试用例，需要自己按照下面步骤调用测试开始和结束代码
*/
// #define PTEST_USE_CUS_TEST

// 尽量使用宏调用
/*
    1、首先调用 PTEST_START，测试代码才会运行
    2、在要测试的接口的入口处添加 PTEST_FUNC_ENTRY，在要测试的接口的出口处添加 PTEST_FUNC_EXIT
    3、PTEST_FUNC_ENTRY与PTEST_FUNC_EXIT在同一个函数内只使用一次
    4、在同一函数内，使用自定义的标识，可调用PTEST_FUNC_CUS_ENTRY(x)、PTEST_FUNC_CUS_EXIT(x)
    5、在想要测试性能的地方（可能是多个函数的组合、异步等待时间等）开始处添加 PTEST_ENTRY(x)，
       x为唯一标识符，通过该标识与 PTEST_EXIT(x) 匹配
    6、在想要测试性能的地方（可能是多个函数的组合、异步等待时间等）结束处添加 PTEST_EXIT(x)，
       x为唯一标识符，通过该标识与 PTEST_ENTRY(x) 匹配
    7、PTEST_ENTRY(x)与 PTEST_EXIT(x)可以在不同的函数、线程中使用，标识符相同时不可以嵌套使用，不同时可以嵌套使用
        {
            PTEST_ENTRY(x);
            PTEST_ENTRY(x);
            PTEST_EXIT(x);
            PTEST_EXIT(x);
        } // 不允许

        {
            PTEST_ENTRY(x);
            PTEST_ENTRY(y);
            PTEST_EXIT(y);
            PTEST_EXIT(x);
        } // 允许
    8、PTEST_START到PTEST_END之间的测试日志会保存在文件内。若想将多个测试用例保存在一个文件，为避免缓冲区满影响测试结果，可以在每个测试用
        例之后调用PTEST_FLUSH刷新缓冲区
    9、可以使用 PTEST_SEPARATE 分隔打印日志，这样经过脚本处理的图表遇到分隔符时会空一行
*/

// 若需要打开所有模块的性能测试（log较多），请在此定义这个宏。否则需要到各模块包含该头文件之前定义（只关注自己想要测试的模块）
// #define MODULE_PTEST
#ifdef MODULE_PTEST
// 在同一函数使用，用函数名做identify
#define PTEST_FUNC_ENTRY \
    long perf_test_func_entry_time = PTEST_GET_TIME_MS; \
    realize_unit_perf_test_entry((char *)__func__, perf_test_func_entry_time)
#define PTEST_FUNC_EXIT realize_unit_perf_test_exit((char *)__func__, perf_test_func_entry_time)

// 在同一函数使用，自定义identify，使用栈空间，调用时identify不加双引号！！！！！！！！！
#define PTEST_FUNC_CUS_ENTRY(identify) \
    long ptest_func_cus_##identify = PTEST_GET_TIME_MS; \
    realize_unit_perf_test_entry(#identify, ptest_func_cus_##identify)
#define PTEST_FUNC_CUS_EXIT(identify) realize_unit_perf_test_exit((char *)#identify, ptest_func_cus_##identify)

// 可在不同线程中使用，identify存到哈希表，调用时identify加双引号！！！！！！！！！！！！
#define PTEST_ENTRY(identify) realize_unit_perf_test_entry(identify, -1)
#define PTEST_EXIT(identify) realize_unit_perf_test_exit(identify, -1)
#define PTEST_SEPARATE realize_unit_perf_test_separate()
#else
#define PTEST_FUNC_ENTRY
#define PTEST_FUNC_EXIT
#define PTEST_FUNC_CUS_ENTRY(identify)
#define PTEST_FUNC_CUS_EXIT(identify)
#define PTEST_ENTRY(identify)
#define PTEST_EXIT(identify)
#define PTEST_SEPARATE
#endif

#define PTEST_START(save_dir, test_name, cache_len, identify_num) \
    realize_unit_perf_test_start(save_dir, test_name, cache_len, identify_num, __func__)
#define PTEST_END realize_unit_perf_test_end(__func__)

#define PTEST_SET_DBG(dbg) realize_unit_perf_test_setdbg(dbg)
#define PTEST_FLUSH \
    do { \
        long start = PTEST_GET_TIME_MS; \
        realize_unit_perf_test_entry("realize_unit_perf_test_flush_buffer", start); \
        realize_unit_perf_test_flush_buffer(); \
        realize_unit_perf_test_exit("realize_unit_perf_test_flush_buffer", start); \
    } while (0)

#else // CONFIG_USE_PERF_TEST

#define PTEST_FUNC_ENTRY
#define PTEST_FUNC_EXIT
#define PTEST_FUNC_CUS_ENTRY(identify)
#define PTEST_FUNC_CUS_EXIT(identify)
#define PTEST_ENTRY(identify)
#define PTEST_EXIT(identify)
#define PTEST_SEPARATE
#define PTEST_START(save_dir, test_name, cache_len, identify_num)
#define PTEST_END
#define PTEST_SET_DBG(identify)
#define PTEST_FLUSH

#endif // CONFIG_USE_PERF_TEST

#endif