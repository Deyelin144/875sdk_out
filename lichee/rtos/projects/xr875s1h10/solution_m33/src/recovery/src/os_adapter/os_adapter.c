
#include "os_adapter.h"
#include "sys/time.h"
#include "kernel/os/os.h"
#include <hal_thread.h>
#include "kernel/os/os_errno.h"
#include "pm_task.h"
#include <stdio.h>
#include <stdlib.h>

void *os_adapter_signal_mutex_create(int init_count)
{
    int ret = -1;
    XR_OS_Semaphore_t *sem = (XR_OS_Semaphore_t *)calloc(1, sizeof(XR_OS_Semaphore_t));

    if (XR_OS_OK != (ret = XR_OS_SemaphoreCreateBinary(sem))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
        free(sem);
        sem = NULL;
    }

    return (void *)sem;
}

void *os_adapter_signal_create(int init_count, unsigned int maxCount)
{
    int ret = -1;
    XR_OS_Semaphore_t *sem = (XR_OS_Semaphore_t *)calloc(1, sizeof(XR_OS_Semaphore_t));

    if (XR_OS_OK != (ret = XR_OS_SemaphoreCreate(sem, init_count, maxCount))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
        free(sem);
        sem = NULL;
    }

    return (void *)sem;
}

int os_adapter_signal_wait(void *sem, long time_ms)
{
    int ret = -1;

    if (XR_OS_OK != (ret = XR_OS_SemaphoreWait(sem, time_ms))) {
        // printf("[%d], %s failed ret = %d. sem = %p, time_ms = %d\n", __LINE__, __func__, ret, sem, time_ms);
    }

    return ret;
}

int os_adapter_signal_post(void *sem)
{
    int ret = -1;

    if (XR_OS_OK != (ret = XR_OS_SemaphoreRelease(sem))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }

    return ret;
}

int os_adapter_signal_delete(void **sem)
{
    int ret = 0;
    if (*sem == NULL) {
        goto exit;
    }

    if (XR_OS_OK != (ret = XR_OS_SemaphoreDelete(*sem))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }
    free(*sem);
    *sem = NULL;

exit:
    return ret;
}

/***************************互斥锁*********************************/
void *os_adapter_mutex_create(void)
{
    int ret = -1;
    XR_OS_Mutex_t *mutex = (XR_OS_Mutex_t *)calloc(1, sizeof(XR_OS_Mutex_t));

    if (XR_OS_OK != (ret = XR_OS_MutexCreate(mutex))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
        free(mutex);
        mutex = NULL;
    }

    return (void *)mutex;
}

int os_adapter_mutex_lock(void *mutex, long time_ms)
{
    int ret = -1;

    if (XR_OS_OK != (ret = XR_OS_MutexLock(mutex, time_ms))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }

    return ret;
}

int os_adapter_mutex_unlock(void *mutex)
{
    int ret = -1;

    if (XR_OS_OK != (ret = XR_OS_MutexUnlock(mutex))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }

    return ret;
}

int os_adapter_mutex_delete(void **mutex)
{
    int ret = 0;

    if (NULL == *mutex) {
        goto exit;
    }
    if (XR_OS_OK != (ret = XR_OS_MutexDelete(*mutex))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }
    free(*mutex);
    *mutex = NULL;

exit:
    return ret;
}

/******************************queue api***********************************/
void *os_adapter_queue_create(unsigned int queue_len, unsigned int size)
{
    int ret = -1;
    XR_OS_Queue_t *queue = (XR_OS_Queue_t *)calloc(1, sizeof(XR_OS_Queue_t));

    if (XR_OS_OK != (ret = XR_OS_QueueCreate(queue, queue_len, size))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
        free(queue);
        queue = NULL;
    }

    return (void *)queue;
}

int os_adapter_queue_send(void *queue, const void *item, long time_ms)
{
    int ret = -1;

    if (XR_OS_OK != (ret = XR_OS_QueueSend(queue, item, time_ms))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }

    return ret;
}

