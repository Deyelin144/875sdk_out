/* ringbuffer.c */
#include <stdlib.h>
#include <string.h>
#include "ringbuffer.h"
#include "mutex.h"

struct ringbuff *ringbuff_init(uint32_t size)
{
    struct ringbuff *rb = malloc(sizeof(*rb));
    if (!rb) return NULL;

    rb->buffer = malloc(size);
    if (!rb->buffer) {
        free(rb);
        return NULL;
    }

    rb->lock = mutex_init();
    if (!rb->lock) {
        free(rb->buffer);
        free(rb);
        return NULL;
    }

    rb->size = size;
    rb->read_pos = 0;
    rb->write_pos = 0;
    rb->data_len = 0;
    return rb;
}

void ringbuff_deinit(struct ringbuff *rb)
{
    if (!rb) return;

    mutex_deinit(rb->lock);
    free(rb->buffer);
    free(rb);
}

static uint32_t get_free_space(struct ringbuff *rb)
{
    return rb->size - rb->data_len;
}

int ringbuff_write(struct ringbuff *rb, const uint8_t *data, uint32_t len)
{
    if (!rb || !data) return -1;

    mutex_lock(rb->lock);

    uint32_t free_space = get_free_space(rb);
    len = (len > free_space) ? free_space : len;
    if (len == 0) {
        mutex_unlock(rb->lock);
        return 0;
    }

    uint32_t first_chunk = rb->size - rb->write_pos;
    if (first_chunk > len) {
        first_chunk = len;
    }

    memcpy(rb->buffer + rb->write_pos, data, first_chunk);
    if (len > first_chunk) {
        memcpy(rb->buffer, data + first_chunk, len - first_chunk);
    }

    rb->write_pos = (rb->write_pos + len) % rb->size;
    rb->data_len += len;

    mutex_unlock(rb->lock);
    return len;
}

int ringbuff_read(struct ringbuff *rb, uint8_t *data, uint32_t len)
{
    if (!rb || !data) return -1;

    mutex_lock(rb->lock);
    len = (len > rb->data_len) ? rb->data_len : len;

    if (len == 0) {
        mutex_unlock(rb->lock);
        return 0;
    }

    uint32_t first_chunk = rb->size - rb->read_pos;
    if (first_chunk > len) {
        first_chunk = len;
    }

    memcpy(data, rb->buffer + rb->read_pos, first_chunk);
    if (len > first_chunk) {
        memcpy(data + first_chunk, rb->buffer, len - first_chunk);
    }

    rb->read_pos = (rb->read_pos + len) % rb->size;
    rb->data_len -= len;

    mutex_unlock(rb->lock);
    return len;
}

int ringbuff_reset(struct ringbuff *rb)
{
    if (!rb) return -1;

    mutex_lock(rb->lock);

    rb->read_pos = 0;
    rb->write_pos = 0;
    rb->data_len = 0;
    memset(rb->buffer, 0, rb->size);

    mutex_unlock(rb->lock);
    return 0;
}

uint32_t ringbuff_available(struct ringbuff *rb)
{
    if (!rb) return 0;

    mutex_lock(rb->lock);
    uint32_t available = rb->data_len;
    mutex_unlock(rb->lock);

    return available;
}
