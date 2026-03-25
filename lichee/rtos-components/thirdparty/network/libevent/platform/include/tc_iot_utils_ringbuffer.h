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

#ifndef _TC_IOT_UTILS_RINGBUFFER_H_
#define _TC_IOT_UTILS_RINGBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/**
 * @brief 初始化环形队列
 * 
 * @param handle 环形队列句柄
 * @param size 环形队列大小
 * @return int 返回值
 */
int tc_utils_ringbuffer_init(void **handle, uint32_t size);

/**
 * @brief 销毁环形队列
 * 
 * @param handle 环形队列句柄
 * @return int 返回值
 */
int tc_utils_ringbuffer_exit(void **handle);

/**
 * @brief 重置环形队列
 * 
 * @param handle 环形队列句柄
 * @return int 返回值
 */
int tc_utils_ringbuffer_reset(void *handle);

/**
 * @brief 写入环形队列，如果len大于环形队列剩余空间则写入失败
 * 
 * @param handle 环形队列句柄
 * @param buf 写入的数据
 * @param len 写入的长度
 * @return int 返回值
 */
int tc_utils_ringbuffer_write(void *handle, const uint8_t *buf, uint32_t len);

/**
 * @brief 有内存复制读，如果len大于环形队列已用空间则读取失败
 * 
 * @param handle 环形队列句柄
 * @param buf 读取的数据
 * @param len 读取的长度
 * @return int 返回值
 */
int tc_utils_ringbuffer_read(void *handle, uint8_t *buf, uint32_t len);

/**
 * @brief 获取环形队列可用空间
 * 
 * @param handle 环形队列句柄
 * @return int32_t 可用空间
 */
int32_t tc_utils_ringbuffer_get_free_size(void *handle);

int32_t tc_utils_ringbuffer_get_used_size(void *handle);

/**
 * @brief 环形队列是否为空
 * 
 * @param handle 环形队列句柄
 * @return int32_t 1为空，0为非空
 */
int32_t tc_utils_ringbuffer_is_empty(void *handle);

/**
 * @brief 环形队列是否为满
 * 
 * @param handle 环形队列句柄
 * @return int32_t 1为满，0为非满
 */
int32_t tc_utils_ringbuffer_is_full(void *handle);

#ifdef __cplusplus
}
#endif

#endif
