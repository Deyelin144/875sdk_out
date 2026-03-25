#ifndef DUIKIT_LIST_H
#define DUIKIT_LIST_H

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:    the type of the container struct this is embedded in.
 * @member:    the name of the member within the struct.
 *
 */
#ifndef container_of
#define container_of(ptr, type, member) ({            \
    const typeof(((type *)0)->member) * __mptr = (ptr);    \
    (type *)((char *)__mptr - offsetof(type, member)); })
#endif

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct duikit_list_head {
    struct duikit_list_head *next, *prev;
};

// #define LIST_POISON1  ((void *) 0)
// #define LIST_POISON2  ((void *) 0)

#define DUIKIT_LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct duikit_list_head name = DUIKIT_LIST_HEAD_INIT(name)

static inline void DUIKIT_INIT_LIST_HEAD(struct duikit_list_head *list)
{
    list->next = list;
    list->prev = list;
}

/*
 * Insert a _new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
#ifndef CONFIG_DEBUG_LIST
static inline void __duikit_list_add(struct duikit_list_head *_new,
                  struct duikit_list_head *prev,
                  struct duikit_list_head *next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}
#else
extern void __duikit_list_add(struct duikit_list_head *_new,
                  struct duikit_list_head *prev,
                  struct duikit_list_head *next);
#endif

/**
 * duikit_list_add - add a _new entry
 * @_new: _new entry to be added
 * @head: list head to add it after
 *
 * Insert a _new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void duikit_list_add(struct duikit_list_head *_new, struct duikit_list_head *head)
{
    __duikit_list_add(_new, head, head->next);
}


/**
 * duikit_list_add_tail - add a _new entry
 * @_new: _new entry to be added
 * @head: list head to add it before
 *
 * Insert a _new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void duikit_list_add_tail(struct duikit_list_head *_new, struct duikit_list_head *head)
{
    __duikit_list_add(_new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __duikit_list_del(struct duikit_list_head * prev, struct duikit_list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * duikit_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: duikit_list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
#ifndef CONFIG_DEBUG_LIST
static inline void __duikit_list_del_entry(struct duikit_list_head *entry)
{
    __duikit_list_del(entry->prev, entry->next);
}

static inline void duikit_list_del(struct duikit_list_head *entry)
{
    __duikit_list_del(entry->prev, entry->next);
    // entry->next = LIST_POISON1;
    // entry->prev = LIST_POISON2;
}
#else
extern void __duikit_list_del_entry(struct duikit_list_head *entry);
extern void duikit_list_del(struct duikit_list_head *entry);
#endif

/**
 * duikit_list_replace - replace old entry by _new one
 * @old : the element to be replaced
 * @_new : the _new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void duikit_list_replace(struct duikit_list_head *old,
                struct duikit_list_head *_new)
{
    _new->next = old->next;
    _new->next->prev = _new;
    _new->prev = old->prev;
    _new->prev->next = _new;
}

static inline void duikit_list_replace_init(struct duikit_list_head *old,
                    struct duikit_list_head *_new)
{
    duikit_list_replace(old, _new);
    DUIKIT_INIT_LIST_HEAD(old);
}

/**
 * duikit_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void duikit_list_del_init(struct duikit_list_head *entry)
{
    __duikit_list_del_entry(entry);
    DUIKIT_INIT_LIST_HEAD(entry);
}

/**
 * duikit_list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void duikit_list_move(struct duikit_list_head *list, struct duikit_list_head *head)
{
    __duikit_list_del_entry(list);
    duikit_list_add(list, head);
}

/**
 * duikit_list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void duikit_list_move_tail(struct duikit_list_head *list,
                  struct duikit_list_head *head)
{
    __duikit_list_del_entry(list);
    duikit_list_add_tail(list, head);
}

/**
 * duikit_list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int duikit_list_is_last(const struct duikit_list_head *list,
                const struct duikit_list_head *head)
{
    return list->next == head;
}

/**
 * duikit_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int duikit_list_empty(const struct duikit_list_head *head)
{
    return head->next == head;
}

/**
 * duikit_list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using duikit_list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is duikit_list_del_init(). Eg. it cannot be used
 * if another CPU could re-duikit_list_add() it.
 */