int os_adapter_queue_recv(void *queue, void *item, long time_ms)
{
    int ret = -1;

    if (XR_OS_OK != (ret = XR_OS_QueueReceive(queue, item, time_ms))) {
        // printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }

    return ret;
}

int os_adapter_queue_delete(void **queue)
{
    int ret = 0;

    if (NULL == *queue) {
        goto exit;
    }

    if (XR_OS_OK != (ret = XR_OS_QueueDelete(*queue))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }
    free(*queue);
    *queue = NULL;

exit:
    return ret;
}

int os_adapter_queue_check(void *queue)
{
    return XR_OS_QueueCheckFull(queue);
}

/*****************************thread api***********************************/
//cmd: 0：gulite_os转sdk的值， 1：sdk转gulite_os
static unsigned char os_adapter_thread_priority_transfer(unsigned char priority, unsigned char cmd)
{
    unsigned char prio = 0;

    if (cmd) {
        switch (priority) {
            case XR_OS_PRIORITY_IDLE:
                prio = OS_ADAPTER_PRIORITY_IDLE;
                break;
            case XR_OS_PRIORITY_LOW:
                prio = OS_ADAPTER_PRIORITY_LOW;
                break;
            case XR_OS_PRIORITY_BELOW_NORMAL:
                prio = OS_ADAPTER_PRIORITY_BELOW_NORMAL;
                break;
            case XR_OS_PRIORITY_NORMAL:
                prio = OS_ADAPTER_PRIORITY_NORMAL;
                break;
            case XR_OS_PRIORITY_ABOVE_NORMAL:
                prio = OS_ADAPTER_PRIORITY_ABOVE_NORMAL;
                break;
            case XR_OS_PRIORITY_HIGH:
                prio = OS_ADAPTER_PRIORITY_HIGH;
                break;
            case XR_OS_PRIORITY_REAL_TIME:
                prio = OS_ADAPTER_PRIORITY_REAL_TIME;
                break;
            default:
                prio = OS_ADAPTER_PRIORITY_NORMAL;
                break;
	    }
    } else {
        switch (priority) {
            case OS_ADAPTER_PRIORITY_IDLE:
                prio = XR_OS_PRIORITY_IDLE;
                break;
            case OS_ADAPTER_PRIORITY_LOW:
                prio = XR_OS_PRIORITY_LOW;
                break;
            case OS_ADAPTER_PRIORITY_BELOW_NORMAL:
                prio = XR_OS_PRIORITY_BELOW_NORMAL;
                break;
            case OS_ADAPTER_PRIORITY_NORMAL:
                prio = XR_OS_PRIORITY_NORMAL;
                break;
            case OS_ADAPTER_PRIORITY_ABOVE_NORMAL:
                prio = XR_OS_PRIORITY_ABOVE_NORMAL;
                break;
            case OS_ADAPTER_PRIORITY_HIGH:
                prio = XR_OS_PRIORITY_HIGH;
                break;
            case OS_ADAPTER_PRIORITY_REAL_TIME:
                prio = XR_OS_PRIORITY_REAL_TIME;
                break;
            default:
                prio = XR_OS_PRIORITY_NORMAL;
                break;
	    }
    }

    return prio;
}

static int os_adapter_thread_prio_reversal(unsigned char priority)
{
    int ret;
	int prio;
	int prio_level;
	int prio_low;
	int prio_normal;
	int prio_high;

	/* priority level need to be greater than 4 */
	prio_level = HAL_THREAD_PRIORITY_HIGHEST - HAL_THREAD_PRIORITY_LOWEST + 1;
	prio_low = (prio_level >> 2) + HAL_THREAD_PRIORITY_LOWEST;
	prio_normal = (prio_level >> 1) + HAL_THREAD_PRIORITY_LOWEST;
	prio_high = ((prio_level * 3) >> 2) + HAL_THREAD_PRIORITY_LOWEST;

	switch (priority) {
	case XR_OS_PRIORITY_IDLE:
		prio = HAL_THREAD_PRIORITY_LOWEST;
		break;
	case XR_OS_PRIORITY_ABOVE_IDLE:
		prio = HAL_THREAD_PRIORITY_LOWEST + 1;
		break;
	case XR_OS_PRIORITY_BELOW_LOW:
		prio = prio_low - 1;
		break;
	case XR_OS_PRIORITY_LOW:
		prio = prio_low;
		break;
	case XR_OS_PRIORITY_ABOVE_LOW:
		prio = prio_low + 1;
		break;
	case XR_OS_PRIORITY_BELOW_NORMAL:
		prio = prio_normal - 1;
		break;
	case XR_OS_PRIORITY_NORMAL:
		prio = prio_normal;
		break;
	case XR_OS_PRIORITY_ABOVE_NORMAL:
		/*
		 * This priority need to be greater or equal to the priority of console.
		 * Because in some case, console task will take a lot of time, which will
		 * make bt unable to execute.
		 */
		prio = prio_normal + 2;
		break;
	case XR_OS_PRIORITY_BELOW_HIGH:
		prio = prio_high - 1;
		break;
	case XR_OS_PRIORITY_HIGH:
		prio = prio_high;
		break;
	case XR_OS_PRIORITY_ABOVE_HIGH:
		prio = prio_high + 1;
		break;
	case XR_OS_PRIORITY_BELOW_REAL_TIME:
		prio = HAL_THREAD_PRIORITY_HIGHEST - 1;
		break;
	case XR_OS_PRIORITY_REAL_TIME:
		prio = HAL_THREAD_PRIORITY_HIGHEST;
		break;
	default:
		prio = prio_normal;
		break;
	}

    return prio;
}

int os_adapter_thread_create(void **thread, void *thread_func, void *param, const char *name, int prior, int stack_depth)
{
    int ret = -1;
    *thread = (XR_OS_Thread_t *)calloc(1, sizeof(XR_OS_Thread_t));

    unsigned char priority = os_adapter_thread_priority_transfer(prior, 0);

    if (XR_OS_OK != (ret = XR_OS_ThreadCreate((XR_OS_Thread_t *)(*thread), name, thread_func, param, priority, stack_depth + 1024 * 3))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
        free(*thread);
        *thread = NULL;
        ret = -1;
        goto exit;
    } else {
       XR_OS_Thread_t *tid = (XR_OS_Thread_t *)(*thread);
       pm_task_register(tid->handle, PM_TASK_TYPE_FREEZE_AT_ONCE);
    }
    printf("-------------->%s %p\n", name, *thread);

    ret = 0;
exit:
    return ret;
}

long os_adapter_thread_get_free_stack(void *thread)
{
    int free_stack = 0;

    if (thread == NULL) {
        free_stack = uxTaskGetStackHighWaterMark(NULL);
    } else {
        free_stack = uxTaskGetStackHighWaterMark(((XR_OS_Thread_t *)thread)->handle);
    }
    return free_stack;
}

int os_adapter_thread_delete(void **thread_handle)
{
    TaskHandle_t handle;
    TaskHandle_t curHandle;

    if (*thread_handle == NULL) {
        return XR_OS_OK;
    }

    XR_OS_Thread_t *tid = (XR_OS_Thread_t *)(*thread_handle);

    printf("============>%s, %ld.\n", pcTaskGetTaskName(tid->handle), os_adapter_thread_get_free_stack(tid));
    pm_task_unregister(tid->handle);

    //  OS_HANDLE_ASSERT(OS_ThreadIsValid(tid), tid->handle);
    if (0 == XR_OS_ThreadIsValid(tid)) {
        printf("os_adapter_thread_delete handle %p\n", tid->handle);
        free(tid);
        *thread_handle = NULL;
        return XR_OS_E_PARAM;
    }

    handle = tid->handle;
    curHandle = xTaskGetCurrentTaskHandle();
    if (handle == curHandle) {
        XR_OS_ThreadSetInvalid(tid);
        free(tid);
        *thread_handle = NULL;
        XR_OS_ThreadDelete(NULL);
    } else {
        XR_OS_ThreadDelete(tid);
        free(tid);
        *thread_handle = NULL;
    }

    return XR_OS_OK;
}

int os_adapter_thread_is_valid(void *thread_handle)
{
    return XR_OS_ThreadIsValid(thread_handle);
}

int os_adapter_thread_set_priority(void *thread, unsigned char priority)
{
    unsigned char prior = os_adapter_thread_priority_transfer(priority, 0);
    prior = os_adapter_thread_prio_reversal(prior);
#if INCLUDE_vTaskPrioritySet
    if (thread == NULL) {
        vTaskPrioritySet(NULL, prior);
    } else {
        vTaskPrioritySet(((XR_OS_Thread_t *)thread)->handle, prior);
    }
    return XR_OS_OK;
#else
    printf("is not support.\n");
    return 0;
#endif
}

int os_adapter_thread_get_priority(void *thread, unsigned long *priority)
{
    // OS_HANDLE_ASSERT(OS_ThreadIsValid(thread), ((XR_OS_Thread_t *)thread)->handle);

    if (thread == NULL) {
        *priority = uxTaskPriorityGet(NULL);
    } else {
        *priority = uxTaskPriorityGet(((XR_OS_Thread_t *)thread)->handle);
    }
    *priority = (unsigned long)os_adapter_thread_priority_transfer(*priority, 1);
    return 0;
}

long os_adapter_thread_get_tid(void *thread)
{
    if (NULL != thread) {
        return (long)((*((XR_OS_Thread_t *)thread)).handle);
    } else {
#if INCLUDE_xTaskGetCurrentTaskHandle
        return (unsigned long)xTaskGetCurrentTaskHandle();
#else
        printf("is not support.\n");
        return 0;
#endif
    }
}

/*****************************time api************************************/
void os_adapter_msleep(long time_ms)
{
    return XR_OS_MSleep((XR_OS_Time_t)time_ms);
}

static long get_sys_runtime(int type)
{
    return 0;
}

long os_adapter_get_time_ms(void)
{
    return XR_OS_JiffiesToMSecs(XR_OS_GetJiffies());
}

long os_adapter_get_forever_time(void)
{
    return portMAX_DELAY;
}

void os_adapter_get_timestamp(char *timestamp)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sprintf(timestamp, "%lu%03lu", tv.tv_sec, tv.tv_usec / 1000);
}

