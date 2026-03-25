ccflags-y += 	-I$(obj)/ \
		-I$(srctree)/include/melis/kernel/drivers

obj-y += clk_core.o
obj-y += clk_source.o
obj-y += clk_fixed_factor.o

obj-y += aw_ccu.o

obj-y += ccu_reset.o

obj-y += ccu_gate.o
obj-y += ccu_mux.o
obj-y += ccu_div.o
obj-y += ccu_multiplier.o
obj-y += ccu_pll.o


ifeq ($(CONFIG_COMPONENTS_PM), y)
CFLAGS += -Icomponents/common/aw/pm/include/
endif
