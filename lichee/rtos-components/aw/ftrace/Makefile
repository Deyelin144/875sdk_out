obj-$(CONFIG_DYNAMIC_FTRACE) += core.o \
		 cmd.o \
		 tracer.o \
		 port_.o \
		 table.o

obj-$(CONFIG_DYNAMIC_FTRACE) += xr_logger/
obj-$(CONFIG_DYNAMIC_FTRACE_KALLSYMS) += kallsyms/_kallsyms.o \
		 symbol.o \

CFLAGS += -I components/common/aw/ftrace/
CFLAGS += -I components/common/aw/ftrace/include/
CFLAGS += -I components/common/aw/ftrace/cmd/
CFLAGS += -I components/common/aw/ftrace/kallsyms/
CFLAGS += -I components/common/aw/ftrace/xr_logger/

CFLAGS += -D __CONFIG_XRADIO_LOGGER

CFLAGS += -DADD_ATTRIBUTE="__attribute__((patchable_function_entry(0, 0)))"
