CFLAGS += -I components/common/thirdparty/sonic/inc
CFLAGS += -Werror

ifeq ($(CONFIG_SONIC_LITE),y)
obj-y += src/sonic_lite.o
CFLAGS+=-D__CONFIG_SONIC_LITE
else
obj-y += src/sonic.o
CFLAGS+=-D__CONFIG_SONIC
endif

obj-y += src/sonic_glue.o

ifeq ($(CONFIG_SONIC_EXAMPLES),y)
obj-y += example/sonic_example.o
CFLAGS += -I components/common/aw/AudioSystem/include
CFLAGS += -I components/common/thirdparty/sonic/
CFLAGS += -I components/common/aw/ringbuffer/include
CFLAGS += -I drivers/rtos-hal/hal/aw-tiny-sound-lib
CFLAGS += -I drivers/rtos-hal/hal/source/sound/driver_v2/component/
endif
