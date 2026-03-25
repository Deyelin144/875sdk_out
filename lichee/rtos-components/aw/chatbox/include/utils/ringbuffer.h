#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#if __cplusplus
}; // extern "C"
#endif

#include <stdint.h>

struct ringbuff {
    uint8_t *buffer;
    uint32_t size;
    uint32_t read_pos;
    uint32_t write_pos;
    struct ch_mutex *lock;
    uint32_t data_len;
};

struct ringbuff *ringbuff_init(uint32_t size);

void ringbuff_deinit(struct ringbuff *rb);

int ringbuff_write(struct ringbuff *rb, const uint8_t *data, uint32_t len);

int ringbuff_read(struct ringbuff *rb, uint8_t *data, uint32_t len);

int ringbuff_reset(struct ringbuff *rb);

uint32_t ringbuff_available(struct ringbuff *rb);

#if __cplusplus
}; // extern "C"
#endif
#endif
