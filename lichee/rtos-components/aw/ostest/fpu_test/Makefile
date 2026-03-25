obj-$(CONFIG_FPU_CTX_TEST) += fpu_ctx_test.o

ifeq ($(CONFIG_FPU_CTX_TEST), y)
obj-$(CONFIG_ARCH_ARM) += port_arm.o
obj-$(CONFIG_ARCH_RISCV) += port_riscv.o
endif

obj-$(CONFIG_SHOW_C906_FPU_CTX_STATISTICS_INFO) += fpu_ctx_statistics.o
