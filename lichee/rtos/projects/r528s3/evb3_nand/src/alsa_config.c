/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <aw-alsa-lib/pcm_config.h>
#include <aw-alsa-lib/pcm_plugin.h>

#define SND_PCM_HW_CONFIG(name, device) \
{ \
	.card_name	= #name,  \
	.device_num	= device, \
}

#define DEFINE_SND_PCM_HW_CONFIG(name, device) \
	const snd_pcm_hw_config_t snd_##name##_hw_config = \
				SND_PCM_HW_CONFIG(name, device);

static DEFINE_SND_PCM_HW_CONFIG(audiocodec, 0);
static DEFINE_SND_PCM_HW_CONFIG(snddaudio0, 0);
static DEFINE_SND_PCM_HW_CONFIG(snddaudio1, 0);
static DEFINE_SND_PCM_HW_CONFIG(snddaudio2, 0);
static DEFINE_SND_PCM_HW_CONFIG(snddmic, 0);

static DEFINE_SND_PCM_HW_CONFIG(ac107, 0);
static DEFINE_SND_PCM_HW_CONFIG(tas5805, 0);
static DEFINE_SND_PCM_HW_CONFIG(ak4490, 0);

static const snd_pcm_softvol_config_t snd_pcm_softvol_config = {
	.type		= "softvol",
	.slave = {
		.pcm		= "PlaybackPlug",
	},
	.control = {
		.control_name	= "Soft Volume Master",
		.card_name	= "audiocodec",
	},
	.min_dB			= -51.0,
	.max_dB			= 0.0,
	.resolution		= 256,
};

static const snd_pcm_dsnoop_config_t snd_pcm_dsnoop_config = {
	.type		= "dsnoop",
	.ipc_key	= 1111,
	.slave = {
		.pcm		= "hw:audiocodec",
		.format		= SND_PCM_FORMAT_S16_LE,
		.rate		= 16000,
		.channels	= 3,
		.period_size	= 1024,
		.periods	= 4,
	},
};

static const snd_pcm_dmix_config_t snd_pcm_dmix_config = {
	.type		= "dmix",
	.ipc_key	= 2222,
	.slave = {
		.pcm		= "hw:audiocodec",
		.format		= SND_PCM_FORMAT_S16_LE,
		.rate		= 48000,
		.channels	= 1,
		.period_size	= 1600,
		.periods	= 4,
	},
};

static const snd_pcm_asym_config_t snd_pcm_asym_config = {
	.type		= "asym",
	.playback_pcm	= "PlaybackSoftVol",
	.capture_pcm	= "CaptureDsnoop",
};

static const snd_pcm_route_config_t snd_pcm_route_config = {
	.type = "route",
	.slave = {
		.pcm = "PlaybackRate",
		.channels = 1,
	},
	.ttable = {
		{0, 0, 0.5},
		{1, 0, 0.5},
		TTABLE_CONFIG_END
	},
};

static const snd_pcm_rate_config_t snd_pcm_rate_config = {
	.type		= "rate",
	.slave = {
		.pcm        = "PlaybackSona",
		.format     = SND_PCM_FORMAT_S16_LE,
		.rate       = 48000,
	},
	.converter  = "speexrate",
};

#ifdef CONFIG_AW_ALSA_PLUGINS_SONA_AUDIOAEF
static const snd_pcm_sona_audioaef_config_t snd_pcm_sona_audioaef_config = {
	.type = "sona_audioaef",
	.slave = {
		.pcm = "PlaybackDmix",
	},
	.config_name = "sona_eq",
	.config_file = "/data/sona_eq.txt",
	.max_frames = 0,
	.runtime_config = 0,
	.save_runtime_config = 0,
	.debug_log = 0,
};
#endif

static const snd_pcm_plug_config_t snd_pcm_plug_config = {
	.type           = "plug",
	.slave          = {
		.pcm            = "PlaybackDmix",
		.format         = SND_PCM_FORMAT_S16_LE,
		.channels       = 1,
		.rate           = 48000,
	},
	.rate_converter = "speexrate",
	.route_policy   = "default",
	.ttable         = {
		TTABLE_CONFIG_END
	},
};

static const snd_pcm_file_config_t snd_pcm_file_pb_config = {
	.type 		= "file",
	.slave 		= {
		.pcm 		= "PlaybackDmix",
	},
	.format 	= "raw",
	.mode 		= "adb",
	.port 		= 20190,
};

static const snd_pcm_file_config_t snd_pcm_file_cap_config = {
	.type 		= "file",
	.slave 		= {
		.pcm 		= "CaptureDsnoop",
	},
	.format 	= "raw",
	.mode 		= "adb",
	.port 		= 20191,
};

