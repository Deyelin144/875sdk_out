obj-$(CONFIG_PROJECT_XR875_MINI_M33) += mini_m33/
obj-$(CONFIG_PROJECT_XR875_EVB1_M33) += evb1_m33/
obj-$(CONFIG_PROJECT_XR875_EVB1_C906) += evb1_c906/
obj-$(CONFIG_PROJECT_XR875_EVB1_NAND_M33) += evb1_nand_m33/

subdir-ccflags-y += -I components/common/aw/pm/include

ifeq ($(CONFIG_DRIVERS_XRADIO), y)
subdir-ccflags-y += -I components/common/aw/xradio/include/sdd
subdir-ccflags-y += -I drivers/rtos-hal/include/hal
endif
