# ----------------------------------------------------------------------------
# common rules
# ----------------------------------------------------------------------------
ROOT_PATH := ../../../../..

include $(ROOT_PATH)/gcc.mk

# ----------------------------------------------------------------------------
# library and objects
# ----------------------------------------------------------------------------
LIBS = libaecm.a

DIRS := .

SRCS_1 = \
        aecm_core.c\
        aecm_core_c.c\
        echo_control_mobile.c\
        ../utility/delay_estimator_wrapper.c\
        ../utility/delay_estimator.c\
        ../utility/ring_buffer.c\
        ../../../common_audio/signal_processing/randomization_functions.c\
        ../../../common_audio/signal_processing/division_operations.c\
        ../../../common_audio/signal_processing/min_max_operations.c\
        ../../../common_audio/signal_processing/cross_correlation.c\
        ../../../common_audio/signal_processing/downsample_fast.c\
        ../../../common_audio/signal_processing/vector_scaling_operations.c\
        ../../../common_audio/signal_processing/real_fft.c\
        ../../../common_audio/signal_processing/complex_bit_reverse.c\
        ../../../common_audio/signal_processing/complex_fft.c\
        ../../../common_audio/signal_processing/spl_sqrt_floor.c\
        ../../../common_audio/signal_processing/spl_init.c

SRCS = $(basename $(SRCS_1))
OBJS = $(addsuffix .o,$(SRCS))

# CC_FLAGS += -D_GNU_SOURCE
# CC_FLAGS += -DWEBRTC_POSIX
CC_FLAGS += -DWEBRTC_ARCH_ARM_V7

# library make rules
include $(LIB_MAKE_RULES)
