#ifndef __ADB_MISC_H
#define __ADB_MISC_H
#include <pthread.h>
#include <string.h>

#define adb_malloc(size)       malloc(size)
#define adb_calloc(num, size)  calloc(num, size)
#define adb_realloc(ptr, size) realloc(ptr, size)
#define adb_strdup(s)          strdup(s)
//#define adb_scandir(dirp,namelist,filter,compar)	scandir(dirp,namelist,filter,compar)
#define adb_free(ptr) free(ptr)

#define ADB_THREAD_LOW_PRIORITY    15
#define ADB_THREAD_NORMAL_PRIORITY 17
#define ADB_THREAD_HIGH_PRIORITY   18

#define ADB_THREAD_STACK_SIZE (8 * 1024)
typedef void *(*adb_thread_func_t)(void *arg);

static inline int adb_thread_create(pthread_t *tid, adb_thread_func_t start, const char *name,
                                    void *arg, int priority, int joinable)
{
    int ret;
    pthread_attr_t attr;
    struct sched_param sched;

    memset(&sched, 0, sizeof(sched));
    sched.sched_priority = priority;
    pthread_attr_init(&attr);
    pthread_attr_setschedparam(&attr, &sched);
    pthread_attr_setdetachstate(&attr, joinable);
    pthread_attr_setstacksize(&attr, ADB_THREAD_STACK_SIZE);
    ret = pthread_create(tid, &attr, start, arg);
    if (ret < 0)
        return ret;
    pthread_setname_np(*tid, name);
    return ret;
}

static inline int adb_thread_wait(adb_thread_t tid, void **retval)
{
    return pthread_join(tid, retval);
}

#endif