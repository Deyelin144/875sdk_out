/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _AMP_THREADPOOL_H
#define _AMP_THREADPOOL_H

#include <hal_mutex.h>

#define DEFAULT_TIME            1
#define MIN_WAIT_TASK_NUM       2
#define DEFAULT_THREAD_NUM      2
#define DEFAULT_PRIORITY        (HAL_THREAD_PRIORITY_HIGHEST - 1)

#define AMP_THREAD_POOL_ADMIN_TASK_STACK_SIZE (1024 * 1)
#define AMP_THREAD_POOL_TASK_STACK_SIZE (1024 * 2)

#define AMP_THD_POOL_MIN_NUM        5
#define AMP_THD_POOL_MAX_NUM        20
#define AMP_THD_POOL_QUEUE_MAX_SIZE 50

#define CONFIG_AMP_THREADPOOL_DEBUG

typedef struct
{
    void (*function)(void *);
    void *arg;
} threadpool_task_t;

typedef struct
{
    void *thread;
    unsigned int run_num;
} thread_debug_t;

typedef struct _threadpool_t
{
    hal_mutex_t lock;
    hal_mutex_t thread_counter;

#ifdef CONFIG_AMP_THREADPOOL_DEBUG
    thread_debug_t *threads;
#else
    TaskHandle_t *threads;
#endif
    TaskHandle_t admin_tid;
    QueueHandle_t task_queue;

    int min_thr_num;
    int max_thr_num;
    int live_thr_num;
    int busy_thr_num;
    int wait_exit_thr_num;

    int queue_size;
    int queue_max_size;
} threadpool_t;

threadpool_t *amp_get_threadpool(void);
int amp_threadpool_init(void);
int amp_threadpool_add_task(threadpool_t *pool, void (*function)(void *arg), void *arg);

#endif
