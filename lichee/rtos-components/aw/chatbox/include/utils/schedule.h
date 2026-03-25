/* schedule.h */
#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
#if __cplusplus
}; // extern "C"
#endif
struct schedule;

typedef void (*schedule_callback_t)(void* arg);

struct schedule* schedule_create(void);
void schedule_destroy(struct schedule* sched);
int schedule_post(struct schedule* sched, schedule_callback_t fn, void* arg);
void schedule_run(struct schedule* sched, void* user_data);
#if __cplusplus
}; // extern "C"
#endif
#endif
