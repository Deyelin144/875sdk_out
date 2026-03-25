#ifndef ___LIST_H
#define ___LIST_H

#include "config.h"

#if THREAD_STACK_SAVE_BY_LIST
struct list_index {
	struct list_index *next, *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}
#define LIST_HEAD_DEF(name) \
struct list_index name = LIST_HEAD_INIT(name)

#define list_for_each(pos, list_head) \
	for (pos = (list_head)->next; pos != list_head; pos = pos->next)

#ifndef __DEQUALIFY
#define __DEQUALIFY(type, var) ((type)(uintptr_t)(const volatile void *)(var))
#endif

#ifndef offsetof
#define offsetof(type, field) \
	((size_t)(uintptr_t)((const volatile void *)&((type *)0)->field))
#endif

#ifndef __containerof
#define __containerof(ptr, type, field) \
	__DEQUALIFY(type	*, (const volatile char *)(ptr) - offsetof(type, field))
#endif

#ifndef container_of
#define container_of(ptr, type, field) __containerof(ptr, type, field)
#endif

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

ADD_ATTRIBUTE \
static inline void __list_add(struct list_index *new, \
		struct list_index *prev, \
		struct list_index *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

ADD_ATTRIBUTE \
static inline void list_add(struct list_index *new, struct list_index *head)
{
	__list_add(new, head, head->next);
}

ADD_ATTRIBUTE \
static inline void __list_del(struct list_index *prev, struct list_index *next)
{
	next->prev = prev;
	prev->next = next;
}

ADD_ATTRIBUTE \
static inline void list_del(struct list_index *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = entry;
	entry->prev = entry;
}
#endif /* THREAD_STACK_SAVE_BY_LIST */
#endif /* ___LIST_H */