/*****************************timer api************************************/
void *os_adapter_timer_create(bool repeat, void *func, void *arg, int period_ms)
{
    int ret = -1;
    XR_OS_Timer_t *timer = (XR_OS_Timer_t *)calloc(1, sizeof(XR_OS_Timer_t));

    if (XR_OS_OK != (ret = XR_OS_TimerCreate(timer, repeat, func, arg, period_ms))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
        free(timer);
        timer = NULL;
    }

    return (void *)timer;
}

int os_adapter_timer_delete(void **timer)
{
    int ret = 0;

    if (NULL == *timer) {
        goto exit;
    }
    if (XR_OS_OK != (ret = XR_OS_TimerDelete(*timer))) {
        printf("[%d], %s failed ret = %d.\n", __LINE__, __func__, ret);
    }
    free(*timer);
    *timer = NULL;

exit:
    return ret;
}

int os_adapter_timer_start(void *timer)
{
    return XR_OS_TimerStart(timer);
}

int os_adapter_timer_stop(void *timer)
{
    return XR_OS_TimerStop(timer);
}

int os_adapter_timer_is_active(void *timer)
{
    return XR_OS_TimerIsActive(timer);
}

int os_adapter_timer_change_period(void *timer, int period_ms)
{
    return XR_OS_TimerChangePeriod(timer, period_ms);
}