static inline int duikit_list_empty_careful(const struct duikit_list_head *head)
{
    struct duikit_list_head *next = head->next;
    return (next == head) && (next == head->prev);
}

/**
 * duikit_list_rotate_left - rotate the list to the left
 * @head: the head of the list
 */
static inline void duikit_list_rotate_left(struct duikit_list_head *head)
{
    struct duikit_list_head *first;

    if (!duikit_list_empty(head)) {
        first = head->next;
        duikit_list_move_tail(first, head);
    }
}

/**
 * duikit_list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int duikit_list_is_singular(const struct duikit_list_head *head)
{
    return !duikit_list_empty(head) && (head->next == head->prev);
}

static inline void __duikit_list_cut_position(struct duikit_list_head *list,
        struct duikit_list_head *head, struct duikit_list_head *entry)
{
    struct duikit_list_head *new_first = entry->next;
    list->next = head->next;
    list->next->prev = list;
    list->prev = entry;
    entry->next = list;
    head->next = new_first;
    new_first->prev = head;
}

/**
 * duikit_list_cut_position - cut a list into two
 * @list: a _new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *    and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void duikit_list_cut_position(struct duikit_list_head *list,
        struct duikit_list_head *head, struct duikit_list_head *entry)
{
    if (duikit_list_empty(head))
        return;
    if (duikit_list_is_singular(head) &&
        (head->next != entry && head != entry))
        return;
    if (entry == head)
        DUIKIT_INIT_LIST_HEAD(list);
    else
        __duikit_list_cut_position(list, head, entry);
}

static inline void __duikit_list_splice(const struct duikit_list_head *list,
                 struct duikit_list_head *prev,
                 struct duikit_list_head *next)
{
    struct duikit_list_head *first = list->next;
    struct duikit_list_head *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/**
 * duikit_list_splice - join two lists, this is designed for stacks
 * @list: the _new list to add.
 * @head: the place to add it in the first list.
 */
static inline void duikit_list_splice(const struct duikit_list_head *list,
                struct duikit_list_head *head)
{
    if (!duikit_list_empty(list))
        __duikit_list_splice(list, head, head->next);
}

/**
 * duikit_list_splice_tail - join two lists, each list being a queue
 * @list: the _new list to add.
 * @head: the place to add it in the first list.
 */
static inline void duikit_list_splice_tail(struct duikit_list_head *list,
                struct duikit_list_head *head)
{
    if (!duikit_list_empty(list))
        __duikit_list_splice(list, head->prev, head);
}

/**
 * duikit_list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the _new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void duikit_list_splice_init(struct duikit_list_head *list,
                    struct duikit_list_head *head)
{
    if (!duikit_list_empty(list)) {
        __duikit_list_splice(list, head, head->next);
        DUIKIT_INIT_LIST_HEAD(list);
    }
}

/**
 * duikit_list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the _new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void duikit_list_splice_tail_init(struct duikit_list_head *list,
                     struct duikit_list_head *head)
{
    if (!duikit_list_empty(list)) {
        __duikit_list_splice(list, head->prev, head);
        DUIKIT_INIT_LIST_HEAD(list);
    }
}

/**
 * duikit_list_entry - get the struct for this entry
 * @ptr:    the &struct duikit_list_head pointer.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the duikit_list_struct within the struct.
 */
#define duikit_list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * duikit_list_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define duikit_list_first_entry(ptr, type, member) \
    duikit_list_entry((ptr)->next, type, member)

/**
 * duikit_list_for_each    -    iterate over a list
 * @pos:    the &struct duikit_list_head to use as a loop cursor.
 * @head:    the head for your list.
 */
#define duikit_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * __duikit_list_for_each    -    iterate over a list
 * @pos:    the &struct duikit_list_head to use as a loop cursor.
 * @head:    the head for your list.
 *
 * This variant doesn't differ from duikit_list_for_each() any more.
 * We don't do prefetching in either case.
 */
#define __duikit_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * duikit_list_for_each_prev    -    iterate over a list backwards
 * @pos:    the &struct duikit_list_head to use as a loop cursor.
 * @head:    the head for your list.
 */
