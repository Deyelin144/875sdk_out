# CFLAGS += -I components/common/thirdparty/openh264/decoder/inc/
# CFLAGS += -I components/common/thirdparty/openh264/decoder/wels/

CXXFLAGS += -I components/common/thirdparty/openh264/decoder/inc/
CXXFLAGS += -I components/common/thirdparty/openh264/decoder/wels/
CXXFLAGS += -I components/common/thirdparty/openh264/common/inc
CXXFLAGS += -I components/common/thirdparty/openh264/console/common/inc
CXXFLAGS += -I components/common/thirdparty/openh264/console/dec/inc

DECODER_SRCDIR=decoder/src
DECODER_CPP_SRCS=\
	$(DECODER_SRCDIR)/au_parser.o\
	$(DECODER_SRCDIR)/bit_stream.o\
	$(DECODER_SRCDIR)/cabac_decoder.o\
	$(DECODER_SRCDIR)/deblocking.o\
	$(DECODER_SRCDIR)/decode_mb_aux.o\
	$(DECODER_SRCDIR)/decode_slice.o\
	$(DECODER_SRCDIR)/decoder.o\
	$(DECODER_SRCDIR)/decoder_core.o\
	$(DECODER_SRCDIR)/decoder_data_tables.o\
	$(DECODER_SRCDIR)/error_concealment.o\
	$(DECODER_SRCDIR)/fmo.o\
	$(DECODER_SRCDIR)/get_intra_predictor.o\
	$(DECODER_SRCDIR)/manage_dec_ref.o\
	$(DECODER_SRCDIR)/memmgr_nal_unit.o\
	$(DECODER_SRCDIR)/mv_pred.o\
	$(DECODER_SRCDIR)/parse_mb_syn_cabac.o\
	$(DECODER_SRCDIR)/parse_mb_syn_cavlc.o\
	$(DECODER_SRCDIR)/pic_queue.o\
	$(DECODER_SRCDIR)/rec_mb.o\
	$(DECODER_SRCDIR)/wels_decoder_thread.o\

COMMON_SRCDIR=common
COMMON_CPP_SRCS=\
	$(COMMON_SRCDIR)/src/common_tables.o\
	$(COMMON_SRCDIR)/src/copy_mb.o\
	$(COMMON_SRCDIR)/src/cpu.o\
	$(COMMON_SRCDIR)/src/crt_util_safe_x.o\
	$(COMMON_SRCDIR)/src/deblocking_common.o\
	$(COMMON_SRCDIR)/src/expand_pic.o\
	$(COMMON_SRCDIR)/src/intra_pred_common.o\
	$(COMMON_SRCDIR)/src/mc.o\
	$(COMMON_SRCDIR)/src/memory_align.o\
	$(COMMON_SRCDIR)/src/sad_common.o\
	$(COMMON_SRCDIR)/src/utils.o\
	$(COMMON_SRCDIR)/src/welsCodecTrace.o\
	$(COMMON_SRCDIR)/src/WelsTaskThread.o\
	$(COMMON_SRCDIR)/src/WelsThread.o\
	$(COMMON_SRCDIR)/src/WelsThreadLib.o\
	$(COMMON_SRCDIR)/src/WelsThreadPool.o\

CONSOLE_SRCDIR=console
CONSOLE_CPP_SRCS=\
	$(CONSOLE_SRCDIR)/dec/src/h264dec.o\
	$(CONSOLE_SRCDIR)/dec/src/d3d9_utils.o\
	$(CONSOLE_SRCDIR)/common/src/read_config.o

ifeq ($(CONFIG_COMPONENT_OPENH264_DECODE_DEMO),y)
obj-y += $(COMMON_CPP_SRCS)
obj-y += $(CONSOLE_CPP_SRCS)
endif

obj-y += $(DECODER_CPP_SRCS)
obj-y += decoder/welsDecoderExt.o

