obj-$(CONFIG_COMPONENTS_AW) += aw/
obj-$(CONFIG_COMPONENTS_AW) += common/aw/
obj-$(CONFIG_COMPONENTS_THIRDPARTY) += thirdparty/
obj-$(CONFIG_COMPONENTS_THIRDPARTY) += common/thirdparty/

subdir-ccflags-y += -I components/common/aw/pm/include/
ifeq ($(CONFIG_DYNAMIC_FTRACE), y)
subdir-ccflags-y += -fpatchable-function-entry=6
endif
subdir-ccflags-y += -DADD_ATTRIBUTE="__attribute__((patchable_function_entry(0, 0)))"
