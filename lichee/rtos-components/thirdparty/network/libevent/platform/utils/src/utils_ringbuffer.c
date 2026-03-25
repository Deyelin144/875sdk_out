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

#include "tc_iot_utils_ringbuffer.h"
#include "tc_iot_hal.h"
#include "tc_iot_log.h"

// ---------------- private typedef begin ----------------

typedef struct _rb_handle{
    uint32_t head; // 偏移量，已使用空间的第一字节
    uint32_t tail; // 偏移量，未使用空间的第一字节
    uint32_t total_size;
    uint32_t used_size;
    void *lock;
    uint8_t buf[1];
} rb_handle;

// ---------------- private typedef end ----------------


int tc_utils_ringbuffer_init(void **handle, uint32_t size)
{
    if (handle == NULL || size == 0) {
        Log_e("invalid params");
        return -1;
    }

    uint32_t total_size = offsetof(struct _rb_handle, buf) + sizeof(uint8_t) * (size);
    rb_handle *p_rb_handle = HAL_Malloc(total_size);
    if (!p_rb_handle) {
        Log_e("malloc buffer failed");
        goto err;
    }
    memset(p_rb_handle, 0, total_size);

    p_rb_handle->total_size = size;
    p_rb_handle->lock = HAL_MutexCreate();
    if (!p_rb_handle->lock) {
        Log_e("malloc lock failed");
        goto err;
    }

    *handle = (void *)p_rb_handle;

    return 0;

err:
    *handle = NULL;
    tc_utils_ringbuffer_exit((void *)&p_rb_handle);
    return -1;
}

int tc_utils_ringbuffer_exit(void **handle)
{
    rb_handle *p_rb_handle = (rb_handle *)*handle;

    if (p_rb_handle) {
        if (p_rb_handle->lock) {
            HAL_MutexDestroy(p_rb_handle->lock);
            p_rb_handle->lock = NULL;
        }

        memset(p_rb_handle, 0, sizeof(rb_handle));
        HAL_Free(p_rb_handle);
        *handle = NULL;
    }

    return 0;
}

int tc_utils_ringbuffer_reset(void *handle)
{
    rb_handle *p_rb_handle = (rb_handle *)handle;

    if (p_rb_handle) {
        HAL_MutexLock(p_rb_handle->lock);
        p_rb_handle->head = 0;
        p_rb_handle->tail = 0;
        p_rb_handle->used_size = 0;
        HAL_MutexUnlock(p_rb_handle->lock);
        return 0;
    }
    else {
        Log_e("invalid handle");
        return -1;
    }
}

int tc_utils_ringbuffer_write(void *handle, const uint8_t *buf, uint32_t len)
{
    rb_handle *p_rb_handle = (rb_handle *)handle;

    if (!p_rb_handle || !buf || !len) {
        Log_e("invalid params");
        return -1;
    }

    if (p_rb_handle->lock) {
        HAL_MutexLock(p_rb_handle->lock);
    }

    int ret = 0;
    if (len <= tc_utils_ringbuffer_get_free_size(p_rb_handle)) {
        if (p_rb_handle->tail + len < p_rb_handle->total_size) {
            memcpy(&p_rb_handle->buf[p_rb_handle->tail], buf, len);
            p_rb_handle->tail += len;
        } else if (p_rb_handle->tail + len > p_rb_handle->total_size) {
            uint32_t count = p_rb_handle->total_size - p_rb_handle->tail;
            memcpy(&p_rb_handle->buf[p_rb_handle->tail], buf, count);
            memcpy(&p_rb_handle->buf[0], &buf[count], len - count);
            p_rb_handle->tail = len - count;
        } else {
            memcpy(&p_rb_handle->buf[p_rb_handle->tail], buf, len);
            p_rb_handle->tail = 0;
        }
        p_rb_handle->used_size += len;
    }
    else {
        Log_e("ring buffer overflow");
        ret = -1;
    }

    if (p_rb_handle->lock) {
        HAL_MutexUnlock(p_rb_handle->lock);
    }

    return ret;
}

int tc_utils_ringbuffer_read(void *handle, uint8_t *buf, uint32_t len)
{
    rb_handle *p_rb_handle = (rb_handle *)handle;

    if (!p_rb_handle || !buf || !len) {
        Log_e("invalid params");
        return -1;
    }

    if (p_rb_handle->lock) {
        HAL_MutexLock(p_rb_handle->lock);
    }

    int ret = 0;
    if (len <= tc_utils_ringbuffer_get_used_size(p_rb_handle)) {
        if (p_rb_handle->head + len < p_rb_handle->total_size) {
            memcpy(buf, &p_rb_handle->buf[p_rb_handle->head], len);
            p_rb_handle->head += len;
        } else if (p_rb_handle->head + len > p_rb_handle->total_size) {
            int32_t count = p_rb_handle->total_size - p_rb_handle->head;
            memcpy(buf, &p_rb_handle->buf[p_rb_handle->head], count);
            memcpy(&buf[count], &p_rb_handle->buf[0], len - count);
            p_rb_handle->head = len - count;
        } else {
            memcpy(buf, &p_rb_handle->buf[p_rb_handle->head], len);
            p_rb_handle->head = 0;
        }
        p_rb_handle->used_size -= len;
    }
    else {
        // Log_e("ring buffer underflow");
        ret = -1;
    }

    if (p_rb_handle->lock) {
        HAL_MutexUnlock(p_rb_handle->lock);
    }

    return ret;
}

inline int32_t tc_utils_ringbuffer_get_free_size(void *handle)
{
    rb_handle *p_rb_handle = (rb_handle *)handle;

    return p_rb_handle ? (p_rb_handle->total_size - p_rb_handle->used_size) : 0;
}

inline int32_t tc_utils_ringbuffer_get_used_size(void *handle)
{
    rb_handle *p_rb_handle = (rb_handle *)handle;

    return p_rb_handle ? p_rb_handle->used_size : 0;
}


inline int32_t tc_utils_ringbuffer_is_empty(void *handle)
{
    rb_handle *p_rb_handle = (rb_handle *)handle;

    return p_rb_handle ? (p_rb_handle->used_size == 0) : 0;
}

inline int32_t tc_utils_ringbuffer_is_full(void *handle)
{
    rb_handle *p_rb_handle = (rb_handle *)handle;

    return p_rb_handle ? (p_rb_handle->used_size == p_rb_handle->total_size) : 0;
}
