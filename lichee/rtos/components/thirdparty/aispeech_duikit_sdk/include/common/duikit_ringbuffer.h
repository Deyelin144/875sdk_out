#ifndef DUIKIT_RINGBUFFER_H
#define DUIKIT_RINGBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "duikit_common.h"

typedef struct duikit_ringbuffer* duikit_ringbuffer_t;

DUIKIT_EXPORT duikit_ringbuffer_t duikit_ringbuffer_init(int size);

DUIKIT_EXPORT void duikit_ringbuffer_release(duikit_ringbuffer_t rb);

DUIKIT_EXPORT void duikit_ringbuffer_reset(duikit_ringbuffer_t rb);

DUIKIT_EXPORT int duikit_ringbuffer_resize(duikit_ringbuffer_t rb, int size);

DUIKIT_EXPORT uint32_t duikit_ringbuffer_length(duikit_ringbuffer_t rb);

DUIKIT_EXPORT uint32_t duikit_ringbuffer_valid(duikit_ringbuffer_t rb);

DUIKIT_EXPORT bool duikit_ringbuffer_is_full(duikit_ringbuffer_t rb);

DUIKIT_EXPORT bool duikit_ringbuffer_is_empty(duikit_ringbuffer_t rb);

DUIKIT_EXPORT int duikit_ringbuffer_get(duikit_ringbuffer_t rb, void *buf, int size, unsigned int timeout);

DUIKIT_EXPORT int duikit_ringbuffer_put(duikit_ringbuffer_t rb, const void *buf, int size);

DUIKIT_EXPORT int duikit_ringbuffer_force_put(duikit_ringbuffer_t rb, const void *buf, int size);

DUIKIT_EXPORT int duikit_ringbuffer_wait_put(duikit_ringbuffer_t rb, const void *buf, int size, int timeout);

#ifdef __cplusplus
}
#endif

#endif
