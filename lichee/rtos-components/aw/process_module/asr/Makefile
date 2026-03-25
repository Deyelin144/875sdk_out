ifeq ($(CONFIG_ARCH_ARM_CORTEX_A7),y)
#obj-y += libaec_a7.o
else ifeq ($(CONFIG_ARCH_RISCV_RV64),y)
#obj-y += libaec_rv64.o
else ifeq ($(CONFIG_ARCH_DSP),y)
#TODO hifi4,hifi5 difference
ifeq ($(CONFIG_XTENSA_HIFI5),y)
obj-y += hifi5/libuvoice_sdk.o
obj-y += hifi5/libuvoice_alg.o
else ifeq ($(CONFIG_XTENSA_HIFI4),y)
#obj-y += libaec_hifi4.o
endif
endif
CFLAGS +=  -I components/common/aw/process_module/asr/include

