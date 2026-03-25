
WEBRTC_COMMON = modules/audio_processing/utility/ring_buffer.o \
	modules/audio_processing/utility/delay_estimator_wrapper.o \
	modules/audio_processing/utility/delay_estimator.o \
	modules/audio_processing/utility/fft4g.o \
	common_audio/signal_processing/randomization_functions.o \
	port/glue.o

WEBRTC_AEC = modules/audio_processing/aec/aec_core.o \
	modules/audio_processing/aec/aec_rdft.o \
	modules/audio_processing/aec/aec_resampler.o \
	modules/audio_processing/aec/echo_cancellation.o

WEBRTC_NS = modules/audio_processing/ns/noise_suppression_x.o \
	modules/audio_processing/ns/noise_suppression.o \
	modules/audio_processing/ns/ns_core.o \
	modules/audio_processing/ns/nsx_core.o

WEBRTC_VAD = common_audio/vad/webrtc_vad.o \
	common_audio/vad/vad_core.o \
	common_audio/vad/vad_sp.o \
	common_audio/vad/vad_gmm.o \
	common_audio/vad/vad_filterbank.o \
	common_audio/signal_processing/get_scaling_square.o \
	common_audio/signal_processing/energy.o \
	common_audio/signal_processing/division_operations.o \
	common_audio/signal_processing/spl_init.o \
	common_audio/signal_processing/real_fft.o \
	common_audio/signal_processing/complex_fft.o \
	common_audio/signal_processing/complex_bit_reverse.o \
	common_audio/signal_processing/vector_scaling_operations.o \
	common_audio/signal_processing/downsample_fast.o \
	common_audio/signal_processing/cross_correlation.o \
	common_audio/signal_processing/min_max_operations.o \
	common_audio/signal_processing/resample_48khz.o \
	common_audio/signal_processing/resample_by_2_internal.o \
	common_audio/signal_processing/resample_fractional.o

WEBRTC_AGC = modules/audio_processing/agc/analog_agc.o \
	modules/audio_processing/agc/digital_agc.o \
	common_audio/signal_processing/division_operations.o \
	common_audio/signal_processing/copy_set_operations.o \
	common_audio/signal_processing/resample_by_2.o \
	common_audio/signal_processing/spl_sqrt.o \
	common_audio/signal_processing/spl_init.o \

obj-$(CONFIG_COMPONENTS_WEBRTC) += $(WEBRTC_COMMON)
obj-$(CONFIG_COMPONENTS_WEBRTC_AEC) += $(WEBRTC_AEC)
obj-$(CONFIG_COMPONENTS_WEBRTC_NS) += $(WEBRTC_NS)
obj-$(CONFIG_COMPONENTS_WEBRTC_VAD) += $(WEBRTC_VAD)
obj-$(CONFIG_COMPONENTS_WEBRTC_AGC) += $(WEBRTC_AGC)

CFLAGS +=  -I components/common/thirdparty/