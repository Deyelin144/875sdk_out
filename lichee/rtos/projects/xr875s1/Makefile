obj-$(CONFIG_PROJECT_XR875S1_PRO_HP_C906) += pro_hp_c906/
obj-$(CONFIG_PROJECT_XR875S1_PRO_HP_M33) += pro_hp_m33/

subdir-ccflags-y += -I components/common/aw/pm/include

ifeq ($(CONFIG_DRIVERS_XRADIO), y)
subdir-ccflags-y += -I components/common/aw/xradio/include/sdd
subdir-ccflags-y += -I drivers/rtos-hal/include/hal
endif
