
obj-y += sun20iw2_src_ccu.o
obj-y += sun20iw2_aon_ccu.o
obj-y += sun20iw2_gprcm_ccu.o
obj-y += sun20iw2_app_ccu.o

ccflags-y += -I drivers/rtos-hal/hal/source/ccu/clk_core/
ifeq ($(CONFIG_ARCH_DSP), y)
CFLAGS += -I drivers/rtos-hal/hal/source/ccu/clk_core/
endif
