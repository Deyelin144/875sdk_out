obj-y += sun20i.o
obj-y += cpufreq.o

ifeq ($(CONFIG_KASAN), y)
obj-y += kasan_init.o

CFLAGS_REMOVE_sun20i.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_kasan_init.o += $(CFLAGS_KASAN)
endif
