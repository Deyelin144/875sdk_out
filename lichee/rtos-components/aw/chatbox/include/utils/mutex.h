#ifndef __MUTEX_H__
#define __MUTEX_H__

#ifdef __cplusplus
}
#endif

struct ch_mutex;

struct ch_mutex *mutex_init(void);

int mutex_deinit(struct ch_mutex *mutex);

int mutex_lock(struct ch_mutex *mutex);

int mutex_unlock(struct ch_mutex *mutex);

#ifdef __cplusplus
}
#endif

#endif
