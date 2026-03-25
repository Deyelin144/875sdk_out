#ifndef DUIKIT_QUEUE_H
#define DUIKIT_QUEUE_H

#include "duikit_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct duikit_queue* duikit_queue_t;

DUIKIT_EXPORT duikit_queue_t duikit_queue_create(unsigned int queue_size, unsigned int item_size);
DUIKIT_EXPORT duikit_err_t duikit_queue_delete(duikit_queue_t queue);
DUIKIT_EXPORT duikit_err_t duikit_queue_send(duikit_queue_t queue, void *buffer, int timeout);
DUIKIT_EXPORT duikit_err_t duikit_queue_recv(duikit_queue_t queue, void *buffer, int timeout);
DUIKIT_EXPORT duikit_err_t duikit_queue_reset(duikit_queue_t queue);
DUIKIT_EXPORT int duikit_queue_len(duikit_queue_t queue);

#ifdef __cplusplus
}
#endif
#endif
