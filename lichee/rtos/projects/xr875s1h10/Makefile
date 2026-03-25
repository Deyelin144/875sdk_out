obj-$(CONFIG_PROJECT_XR875S1H10_PRO_C906) += pro_c906/
obj-$(CONFIG_PROJECT_XR875S1H10_PRO_M33) += pro_m33/

obj-$(CONFIG_PROJECT_SOLUTION_C906) += solution_c906/
obj-$(CONFIG_PROJECT_SOLUTION_M33) += solution_m33/
# recovery与C906共用一套驱动代码
obj-$(CONFIG_PROJECT_BUILD_RECOVERY) += solution_c906/src/drivers/drv_log.o
obj-$(CONFIG_PROJECT_BUILD_RECOVERY) += solution_c906/src/drivers/extio/
obj-$(CONFIG_PROJECT_BUILD_RECOVERY) += solution_c906/src/drivers/battery/
obj-$(CONFIG_PROJECT_BUILD_RECOVERY) += solution_c906/src/drivers/key/
obj-$(CONFIG_PROJECT_BUILD_RECOVERY) += solution_c906/src/drivers/led/
obj-$(CONFIG_PROJECT_BUILD_RECOVERY) += solution_c906/src/drivers/sd/
obj-$(CONFIG_PROJECT_BUILD_RECOVERY) += solution_c906/src/drivers/atcmd/

subdir-ccflags-y += -I components/common/aw/pm/include

ifeq ($(CONFIG_DRIVERS_XRADIO), y)
subdir-ccflags-y += -I components/common/aw/xradio/include/sdd
subdir-ccflags-y += -I drivers/rtos-hal/include/hal
endif
