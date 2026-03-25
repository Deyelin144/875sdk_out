ifndef CONFIG_ARCH_RISCV_C906
obj-y += arm_stub.o
obj-y += breakpoint.o

obj-y += breakpoint_main.o
obj-y += watchpoint_main.o
else
obj-y += riscv_watchpoint_main.o
obj-y += trigger_module.o
endif

obj-$(CONFIG_ARCH_ARM_CORTEX_A7) += hardware_wbp.o
obj-$(CONFIG_ARCH_ARM_CORTEX_M33) += cm33_wbp.o
