obj-y += osal/
obj-y += rtos-hal/hal/
obj-y += drv/

ifeq ($(CONFIG_DYNAMIC_FTRACE), y)
subdir-ccflags-y += -fpatchable-function-entry=6
endif
subdir-ccflags-y += -DADD_ATTRIBUTE="__attribute__((patchable_function_entry(0, 0)))"
