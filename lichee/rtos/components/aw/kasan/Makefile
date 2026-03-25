obj-y += kasan.o
obj-y += freertos.o
#obj-y += hexdump.o
obj-y += report.o
obj-y += kasan_test.o

#CFLAGS += -fno-conserve-stack -fno-stack-protector

CFLAGS_REMOVE_kasan.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_freertos.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_report.o += $(CFLAGS_KASAN)
