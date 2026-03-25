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
#include "tc_iot_platform_inc.h"
#include "tc_iot_ret_code.h"
#include <pthread.h>

#ifdef PLATFORM_ESP_RTOS
#include "esp_heap_caps.h"
#include "esp_random.h"
#include "esp_mac.h"
typedef struct {
    StackType_t  *taskStack;
    StaticTask_t *taskTCB;
} FreeRTOSStackInfo;

#endif

#ifndef MS_TO_TICKS
#define MS_TO_TICKS(n) ((n) / portTICK_PERIOD_MS ? (n) / portTICK_PERIOD_MS : 1)
#endif

void HAL_SleepMs(uint32_t ms)
{
#ifdef JIELI_RTOS
    mdelay(ms);
#else
    vTaskDelay(MS_TO_TICKS(ms)); /* Minimum delay = 1 tick */
#endif

    return;
}

void *HAL_Malloc(uint32_t size)
{
    void *ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
        return ptr;
    }
    return NULL;
}

void *HAL_Realloc(void *ptr, uint32_t size)
{
    return realloc(ptr, size);
}

void HAL_Free(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
    ptr = NULL;
}

void HAL_Printf(const char *fmt, ...)
{
    char    buf[512] = {0};
    va_list args;
    int     len = 0;

    va_start(args, fmt);
    len = vsnprintf(buf, 511, fmt, args);
    va_end(args);

    printf("%s\n", buf);
}

int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

void HAL_GetMAC(uint8_t *mac, uint8_t len)
{
    // !获取MAC地址
#ifdef PLATFORM_ESP_RTOS
    esp_err_t ret = esp_read_mac(mac, ESP_MAC_BASE);
    if (ret == ESP_OK) {
        Log_d("MAC address: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        Log_e("Failed to get MAC address");
    }
#else
    Log_e("HAL_GetMAC not implement!!!");
    return;
#endif
}

uint32_t HAL_GetMemSize(void)
{
    // !获取剩余内存大小
#ifdef PLATFORM_ESP_RTOS
    return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
#else
    //Log_e("HAL_GetMemSize not implement!!!");
    uint32_t freesize = 0;
    freesize = xPortGetFreeHeapSize();
    return freesize;
#endif
}

long HAL_Random(void)
{
    // !获取随机数
#ifdef PLATFORM_ESP_RTOS
    return esp_random();
#else
    //Log_e("HAL_Random not implemented!!!");

    return (long)rand();
#endif
}

char *HAL_GetPlatform(void)
{
    return "freertos";
}

void HAL_Signal(int sginum, void (*handler)(int))
{
    return;
}

// platform-dependant thread routine/entry function
static void _HAL_thread_func_wrapper_(void *ptr)
{
    ThreadParams *params = (ThreadParams *)ptr;

    params->thread_func(params->user_arg);

#ifndef JIELI_RTOS
    vTaskDelete(NULL);

#ifdef PLATFORM_ESP_RTOS
    FreeRTOSStackInfo *stack_info = params->stack_ptr;
    HAL_Free(stack_info->taskStack);
    HAL_Free(stack_info->taskTCB);
    HAL_Free(stack_info);
#endif  // PLATFORM_ESP_RTOS

#endif  // JIELI_RTOS
}

// platform-dependant thread create function
int HAL_ThreadCreate(ThreadParams *params)
{
    if (params == NULL) {
        Log_e("null ThreadParams!");
        return QCLOUD_ERR_INVAL;
    }

    if (params->thread_name == NULL || params->thread_func == NULL || params->stack_size == 0) {
        Log_e("thread params (%p %p %u) invalid", params->thread_name, params->thread_func, params->stack_size);
        return QCLOUD_ERR_INVAL;
    }

    // the specific thread priority is platform dependent
    int priority = 1;

    switch (params->priority) {
        case THREAD_PRIORITY_HIGHEST:
            priority = configMAX_PRIORITIES / 3 + 2;
            break;
        case THREAD_PRIORITY_HIGHER:
            priority = configMAX_PRIORITIES / 3 + 1;
            break;
        case THREAD_PRIORITY_HIGH:
            priority = configMAX_PRIORITIES / 3;
            break;
        case THREAD_PRIORITY_NORMAL:
            priority = configMAX_PRIORITIES / 3 - 1;
            break;
        case THREAD_PRIORITY_LOW:
            priority = configMAX_PRIORITIES / 3 - 2;
            break;
        default:
            priority = params->priority;
            break;
    }

#ifdef JIELI_RTOS
    priority = params->priority + 26;
#endif

#ifdef JIELI_RTOS
    int ret = thread_fork(params->thread_name, priority, params->stack_size, 1024, (int *)&params->thread_id,
                          _HAL_thread_func_wrapper_, params);
    if (ret) {
        Log_e("thread_fork failed: %d", ret);
        return QCLOUD_ERR_FAILURE;
    }
#else

#ifdef PLATFORM_ESP_RTOS
    // ESP32 都使用外部RAM CPU1来创建任务
    params->stack_ptr             = (FreeRTOSStackInfo *)HAL_Malloc(sizeof(FreeRTOSStackInfo));
    FreeRTOSStackInfo *stack_info = (FreeRTOSStackInfo *)params->stack_ptr;
    if (!stack_info) {
        Log_e("Error stack info malloc fail. create %s stack fail.\n", params->thread_name);
        return QCLOUD_ERR_FAILURE;
    }

    stack_info->taskStack = (StackType_t *)HAL_Malloc(params->stack_size);

    if (stack_info->taskStack) {
        memset(stack_info->taskStack, 0, params->stack_size);
    } else {
        Log_e("Error malloc %s task stack %d bytes.\n", params->thread_name, params->stack_size);
        return QCLOUD_ERR_FAILURE;
    }

    stack_info->taskTCB = heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);  // 内部sram
    if (!stack_info->taskTCB) {
        Log_e("%s task. HAL_Malloc taskTCB failed\n", params->thread_name);
        HAL_Free(stack_info->taskStack);
        HAL_Free(stack_info);
        return QCLOUD_ERR_FAILURE;
    }

    memset(stack_info->taskTCB, 0, sizeof(StaticTask_t));
    params->thread_id = xTaskCreateStaticPinnedToCore(_HAL_thread_func_wrapper_, params->thread_name,
                                                      params->stack_size, (void *)params, priority | portPRIVILEGE_BIT,
                                                      stack_info->taskStack, stack_info->taskTCB, 1);
    if (params->thread_id == NULL) {
        Log_e("Error creating xTaskCreateStaticPinnedToCore %s\n", params->thread_name);
        HAL_Free(stack_info->taskStack);
        HAL_Free(stack_info->taskTCB);
        HAL_Free(stack_info);
        return QCLOUD_ERR_FAILURE;
    }

#else
	if (!strcmp(params->thread_name, "p2p_main_thread")) {
		params->stack_size = 1024 * 20;  // 2K
		priority = 22;
	}
    int ret = xTaskCreate(_HAL_thread_func_wrapper_, params->thread_name,
                          params->stack_size / sizeof(StackType_t),  // byte --> world
                          (void *)params, priority, (void *)&params->thread_id);
    if (ret != pdPASS) {
        Log_e("xTaskCreate failed: %d", ret);
        return QCLOUD_ERR_FAILURE;
    }
#endif  // PLATFORM_ESP_RTOS

#endif  // JIELI_RTOS

    Log_i("created task %s %p,pri:%d stack size : %d Kbytes", params->thread_name, params->thread_id, priority,
          ((params->stack_size) >> 10));
    return QCLOUD_RET_SUCCESS;
}

