
obj-y += hal_spi_nor.o
obj-y += spi_nor.o
obj-y += spi_nor_xip.o
obj-y += sfdp.o
obj-y += port_controller.o

obj-$(CONFIG_ACCESS_SPI_NOR_BY_BLKPART) += spi_nor_blkpart.o

CFLAGS += -Wall -Werror

ifeq ($(CONFIG_COMPONENTS_PM), y)
CFLAGS += -Icomponents/common/aw/pm/include/
endif
