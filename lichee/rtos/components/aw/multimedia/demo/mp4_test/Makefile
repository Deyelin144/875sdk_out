MP4_CUSTOMER_FILE=cdx_interface/cdx_audio_decoder.o \
					cdx_interface/cdx_customer_stream.o \
					cdx_interface/cdx_audio_rendor.o \
					cdx_interface/cdx_video_rendor.o

ifeq ($(CONFIG_COMPONENT_XVID_DECRAW), y)
MP4_CUSTOMER_FILE += cdx_interface/cdx_video_dec.o
CFLAGS += -I components/common/thirdparty/xvid/include
else
ifneq ($(CONFIG_AMP_XVID_STUB), y)
MP4_CUSTOMER_FILE += cdx_interface/cdx_video_dec_avc.o
endif
endif

MP4_CORE_FILE=video_core/video_fsm.o \
				video_core/video_message.o \
				video_core/video_stack.o \
				video_core/aac_packet.o \
				video_core/video_command.o

MP4_PARSER_FILE=mp4_parser/mp4.o \
				mp4_parser/stream.o

MP4_DEMO_FILE=video_demo.o

obj-y += $(MP4_CUSTOMER_FILE) \
			$(MP4_CORE_FILE) \
			$(MP4_DEMO_FILE)

CFLAGS += -I include/
CFLAGS += -I drivers/rtos-hal/include/hal/
CFLAGS += -I components/common/thirdparty/libjpeg-turbo/source/
CFLAGS += -I components/common/thirdparty/libjpeg-turbo/include/
CFLAGS += -I components/common/aw/AudioSystem/include
CFLAGS += -I components/aw/multimedia/include
CFLAGS += -I components/aw/multimedia/include/include/cedarx/xplayer/include
CFLAGS += -I components/aw/multimedia/include/libcore/base/include
CFLAGS += -I components/aw/multimedia/include/libcore/playback/include
CFLAGS += -I components/aw/multimedia/include/Cdx2.0Plugin/include
CFLAGS += -I components/aw/multimedia/include/libcore/parser/include
CFLAGS += -I components/aw/multimedia/include/libcore/stream/include
CFLAGS += -I components/aw/multimedia/demo/mp4_test
CFLAGS += -I components/aw/multimedia/demo/mp4_test/cdx_interface
CFLAGS += -I components/common/aw/ringbuffer/include
CFLAGS += -I drivers/rtos-hal/hal/source/g2d_rcq
CXXFLAGS += -I components/common/thirdparty/openh264/decoder
CXXFLAGS += -I components/common/thirdparty/openh264/decoder/wels
CXXFLAGS += -I components/common/thirdparty/openh264/decoder/inc
CXXFLAGS += -I components/common/thirdparty/openh264/common/inc

CFLAGS += -Werror
