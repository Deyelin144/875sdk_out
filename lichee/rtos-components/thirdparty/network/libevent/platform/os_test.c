/**
 * @file os_test.c
 * @author hubertxxu (hubertxxu@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-26
 * 
 * @copyright
 * Tencent is pleased to support the open source community by making IoT Hub available. 
 * Copyright(C) 2021 - 2026 THL A29 Limited, a Tencent company.All rights reserved.
 * Licensed under the MIT License(the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "tc_iot_hal.h"
#include "tc_iot_log.h"

#define NUM_THREADS 3

void *mutex;
void *recursive_mutex;
void *cond;
int shared_data = 0;

void random_sleep(char id, int max_ms)
{
    int delay_ms;
    delay_ms = (HAL_Random() % max_ms) + 1;
    Log_d("Thread(%d) delay_ms:%d", id, delay_ms);
    HAL_SleepMs(delay_ms);
}

// 线程运行函数示例
void thread_func(void *arg) {
    char id = *(char *)arg - '0';
    
    random_sleep(id, 5);
    // 测试普通互斥锁
    HAL_MutexLock(mutex);
    Log_d("Thread(%d) acquired mutex\n", id);
    random_sleep(id, 6);
    shared_data++;
    Log_d("Thread(%d) incremented shared_data to %d\n", id, shared_data);
    HAL_MutexUnlock(mutex);
    Log_d("Thread(%d) released mutex\n", id);

    // 测试递归互斥锁
    HAL_RecursiveMutexLock(recursive_mutex, 0);
    Log_d("Thread(%d) acquired recursive mutex\n", id);
    random_sleep(id, 7);    
    HAL_RecursiveMutexLock(recursive_mutex, 0);
    Log_d("Thread(%d) acquired recursive mutex again\n", id);
    shared_data++;
    Log_d("Thread(%d) incremented shared_data to %d\n", id, shared_data);
    HAL_RecursiveMutexUnLock(recursive_mutex);

    random_sleep(id, 6);

    Log_d("Thread(%d) released recursive mutex\n", id);
    HAL_RecursiveMutexUnLock(recursive_mutex);
    Log_d("Thread(%d) released recursive mutex again\n", id);

    // 测试条件变量
    HAL_MutexLock(mutex);
    while (shared_data <= NUM_THREADS * 2) {
        Log_d("Thread(%d) waiting for condition\n", id);
        HAL_CondWait(cond, mutex, 0);
    }
    Log_d("Thread(%d) received condition signal\n", id);
    HAL_MutexUnlock(mutex);
}

static int os_thread_test(void)
{   
    int rc = -1;

    mutex = HAL_MutexCreate();
   if(!mutex){
        Log_e("Mutex create failed\n");
        goto exit;
   }
    recursive_mutex = HAL_RecursiveMutexCreate();
    if(!recursive_mutex){
        Log_e("RecursiveMutex create failed\n");
        goto exit;
    }
    cond = HAL_CondCreate();
    if(!cond){
        Log_e("Cond create failed\n");
        goto exit;
    }

    static ThreadParams tp1,tp2,tp3;
    tp1.thread_name = "TestThread1";
    tp1.thread_func = thread_func;
    tp1.user_arg = "1";
    tp1.stack_size = 1024 *10;
    tp1.priority = THREAD_PRIORITY_NORMAL;

    tp2.thread_name = "TestThread2";
    tp2.thread_func = thread_func;
    tp2.user_arg = "2";
    tp2.stack_size = 1024 * 20;
    tp2.priority = THREAD_PRIORITY_HIGH;

    tp3.thread_name = "TestThread3";
    tp3.thread_func = thread_func;
    tp3.user_arg = "3";
    tp3.stack_size = 1024 * 20;
    tp3.priority = THREAD_PRIORITY_HIGHEST;

    rc = HAL_ThreadCreate(&tp1);
    rc |= HAL_ThreadCreate(&tp2);
    rc |= HAL_ThreadCreate(&tp3);
    if(rc){
        Log_e("Thread create failed\n");
        goto exit;
    }

    HAL_SleepMs(3000);

    // 发送条件变量信号
    HAL_MutexLock(mutex);
    shared_data = NUM_THREADS * 2 + 1;
    HAL_CondSignal(cond, 1); // 广播信号，唤醒所有等待线程
    HAL_MutexUnlock(mutex);

    HAL_SleepMs(1000);
    rc = 0;

exit:
    if(mutex) HAL_MutexDestroy(mutex);
    if(recursive_mutex) HAL_RecursiveMutexDestroy(recursive_mutex);
    if(cond) HAL_CondFree(cond);
    return rc;
}

static int os_time_test(void)
{
    uint32_t t1 = HAL_GetTicksTimeMs();
    Log_d("HAL_GetTicksTimeMs : %u \n",t1);
    HAL_SleepMs(1000);
    Log_d("HAL_GetTicksTimeMs : %u == %u ? \n",HAL_GetTicksTimeMs(), t1+1000);
    Log_d("HAL_GetTimeMs : %lu \n",HAL_GetTimeMs());
    Log_d("HAL_GetTimeSecond : %u \n",HAL_GetTimeSecond());
    Timer timer;
    HAL_Timer_init(&timer);
    HAL_Timer_countdown_ms(&timer, 1000);
    HAL_SleepMs(300);
    Log_d("HAL_Timer_is_expired : %u \n",HAL_Timer_expired(&timer));
    Log_d("HAL_Timer_remain : %u == 700 ?\n",HAL_Timer_remain(&timer));
    HAL_Timer_init(&timer);
    HAL_Timer_countdown(&timer, 5);
    HAL_SleepMs(1000);
    Log_d("HAL_Timer_is_expired : %d \n",HAL_Timer_expired(&timer));
    Log_d("HAL_Timer_remain : %d == 4000 ?\n",HAL_Timer_remain(&timer));

    return 0;
}

static int os_file_test(void)
{
#define DEVICE_INFO "{\"productId\": \"35BQFXQE3G\",\"deviceName\": \"YOUR_DEVICE_NAME\",\"key_deviceinfo\": {\"deviceSecret\": \"IOT_PSK\"}}"

    uint32_t disk_size = HAL_FileGetDiskSize();
    if(disk_size < 4){
        Log_e("Disk size(%d)Kbytes is too small\n",disk_size);
        return -1;
    }
    Log_d("Disk size : %d Kbytes\n",disk_size);

    HAL_FileRemove("sdmmc/near.pcm");

    void *p = HAL_FileOpen("sdmmc/near.pcm", "w+");
	printf("p=%p\n",p);
    if(!p){
        Log_e("File open failed\n");
        return -1;
    }

    size_t wirte_len = HAL_FileWrite(DEVICE_INFO, 1, 108, p);
    if(wirte_len != 108){
        Log_e("File write failed\n");
        HAL_FileClose(p);
        return -1;
    }
    
    if( HAL_FileFlush(p)){
        Log_e("File flush failed\n");
        HAL_FileClose(p);
        return -1;
    }

    char buf[256] = {0};
    memset(buf, 0, sizeof(buf));
    HAL_FileSeek(p, 0, HAL_SEEK_SET);
    size_t read_len = HAL_FileRead(buf, 1, 108, p);
    if(read_len != 108){
        Log_e("File read failed\n");
        return -1;
    }
    Log_d("File content : %s\n",buf);

    long file_size =  HAL_FileSize(p);
    Log_d("File size : %ld\n",file_size);

    if( HAL_FileEof(p)){
        Log_d("File is eof\n");
    }

    if( HAL_FileError(p)){
        Log_e("File is error\n");
    }

    HAL_FileClose(p);

    HAL_FileRemove("./device_info.json");

    return 0;
}

int os_test(void)
{
    int rc = -1;
    // 测试时间
    HAL_Printf("\r\n============================>>> os time test <<<=========================>>>\r\n\r\n");
    os_time_test();

    // 测试文件
    HAL_Printf("\r\n============================>>> os file test <<<=========================>>>\r\n\r\n");
    rc = os_file_test();
    rc = 0;
    if(rc){
        Log_e("File test failed\n");
        return rc;
    }

    // 测试线程 锁 信号量等
    HAL_Printf("\r\n============================>>> os thread test <<<==1=======================>>>\r\n\r\n");
    rc = os_thread_test();
    if(rc){
        Log_e("Thread test failed\n");
        return rc;
    }
    // 测试内存申请
    HAL_Printf("\r\n============================>>> os memory test <<<===2======================>>>\r\n\r\n");
    uint32_t can_use_mem = HAL_GetMemSize();
    Log_i("can use memory size : %dbytes %dKbytes",can_use_mem, can_use_mem>>10);
    char *p[20] = {NULL};
    int total_malloc = 512*1024;
    if(total_malloc > can_use_mem){
        Log_e("malloc %d bytes is too large\n", total_malloc);
        return -1;
    }
    p[0] = HAL_Malloc(total_malloc);
    if(!p[0]){
        Log_e("Malloc %d bytes failed\n", total_malloc);
        return -1;
    }
    for(int i = 1; i < 20; i++){
        p[i] = HAL_Malloc(64*i*i);
        if(!p[i]){
            Log_e("Malloc %d bytes failed\n",64*i*i);
            return -1;
        }
        Log_d("malloc %d bytes in %p\n", 64*i*i, p[i]);
        memset(p[i], i, 64*i*i);
        total_malloc += 64*i*i;
    }
    Log_d("total malloc %d bytes\n", total_malloc);
    for(int i = 0; i < 20; i++){
        HAL_Free(p[i]);
    }

    return 0;    
}











