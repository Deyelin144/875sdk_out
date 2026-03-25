obj-y += hal_flashc.o
obj-y += flash_controller.o
obj-y += platform/

CFLAGS += -Wall -Werror

ifeq ($(CONFIG_COMPONENTS_PM), y)
CFLAGS += -Icomponents/common/aw/pm/include/
endif
