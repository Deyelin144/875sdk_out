obj-y += head_s.o
obj-y += plic.o
obj-y += cache.o
obj-y += spinlock.o
obj-y += clic.o
obj-y += interrupt.o

ifdef CONFIG_ARCH_RISCV_VECTOR
obj-y += memcpy.o
obj-y += memcpy_vector.o
endif


CFLAGS += -I $(BASE)/arch/risc-v/sun20iw2p1/

ifeq ($(CONFIG_KASAN), y)

CFLAGS_REMOVE_head_s.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_plic.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_cache.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_spinlock.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_clic.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_interrupt.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_amp_threadpool.o += $(CFLAGS_KASAN)
endif