static const snd_pcm_dsnoop_config_t snd_pcm_dsnoop_ac107_config = {
	.type		= "dsnoop",
	.ipc_key	= 1113,
	.slave = {
		.pcm		= "hw:ac107",
		.format		= SND_PCM_FORMAT_S16_LE,
		.rate		= 16000,
		.channels	= 2,
		.period_size	= 1024,
		.periods	= 4,
	},
};

static const snd_pcm_dsnoop_config_t snd_pcm_dsnoop_3mic_config = {
	.type		= "dsnoop",
	.ipc_key	= 1113,
	.slave = {
		.pcm		= "hw:audiocodec",
		.format		= SND_PCM_FORMAT_S16_LE,
		.rate		= 16000,
		.channels	= 3,
		.period_size	= 1024,
		.periods	= 4,
	},
};

static const snd_pcm_dsnoop_config_t snd_pcm_dsnoop_ref_config = {
	.type		= "dsnoop",
	.ipc_key	= 1114,
	.slave = {
		.pcm		= "hw:ac107",
		.format		= SND_PCM_FORMAT_S16_LE,
		.rate		= 16000,
		.channels	= 1,
		.period_size	= 1024,
		.periods	= 4,
	},
};

static const snd_pcm_multi_config_t snd_pcm_3mic_1ref_config = {
	.type 		= "multi",
	.slaves 	= {
		{ "a", "CaptureDsnoop3Mic", 3 },
		{ "b", "CaptureDsnoopRef", 1 },
		{ NULL, NULL, -1 },
	},
	.bindings 	= {
		{ 0, "a", 0 },
		{ 1, "a", 1 },
		{ 2, "a", 2 },
		{ 3, "b", 0 },
		{ -1, NULL, -1 },
	},
};

static const snd_pcm_file_config_t snd_pcm_file_3mic_1ref_config = {
	.type 		= "file",
	.slave 		= {
		.pcm 		= "Capture3Mic1Ref",
	},
	.format 	= "raw",
	.mode 		= "adb",
	.port 		= 20192,
};

const snd_pcm_config_t snd_pcm_global_configs[] = {
	SND_PCM_CONFIG("CaptureDsnoop", "dsnoop", &snd_pcm_dsnoop_config),
	SND_PCM_CONFIG("PlaybackDmix", "dmix", &snd_pcm_dmix_config),
	SND_PCM_CONFIG("hw:audiocodec", "hw", &snd_audiocodec_hw_config),
	SND_PCM_CONFIG("hw:snddaudio0", "hw", &snd_snddaudio0_hw_config),
	SND_PCM_CONFIG("hw:snddaudio1", "hw", &snd_snddaudio1_hw_config),
	SND_PCM_CONFIG("hw:snddaudio2", "hw", &snd_snddaudio2_hw_config),
	SND_PCM_CONFIG("hw:snddmic", "hw", &snd_snddmic_hw_config),
	SND_PCM_CONFIG("default", "asym", &snd_pcm_asym_config),
	SND_PCM_CONFIG("PlaybackSoftVol", "softvol", &snd_pcm_softvol_config),
	SND_PCM_CONFIG("PlaybackRoute", "route", &snd_pcm_route_config),
	SND_PCM_CONFIG("PlaybackRate", "rate", &snd_pcm_rate_config),
#ifdef CONFIG_AW_ALSA_PLUGINS_SONA_AUDIOAEF
	SND_PCM_CONFIG("PlaybackSona", "sona_audioaef", &snd_pcm_sona_audioaef_config),
#endif
	SND_PCM_CONFIG("PlaybackPlug", "plug", &snd_pcm_plug_config),
	SND_PCM_CONFIG("PlaybackFile", "file", &snd_pcm_file_pb_config),
	SND_PCM_CONFIG("CaptureFile", "file", &snd_pcm_file_cap_config),
	SND_PCM_CONFIG("hw:ac107", "hw", &snd_ac107_hw_config),
	SND_PCM_CONFIG("CaptureDsnoopAc107", "dsnoop", &snd_pcm_dsnoop_ac107_config),
	SND_PCM_CONFIG("CaptureDsnoop3Mic", "dsnoop", &snd_pcm_dsnoop_3mic_config),
	SND_PCM_CONFIG("CaptureDsnoopRef", "dsnoop", &snd_pcm_dsnoop_ref_config),
	SND_PCM_CONFIG("Capture3Mic1Ref", "multi", &snd_pcm_3mic_1ref_config),
	SND_PCM_CONFIG("CaptureFile3Mic1Ref", "file", &snd_pcm_file_3mic_1ref_config),
	SND_PCM_CONFIG("hw:tas5805", "hw", &snd_tas5805_hw_config),
	SND_PCM_CONFIG("hw:ak4490", "hw", &snd_ak4490_hw_config),
};

REGISTER_SND_PCM_GLOBAL_CONFIGS(snd_pcm_global_configs);
