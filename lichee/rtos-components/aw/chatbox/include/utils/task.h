#ifndef __TASK_H__
#define __TASK_H__

struct thread;

struct thread *thread_create(void (*threadfn)(void *data),
		void *data, const char *namefmt, int stacksize, int priority);

int thread_stop(struct thread *thread);

int thread_start(struct thread *thread);

int thread_msleep(unsigned int ms);

int thread_sleep(unsigned int s);

#endif
