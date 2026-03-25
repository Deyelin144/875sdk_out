/*****************************************************************************
 * Copyright (C) 2022 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the MIT License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://opensource.org/licenses/MIT
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "tc_iot_hal.h"
#include "tc_iot_log.h"

// 默认使能日志着色
#define LOG_COLOR_ENABLED

#ifndef MAX_LOG_MSG_LEN
#define MAX_LOG_MSG_LEN (1024)
#endif

#ifdef LOG_COLOR_ENABLED
static char *level_str[] = {
    "\033[34mDIS",  // 蓝色
    "\033[31mERR",  // 红色
    "\033[33mWRN",  // 黄色
    "\033[32mINF",  // 绿色
    "\033[0mDBG"    // 默认颜色
};
#else
static char *level_str[] = {"DIS", "ERR", "WRN", "INF", "DBG"};
#endif

static LogMessageHandler sg_log_message_handler = NULL;
static LogExtraHandler sg_log_extra_handler     = NULL;
TC_LOG_LEVEL g_log_print_level                     = eLOG_DEBUG;

static const char *_get_filename(const char *p)
{

    char ch = '/';

    const char *q = strrchr(p, ch);
    if (q == NULL) {
        q = p;
    } else {
        q++;
    }
    return q;
}

void IOT_Log_Set_Level(TC_LOG_LEVEL logLevel)
{
    g_log_print_level = logLevel;
}

TC_LOG_LEVEL IOT_Log_Get_Level(void)
{
    return g_log_print_level;
}

void IOT_Log_Set_MessageHandler(LogMessageHandler handler)
{
    sg_log_message_handler = handler;
}

void IOT_Log_Set_ExtraHandler(LogExtraHandler handler)
{
    sg_log_extra_handler = handler;
}

void IOT_Log_Gen(const char *file, const char *func, const int line, const int level, const char *fmt, ...)
{
    if (level > g_log_print_level) {
        return;
    }

    /* format log content */
    const char *file_name = _get_filename(file);

    char sg_text_buf[MAX_LOG_MSG_LEN + 1] = {0};
    char *tmp_buf                         = sg_text_buf;
    char *o                               = tmp_buf;

    memset(sg_text_buf, 0, sizeof(sg_text_buf));
    
    char time_str[TIME_FORMAT_STR_LEN] = {0};
    HAL_GetLocalTime(time_str, sizeof(time_str));

    o += HAL_Snprintf(o, sizeof(sg_text_buf), "%s|TCIV|%s|%s|%s(%d): ", level_str[level], time_str,
                      STR_SAFE_PRINT(file_name), STR_SAFE_PRINT(func), line);

    va_list ap;
    va_start(ap, fmt);
    HAL_Vsnprintf(o, MAX_LOG_MSG_LEN - 2 - strlen(tmp_buf), fmt, ap);
    va_end(ap);

#ifndef PLAT_USE_THREADX
    strcat(tmp_buf, "\n");
#endif

#ifdef LOG_COLOR_ENABLED
    strcat((char *)tmp_buf, "\033[0m");
#endif

    /* extra log handle such as log upload */
    if (sg_log_extra_handler) {
        sg_log_extra_handler((TC_LOG_LEVEL)level, tmp_buf, strlen(tmp_buf));
    }

    if (level <= g_log_print_level) {
        /* customer defined log print handler */
        if (sg_log_message_handler != NULL && sg_log_message_handler(tmp_buf)) {
            return;
        }

        /* default log handler: print to console */
        HAL_Printf("%s", tmp_buf);
    }

    return;
}

#define HEX_DUMP_BYTE_PER_LINE 16

static void _hex_dump(const void *pdata, int len)
{
    int i, j, k, l;
    const char *data = (const char *)pdata;
    char buf[256], str[64], t[] = "0123456789ABCDEF";
    if (len <= 0) {
        return;
    }
    for (i = j = k = 0; i < len; i++) {
        if (0 == i % HEX_DUMP_BYTE_PER_LINE) {
            j += HAL_Snprintf(buf + j, sizeof(buf) - j, "%04xh: ", i);
        }
        buf[j++] = t[0x0f & (data[i] >> 4)];
        buf[j++] = t[0x0f & data[i]];
        buf[j++] = ' ';
        str[k++] = (data[i] >= ' ' && data[i] <= '~') ? data[i] : '.';
        if (0 == (i + 1) % HEX_DUMP_BYTE_PER_LINE) {
            str[k] = 0;
            j += HAL_Snprintf(buf + j, sizeof(buf) - j, "| %s\n", str);
            HAL_Printf("%s", buf);
            j = k = buf[0] = str[0] = 0;
        }
    }
    str[k] = 0;
    if (k) {
        for (l = 0; l < 3 * (HEX_DUMP_BYTE_PER_LINE - k); l++) {
            buf[j++] = ' ';
        }
        j += HAL_Snprintf(buf + j, sizeof(buf) - j, "| %s\n", str);
    }
    if (buf[0]) {
        HAL_Printf("%s", buf);
    }

    HAL_Printf("\r\n");
}

/**
 * @brief Print hex dump of array.
 *
 * @param[in] array pointer to array
 * @param[in] array_len array length
 */
void Log_dump(void *array, size_t array_len)
{
    if (array && array_len) {
        HAL_Printf("offset: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\r\n");
        HAL_Printf("======================================================\r\n");
        _hex_dump(array, array_len);
    }
}
