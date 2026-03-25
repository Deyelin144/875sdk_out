#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "include/mod_realize/realize_unit_mem/realize_unit_mem.h"

static unit_mem_conf_t s_mem_conf;

void realize_unit_mem_init(unsigned int mem_size)
{
    s_mem_conf.mem_size = mem_size;
}

void *realize_unit_mem_malloc_ex(size_t size, const char *file, const int line, guMemMonitorType_t type)
{
    (void)file;
    (void)line;
    (void)type;
    return malloc(size);
}

void *realize_unit_mem_realloc_ex(void *rmem, size_t newsize, const char *file, const int line, guMemMonitorType_t type)
{
    (void)file;
    (void)line;
    (void)type;
    return realloc(rmem, newsize);
}

void *realize_unit_mem_calloc_ex(int nmemb, size_t size, const char *file, const int line, guMemMonitorType_t type)
{
    (void)file;
    (void)line;
    (void)type;
    return calloc((size_t)nmemb, size);
}

void realize_unit_mem_free_ex(void *p, const char *file, const int line, guMemMonitorType_t type)
{
    (void)file;
    (void)line;
    (void)type;
    free(p);
}

char *realize_unit_mem_strdup_ex(const char *p, const char *file, const int line, guMemMonitorType_t type)
{
    size_t len;
    char *copy;

    (void)file;
    (void)line;
    (void)type;

    if (p == NULL) {
        return NULL;
    }

    len = strlen(p) + 1;
    copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, p, len);
    }

    return copy;
}

void *realize_unit_mem_get_handle(void)
{
    return NULL;
}

int realize_unit_mem_get_block_size(void *p)
{
    (void)p;
    return 0;
}

void realize_unit_mem_deinit(void)
{
    s_mem_conf.mem_size = 0;
}

unsigned int realize_unit_mem_get_size(void)
{
    return s_mem_conf.mem_size;
}

void realize_unit_mem_upd_mem_used(guMemMonitorType_t type, int used_size)
{
    (void)type;
    (void)used_size;
}

unsigned int realize_unit_mem_get_mem_used(guMemMonitorType_t type)
{
    (void)type;
    return 0;
}

int is_pool_mem(void *p)
{
    (void)p;
    return 0;
}

int realize_unit_mem_check(void)
{
    return 0;
}

void realize_unit_mem_set_pool_config(mem_pool_config_t *config)
{
    (void)config;
}

mem_pool_config_t *realize_unit_mem_get_pool_config()
{
    return NULL;
}

void realize_unit_mem_lock()
{
}

void realize_unit_mem_unlock()
{
}

void set_warning_mem_alloc_size(int size)
{
    (void)size;
}
