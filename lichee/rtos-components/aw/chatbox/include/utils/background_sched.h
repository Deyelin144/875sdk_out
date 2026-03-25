#ifndef __BACKGROUND_SCHED_H__
#define __BACKGROUND_SCHED_H__
#if __cplusplus
}; // extern "C"
#endif
struct sched;

typedef void (*bg_sched_cb)(void* arg);

struct sched* bg_sched_create(const char *namefmt, int stacksize, int priority);

void bg_sched_destroy(struct sched* sched);

int bg_sched_post(struct sched* sched, bg_sched_cb fn, void* arg);
#if __cplusplus
}; // extern "C"
#endif
#endif
