#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

struct thread {
    pthread_t tid;
    void (*threadfn)(void *data);
    void *data;
};

struct thread *thread_create(void (*threadfn)(void *data), void *data,
                   const char *namefmt, int stacksize, int priority)
{
    struct thread* thread = malloc(sizeof(struct thread));
    if (!thread) return NULL;

    thread->threadfn = threadfn;
    thread->data = data;

    pthread_attr_t attr;
    struct sched_param sched;
    sched.sched_priority = priority;
    pthread_attr_init(&attr);
    pthread_attr_setschedparam(&attr, &sched);
    if (stacksize > 0) {
        pthread_attr_setstacksize(&attr, stacksize * 8);
    }
    if (pthread_create(&thread->tid, &attr,
                      (void* (*)(void*))threadfn, data) != 0) {
        free(thread);
        return NULL;
    }

    pthread_attr_destroy(&attr);
    if (namefmt)
        pthread_setname_np(thread->tid, namefmt);
    return thread;
}

int thread_stop(struct thread *thread)
{
	if (!thread)
		return 0;

	pthread_cancel(thread->tid);
	pthread_join(thread->tid, NULL);

	free(thread);
    return 0;
}

int thread_msleep(unsigned int ms)
{
	return msleep(ms);
}

int thread_sleep(unsigned int sec)
{
    return sleep(sec);
}
