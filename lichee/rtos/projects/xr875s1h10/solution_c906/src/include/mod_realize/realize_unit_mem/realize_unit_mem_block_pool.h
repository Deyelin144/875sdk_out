#ifndef __REALIZE_UNIT_MEM_BLOCK_POOL_H__
#define __REALIZE_UNIT_MEM_BLOCK_POOL_H__

#include <stdbool.h>
#include <stdint.h>

#include "../../platform/gulitesf_config.h"

#ifdef CONFIG_MEM_V2_USE_BLOCK_POOL

#define BLOCK_POOL_ON_DEBUG 1

typedef struct {
    short mem_size;
    short block_num;
} mem_block_array_t;

typedef struct {
    mem_block_array_t *block_array;
    int array_size;
    int total_mem_size;
} mem_block_pool_param_t;

void *realize_unit_mem_block_pool_init(mem_block_pool_param_t *block_pool, void *mem, uint32_t size);
int realize_unit_mem_block_pool_deinit(void **handle);
void *realize_unit_mem_block_pool_malloc(void *handle, unsigned int size, unsigned int *block_size);
void realize_unit_mem_block_pool_free(void *handle, void *ptr, unsigned int *block_size);
bool realize_unit_mem_block_pool_check(void *handle, void *ptr);
void *realize_unit_mem_block_pool_realloc(void *handle, void *ptr, unsigned int newsize, unsigned int *old_block_size, unsigned int *new_block_size, void *lock_func);
unsigned int realize_unit_mem_block_pool_get_size(void *handle, void *ptr);

void *realize_unit_mem_block_pool_malloc_aligned(void *handle, unsigned int size, unsigned int *block_size);
void *realize_unit_mem_block_pool_realloc_aligned(void *handle, void *ptr, unsigned int newsize, unsigned int *old_block_size, unsigned int *new_block_size, void *lock_func);

#if BLOCK_POOL_ON_DEBUG
int realize_unit_mem_block_pool_used(void *handle, char *buffer, int buffer_size, int *offset, char disp_info, uint32_t *remaining_memory, uint32_t *biggest_mem_block);
#endif

#endif

#endif