int os_adapter_timer_get_time_ms(void* timer)
{
    return XR_OS_TimerGetExpiryTime(timer);	
}

int os_adapter_get_errno(void)
{
    return XR_OS_GetErrno();
}

void os_adapter_set_errno(int err)
{
    XR_OS_SetErrno(err);
}

static os_adapter_t s_os_adapter = {
    .signal_mutex_create = os_adapter_signal_mutex_create,
    .signal_create = os_adapter_signal_create,
    .signal_wait = os_adapter_signal_wait,
    .signal_post = os_adapter_signal_post,
    .signal_delete = os_adapter_signal_delete,
    .mutex_create = os_adapter_mutex_create,
    .mutex_lock = os_adapter_mutex_lock,
    .mutex_unlock = os_adapter_mutex_unlock,
    .mutex_delete = os_adapter_mutex_delete,
    .queue_create = os_adapter_queue_create,
    .queue_send = os_adapter_queue_send,
    .queue_recv = os_adapter_queue_recv,
    .queue_delete = os_adapter_queue_delete,
    .queue_check = os_adapter_queue_check,
    .thread_create = os_adapter_thread_create,
    .thread_delete = os_adapter_thread_delete,
    .thread_is_valid = os_adapter_thread_is_valid,
    .thread_set_priority = os_adapter_thread_set_priority,
    .thread_get_priority = os_adapter_thread_get_priority,
    .thread_get_tid = os_adapter_thread_get_tid,
    .thread_get_free_stack = os_adapter_thread_get_free_stack,
    .msleep = os_adapter_msleep,
    .get_time_ms = os_adapter_get_time_ms,
    .get_forever_time = os_adapter_get_forever_time,
    .get_timestamp = os_adapter_get_timestamp,
    .timer_create = os_adapter_timer_create,
    .timer_delete = os_adapter_timer_delete,
    .timer_start = os_adapter_timer_start,
    .timer_stop = os_adapter_timer_stop,
    .timer_is_active = os_adapter_timer_is_active,
    .timer_change_period = os_adapter_timer_change_period,
    .timer_get_time_ms = os_adapter_timer_get_time_ms,//获取定时器剩余时间
    .os_get_errno = os_adapter_get_errno,
    .os_set_errno = os_adapter_set_errno,
    .malloc = malloc,
    .realloc = realloc,
    .calloc = calloc,
    .free = free,
};

os_adapter_t *os_adapter(void)
{
    return &s_os_adapter;
}
