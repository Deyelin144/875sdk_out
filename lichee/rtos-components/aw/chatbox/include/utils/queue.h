#ifndef __QUEUE_H__
#define __QUEUE_H__

struct queue;

struct queue *queue_create(const char *name,
		unsigned int item_size, unsigned int queue_size);

void queue_delete(struct queue *queue);

int queue_send(struct queue *queue, void *buffer);

int queue_recv(struct queue *queue, void *buffer, int timeout_ms);

int queue_reset(struct queue *queue);
#endif
