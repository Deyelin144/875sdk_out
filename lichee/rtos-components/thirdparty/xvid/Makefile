XVID_INCLUDE_DIR := components/common/thirdparty/xvid/include/

CFLAGS += -I $(XVID_INCLUDE_DIR)
CFLAGS += -DARCH_IS_GENERIC -DARCH_IS_LITTLE_ENDIAN

ifeq ($(CONFIG_ARCH_RISCV), y)
  CFLAGS += -DARCH_IS_64BIT
else
  CFLAGS += -DARCH_IS_32BIT
endif

# obj-$(CONFIG_COMPONENT_XVID_DECRAW) += xvid_decraw/xvid_decraw.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/xvid.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/decoder.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/dct/idct.o
# obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/dct/fdct.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/bitstream/bitstream.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/bitstream/mbcoding.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/image/image.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/image/colorspace.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/image/font.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/image/postprocessing.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/image/interpolate8x8.o
# obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/image/qpel.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/quant/quant_h263.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/quant/quant_matrix.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/quant/quant_mpeg.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/prediction/mbprediction.o
# obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/motion/gmc.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/motion/sad.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/motion/cbp.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/motion/estimation_common.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/utils/mem_align.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/utils/timer.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/utils/emms.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/utils/mem_transfer.o
obj-$(CONFIG_COMPONENT_XVID_DECRAW) += src/utils/mbtransquant.o
