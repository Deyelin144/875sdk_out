#ifndef _LINUX_KASAN_H
#define _LINUX_KASAN_H

#ifdef CONFIG_KASAN

#define KASAN_SHADOW_SCALE_SHIFT (3)

#ifdef CONFIG_ARCH_RISCV_C906

#ifdef CONFIG_KASAN_SHADOW_LENGTH
#define KASAN_SHADOW_SIZE   CONFIG_KASAN_SHADOW_LENGTH
#else
#define KASAN_SHADOW_SIZE   (CONFIG_ARCH_MEM_LENGTH>>3)
#endif

#define KASAN_SHADOW_START      (CONFIG_ARCH_START_ADDRESS + CONFIG_ARCH_MEM_LENGTH - KASAN_SHADOW_SIZE)
#else
#define VA_BITS 32
#define KASAN_SHADOW_SIZE   (1 << (VA_BITS - 3))
#define VA_START (0x20000000L)
#define KASAN_SHADOW_START      (VA_START)
#endif

#define KASAN_SHADOW_END        ((unsigned long)KASAN_SHADOW_START + (unsigned long)KASAN_SHADOW_SIZE)

#undef likely
#undef unlikely
#define  likely(x)        __builtin_expect(!!(x), 1)
#define  unlikely(x)      __builtin_expect(!!(x), 0)

/*
 * This value is used to map an address to the corresponding shadow
 * address by the following formula:
 *     shadow_addr = (address >> 3) + KASAN_SHADOW_OFFSET;
 *
 * (1 << 61) shadow addresses - [KASAN_SHADOW_OFFSET,KASAN_SHADOW_END]
 * cover all 64-bits of virtual addresses. So KASAN_SHADOW_OFFSET
 * should satisfy the following equation:
 *      KASAN_SHADOW_OFFSET = KASAN_SHADOW_END - (1ULL << 61)
 */
#ifdef CONFIG_KASAN_SHADOW_OFFSET
#define KASAN_SHADOW_OFFSET     CONFIG_KASAN_SHADOW_OFFSET
#else
#define KASAN_SHADOW_OFFSET   (KASAN_SHADOW_START - (CONFIG_ARCH_START_ADDRESS>>3))
#endif

void kasan_init(void);
void kasan_early_init(void);
void kasan_init_report(void);

#ifdef CONFIG_ARCH_RISCV_C906
void rt_malloc_small_func_hook(void *ptr, uint32_t size);
void rt_free_small_func_hook(void *ptr, uint32_t size);
#else
void rt_page_alloc_func_hook(void *ptr, size_t pages);
void rt_page_free_func_hook(void *ptr, size_t pages);
void rt_malloc_large_func_hook(void *ptr, size_t size);
void rt_free_large_func_hook(void *ptr, size_t size);
void rt_malloc_small_func_hook(void *ptr, size_t size);
void rt_free_small_func_hook(void *ptr, size_t size);
void rt_realloc_small_func_hook(void *ptr, size_t size);
void rt_free_func_hook(void *ptr);
#endif

void kasan_populate_zero_shadow(const void *shadow_start,
                                const void *shadow_end);

static inline void *kasan_mem_to_shadow(const void *addr)
{
    return (void *)((unsigned long)addr >> KASAN_SHADOW_SCALE_SHIFT)
           + KASAN_SHADOW_OFFSET;
}

#else /* CONFIG_KASAN */


#endif /* CONFIG_KASAN */

#endif /* LINUX_KASAN_H */
