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

#ifndef _TC_IOT_LOG_H_
#define _TC_IOT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_libs_inc.h"

/**
 * SDK log print/upload level
 */
typedef enum { eLOG_DISABLE = 0, eLOG_ERROR = 1, eLOG_WARN = 2, eLOG_INFO = 3, eLOG_DEBUG = 4 } TC_LOG_LEVEL;

/**
 * log print level control, only print logs with level less or equal to this
 * variable
 */
extern TC_LOG_LEVEL g_log_print_level;


/* user's self defined log handler callback */
typedef bool (*LogMessageHandler)(const char *message);

/* user's self defined log handler callback for */
typedef void (*LogExtraHandler)(TC_LOG_LEVEL level, const char *msg, size_t len);


/**
 * @brief Set the global log level of print
 *
 * @param level
 */
void IOT_Log_Set_Level(TC_LOG_LEVEL level);

/**
 * @brief Get the global log level of print
 *
 * @return
 */
TC_LOG_LEVEL IOT_Log_Get_Level();


/**
 * @brief Set user callback to print log into wherever you want
 *
 * @param handler function pointer of callback
 *
 */
void IOT_Log_Set_MessageHandler(LogMessageHandler handler);

/**
 * @brief Set user callback to handle log before print, such as upload
 *
 * @param handler function pointer of callback
 *
 */
void IOT_Log_Set_ExtraHandler(LogExtraHandler handler);


/**
 * @brief Generate log for print/upload, call LogMessageHandler if defined
 *
 * When LOG_UPLOAD is enabled, the log will be uploaded to cloud server
 *
 * @param file
 * @param func
 * @param line
 * @param level
 */
void IOT_Log_Gen(const char *file, const char *func, const int line, const int level, const char *fmt, ...);

void Log_dump(void *array, size_t array_len);

/* Simple APIs for log generation in different level */
#define Log_d(fmt, ...) IOT_Log_Gen(__FILE__, __FUNCTION__, __LINE__, eLOG_DEBUG, fmt, ##__VA_ARGS__)
#define Log_i(fmt, ...) IOT_Log_Gen(__FILE__, __FUNCTION__, __LINE__, eLOG_INFO, fmt, ##__VA_ARGS__)
#define Log_w(fmt, ...) IOT_Log_Gen(__FILE__, __FUNCTION__, __LINE__, eLOG_WARN, fmt, ##__VA_ARGS__)
#define Log_e(fmt, ...) IOT_Log_Gen(__FILE__, __FUNCTION__, __LINE__, eLOG_ERROR, fmt, ##__VA_ARGS__)
#define Log_dump(array, array_len) Log_dump(array, array_len)

#ifdef __cplusplus
}
#endif

#endif /* _TC_IOT_LOG_H_ */
