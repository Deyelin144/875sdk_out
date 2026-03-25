obj-y += common/exception.o
obj-y += common/clocksource.o
obj-$(CONFIG_DEBUG_BACKTRACE) += common/backtrace.o

obj-$(CONFIG_ARCH_RISCV_FPU) += fpu/

obj-$(CONFIG_ARCH_RISCV_C906) += c906/

obj-$(CONFIG_ARCH_SUN20IW2) += sun20iw2p1/

CFLAGS-$(CONFIG_ARCH_SUN20IW2) += -I $(BASE)/arch/risc-v/sun20iw2p1/
CFLAGS-$(CONFIG_ARCH_RISCV_C906) += -I $(BASE)/arch/risc-v/c906/

CFLAGS-$(CONFIG_COMPONENTS_BOOT_REASON) += -I $(BASE)/components/common/aw/boot_reason/
CFLAGS-$(CONFIG_COMPONENTS_KALLSYMS) += -I $(BASE)/components/common/aw/kallsyms/include/


ifeq ($(CONFIG_KASAN), y)

CFLAGS_REMOVE_exception.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_clocksource.o += $(CFLAGS_KASAN)
CFLAGS_REMOVE_backtrace.o += $(CFLAGS_KASAN)
endif
