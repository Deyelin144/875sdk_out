#include <stdlib.h>
#include <string.h>
#include "list_buff.h"
#include "mutex.h"

struct buff_node {
    void *buff_data;
    unsigned int buff_len;
    struct buff_node *next;
    enum buff_store_mode mode;
};

struct list_buff {
    struct buff_node *head;
    struct buff_node *tail;
    unsigned int count;
    unsigned int total_len;
    struct ch_mutex *mutex;
};

struct list_buff *list_buff_create(void)
{
    struct list_buff *cache = (struct list_buff *)malloc(sizeof(struct list_buff));
    if (!cache) return NULL;

    cache->mutex = mutex_init();  // 使用项目初始化接口
    if (!cache->mutex) {
        free(cache);
        return NULL;
    }

    if (cache) {
        cache->head = NULL;
        cache->tail = NULL;
        cache->count = 0;
        cache->total_len = 0;
    }
    return cache;
}

void list_buff_destroy(struct list_buff *cache, void (*free_buff)(void *))
{
    if (!cache) return;

    mutex_lock(cache->mutex);
    list_buff_clear(cache, free_buff);
    mutex_unlock(cache->mutex);

    mutex_deinit(cache->mutex);
    free(cache);
}

int list_buff_append(struct list_buff *cache, void *buff_data,
		unsigned int buff_len, enum buff_store_mode mode)
{
    if (!cache || !buff_data || buff_len == 0) return -1;

    if (mutex_lock(cache->mutex) != 0)
        return -1;

    struct buff_node *node = (struct buff_node *)malloc(sizeof(struct buff_node));
    if (!node) {
        mutex_unlock(cache->mutex);
        return -1;
    }

    if (mode == BUFF_STORE_COPY) {
        node->buff_data = malloc(buff_len);
        if (!node->buff_data) {
            free(node);
            return -1;
        }
        memcpy(node->buff_data, buff_data, buff_len);
    } else {
        node->buff_data = buff_data;
    }

    node->buff_len = buff_len;
    node->mode = mode;
    node->next = NULL;

    if (cache->tail) {
        cache->tail->next = node;
    } else {
        cache->head = node;
    }
    cache->tail = node;
    cache->count++;
    cache->total_len += buff_len;

    mutex_unlock(cache->mutex);
    return 0;
}

int list_buff_pop(struct list_buff *cache, void **buff_data, unsigned int *buff_len, int free_buff)
{
    if (!cache || !buff_data || !buff_len || !cache->head) return -1;

    if (mutex_lock(cache->mutex) != 0) return -1;

    struct buff_node *node = cache->head;
    *buff_data = node->buff_data;
    *buff_len = node->buff_len;

    cache->head = node->next;
    if (!cache->head) {
        cache->tail = NULL;
    }
    cache->count--;
    cache->total_len -= node->buff_len;

    if (free_buff && node->mode == BUFF_STORE_COPY) {
        free(node->buff_data);
    }

    free(node);

    mutex_unlock(cache->mutex);
    return 0;
}

void list_buff_free_pop_buff(void *buff_data)
{
    if (!buff_data)
        free(buff_data);
}

int list_buff_peek(struct list_buff *cache, void **buff_data, unsigned int *buff_len)
{
    if (!cache || !buff_data || !buff_len || !cache->head) return -1;

    if (mutex_lock(cache->mutex) != 0) return -1;

    *buff_data = cache->head->buff_data;
    *buff_len = cache->head->buff_len;

    mutex_unlock(cache->mutex);
    return 0;
}

unsigned int list_buff_count(struct list_buff *cache)
{
    return cache ? cache->count : 0;
}

unsigned int list_buff_total_len(struct list_buff *cache)
{
    return cache ? cache->total_len : 0;
}

int list_buff_is_empty(struct list_buff *cache)
{
    return cache ? (cache->count == 0) : 1;
}

void list_buff_clear(struct list_buff *cache, void (*free_buff)(void *))
{
    if (!cache) return;

    struct buff_node *node = cache->head;
    while (node) {
        struct buff_node *next = node->next;
		if (node->mode == BUFF_STORE_COPY && free_buff) {
            free_buff(node->buff_data);
        }
        free(node);
        node = next;
    }

    cache->head = NULL;
    cache->tail = NULL;
    cache->count = 0;
    cache->total_len = 0;
}

void list_buff_foreach(struct list_buff *cache,
                     void (*callback)(void *buff_data, unsigned int buff_len, void *user_data),
                     void *user_data)
{
    if (!cache || !callback) return;

    struct buff_node *node = cache->head;
    while (node) {
        callback(node->buff_data, node->buff_len, user_data);
        node = node->next;
    }
}