#define duikit_list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * duikit_list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the &struct duikit_list_head to use as a loop cursor.
 * @n:        another &struct duikit_list_head to use as temporary storage
 * @head:    the head for your list.
 */
#define duikit_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

/**
 * duikit_list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:    the &struct duikit_list_head to use as a loop cursor.
 * @n:        another &struct duikit_list_head to use as temporary storage
 * @head:    the head for your list.
 */
#define duikit_list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; \
         pos != (head); \
         pos = n, n = pos->prev)

/**
 * duikit_list_for_each_entry    -    iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 */
#define duikit_list_for_each_entry(pos, head, member)                          \
    for (pos = duikit_list_entry((head)->next, typeof(*pos), member);          \
         &pos->member != (head);    \
         pos = duikit_list_entry(pos->member.next, typeof(*pos), member))

/**
 * duikit_list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 */
#define duikit_list_for_each_entry_reverse(pos, head, member)                  \
    for (pos = duikit_list_entry((head)->prev, typeof(*pos), member);          \
         &pos->member != (head);    \
         pos = duikit_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * duikit_list_prepare_entry - prepare a pos entry for use in duikit_list_for_each_entry_continue()
 * @pos:    the type * to use as a start point
 * @head:    the head of the list
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in duikit_list_for_each_entry_continue().
 */
#define duikit_list_prepare_entry(pos, head, member)                           \
    ((pos) ? : duikit_list_entry(head, typeof(*pos), member))

/**
 * duikit_list_for_each_entry_continue - continue iteration over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define duikit_list_for_each_entry_continue(pos, head, member)                 \
    for (pos = duikit_list_entry(pos->member.next, typeof(*pos), member);      \
         &pos->member != (head);                                        \
         pos = duikit_list_entry(pos->member.next, typeof(*pos), member))

/**
 * duikit_list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define duikit_list_for_each_entry_continue_reverse(pos, head, member)         \
    for (pos = duikit_list_entry(pos->member.prev, typeof(*pos), member);      \
         &pos->member != (head);                                        \
         pos = duikit_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * duikit_list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define duikit_list_for_each_entry_from(pos, head, member)                     \
    for (; &pos->member != (head);                                      \
         pos = duikit_list_entry(pos->member.next, typeof(*pos), member))

/**
 * duikit_list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 */
#define duikit_list_for_each_entry_safe(pos, n, head, member)                  \
    for (pos = duikit_list_entry((head)->next, typeof(*pos), member),          \
        n = duikit_list_entry(pos->member.next, typeof(*pos), member);         \
         &pos->member != (head);                                        \
         pos = n, n = duikit_list_entry(n->member.next, typeof(*n), member))

/**
 * duikit_list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define duikit_list_for_each_entry_safe_continue(pos, n, head, member)         \
    for (pos = duikit_list_entry(pos->member.next, typeof(*pos), member),      \
        n = duikit_list_entry(pos->member.next, typeof(*pos), member);         \
         &pos->member != (head);                        \
         pos = n, n = duikit_list_entry(n->member.next, typeof(*n), member))

/**
 * duikit_list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define duikit_list_for_each_entry_safe_from(pos, n, head, member)             \
    for (n = duikit_list_entry(pos->member.next, typeof(*pos), member);        \
         &pos->member != (head);                        \
         pos = n, n = duikit_list_entry(n->member.next, typeof(*n), member))

/**
 * duikit_list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define duikit_list_for_each_entry_safe_reverse(pos, n, head, member)          \
    for (pos = duikit_list_entry((head)->prev, typeof(*pos), member),          \
        n = duikit_list_entry(pos->member.prev, typeof(*pos), member);         \
         &pos->member != (head);                                        \
         pos = n, n = duikit_list_entry(n->member.prev, typeof(*n), member))

/**
 * duikit_list_safe_reset_next - reset a stale duikit_list_for_each_entry_safe loop
 * @pos:    the loop cursor used in the duikit_list_for_each_entry_safe loop
 * @n:        temporary storage used in duikit_list_for_each_entry_safe
 * @member:    the name of the duikit_list_struct within the struct.
 *
 * duikit_list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and duikit_list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define duikit_list_safe_reset_next(pos, n, member)                \
    n = duikit_list_entry(pos->member.next, typeof(*pos), member)

#endif