int HAL_ThreadDestroy(ThreadHandle_t *thread_handle)
{
    if (NULL == thread_handle) {
        return QCLOUD_ERR_FAILURE;
    }

#ifdef JIELI_RTOS
    // thread_kill(thread_handle, KILL_FORCE);
#else
    vTaskDelete((TaskHandle_t )thread_handle);
#endif

    ThreadParams *params = container_of(thread_handle, ThreadParams, thread_id);
    if (params) {
        Log_i("destroy task %s %p %p", params->thread_name, params, params->thread_id);
    }
    return 0;
}

unsigned long HAL_GetCurrentThreadHandle(void)
{
    unsigned long handle = (unsigned long)xTaskGetCurrentTaskHandle();
    return handle;
}

void *HAL_MutexCreate(void)
{
    int ret;
    pthread_mutex_t *mutex = (pthread_mutex_t *)HAL_Malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex) {
        Log_e("malloc mutex failed");
        return NULL;
    }

    ret = pthread_mutex_init(mutex, NULL);
    if (ret) {
        Log_e("pthread_mutex_init failed %d", ret);
        HAL_Free(mutex);
        return NULL;
    }

    return mutex;
}

void HAL_MutexDestroy(void *mutex)
{
    if (!mutex) {
        Log_e("invalid mutex");
        return;
    }

    int ret = pthread_mutex_destroy((pthread_mutex_t *)mutex);
    if (ret) {
        Log_e("pthread_mutex_destroy failed %d", ret);
    }

    HAL_Free(mutex);
}

