#ifndef __EVENT_GROUP_H__
#define __EVENT_GROUP_H__
#if __cplusplus
extern "C" {
#endif

struct event_group;

struct event_group *event_group_create(void);

void event_group_set_bit(struct event_group *evt, unsigned char event_bit);

void event_group_clear_bit(struct event_group *evt, unsigned char event_bit);

unsigned char event_group_wait(struct event_group* eg, unsigned char wait_bits,
    unsigned char clear_bits, int wait_all, unsigned int timeout_ms);

unsigned char event_group_get_bit(struct event_group *evt, unsigned char event_bit);

void event_group_delete(struct event_group *evt);

#if __cplusplus
}; // extern "C"
#endif
#endif