int HAL_MutexLock(void *mutex)
{
    if (!mutex) {
        Log_e("invalid mutex");
        return -1;
    }

    int ret = pthread_mutex_lock((pthread_mutex_t *)mutex);
    if (ret) {
        Log_e("pthread_mutex_lock failed %d", ret);
        return -1;
    }

    return 0;
}

int HAL_MutexTryLock(void *mutex)
{
    if (!mutex) {
        Log_e("invalid mutex");
        return -1;
    }

    int ret = pthread_mutex_trylock((pthread_mutex_t *)mutex);
    if (ret) {
        Log_e("pthread_mutex_trylock failed %d", ret);
        return -1;
    }
    return 0;
}

int HAL_MutexUnlock(void *mutex)
{
    if (!mutex) {
        Log_e("invalid mutex");
        return -1;
    }

    int ret = pthread_mutex_unlock((pthread_mutex_t *)mutex);
    if (ret) {
        Log_e("pthread_mutex_trylock failed %d", ret);
        return -1;
    }

    return 0;
}

void *HAL_RecursiveMutexCreate(void)
{
    static pthread_mutexattr_t attr_recursive;
    /* Set ourselves up to get recursive locks. */
    if (pthread_mutexattr_init(&attr_recursive))
        return NULL;
    if (pthread_mutexattr_settype(&attr_recursive, PTHREAD_MUTEX_RECURSIVE))
        return NULL;

    pthread_mutex_t *mutex = HAL_Malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex) {
        Log_e("malloc mutex failed");
        return NULL;
    }

    int ret = pthread_mutex_init(mutex, &attr_recursive);
    if (ret) {
        Log_e("pthread_mutex_init failed %d", ret);
        HAL_Free(mutex);
        return NULL;
    }

    return mutex;
}

void HAL_RecursiveMutexDestroy(void *mutex)
{
    return HAL_MutexDestroy(mutex);
}

int HAL_RecursiveMutexLock(void *mutex, int try_flag)
{
    if (!mutex) {
        Log_e("invalid mutex");
        return -1;
    }

    int ret = 0;
    if (try_flag)
        ret = pthread_mutex_trylock((pthread_mutex_t *)mutex);
    else
        ret = pthread_mutex_lock((pthread_mutex_t *)mutex);

    if (ret) {
        Log_e("pthread_mutex_lock failed %d", ret);
        return -1;
    }

    return 0;
}

int HAL_RecursiveMutexUnLock(void *mutex)
{
    if (!mutex) {
        Log_e("invalid mutex");
        return -1;
    }

    int ret = pthread_mutex_unlock((pthread_mutex_t *)mutex);
    if (ret) {
        Log_e("pthread_mutex_unlock failed %d", ret);
        return -1;
    }

    return 0;
}


void *HAL_CondCreate(void)
{
    pthread_cond_t *cond = HAL_Malloc(sizeof(pthread_cond_t));
    if (!cond) {
        Log_e("malloc cond failed");
        return NULL;
    }

    int ret = pthread_cond_init(cond, NULL);
    if (ret) {
        Log_e("pthread_cond_init failed %d", ret);
        HAL_Free(cond);
        return NULL;
    }
    return cond;
}

void HAL_CondFree(void *cond_)
{
    if (!cond_) {
        Log_e("invalid cond");
        return;
    }

    int ret = pthread_cond_destroy((pthread_cond_t *)cond_);
    if (ret) {
        Log_e("pthread_cond_destroy failed %d", ret);
    }

    HAL_Free(cond_);
}

int HAL_CondSignal(void *cond_, int broadcast)
{
    if (!cond_) {
        Log_e("invalid cond");
        return -1;
    }

    pthread_cond_t *cond = cond_;
    int r;
    if (broadcast)
        r = pthread_cond_broadcast(cond);
    else
        r = pthread_cond_signal(cond);

    if (r)
        Log_e("cond signal failed %d", r);
    return r ? -1 : 0;
}

int HAL_CondWait(void *cond_, void *lock_, unsigned long timeout_ms)
{
    int r;
    if (!cond_ || !lock_) {
        Log_e("invalid handle");
        return -1;
    }

    pthread_cond_t *cond  = cond_;
    pthread_mutex_t *lock = lock_;

    if (timeout_ms) {
        struct timeval now;
        struct timespec ts;
        gettimeofday(&now, NULL);

        ts.tv_sec  = now.tv_sec + timeout_ms / 1000;
        ts.tv_nsec = now.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;
        r          = pthread_cond_timedwait(cond, lock, &ts);
        if (r == ETIMEDOUT)
            return 1;
        else if (r)
            return -1;
        else
            return 0;
    } else {
        r = pthread_cond_wait(cond, lock);
        return r ? -1 : 0;
    }
}
