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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <console.h>

#include <AudioSystem.h>

#include "wav_parser.h"
#include <hal_thread.h>
#include <hal_time.h>

#define EQT_CAP_BIT       16
#define EQT_CAP_CANNAL    3 // MUST BE 3
#define EQT_CAP_SAMPLE    16000
#define EQT_CAP_PSIZE     1024
#define EQT_CAP_BUFF_SIZE (EQT_CAP_PSIZE * EQT_CAP_CANNAL)

#define CODEC_AC_DHP_ANA_CTRL3           (0x4004B000 + 0x0014)
#define CODEC_ADC3_MIC_MIX               (1 << 30)

#define CODEC_REG_READ_32(reg)           (*(volatile uint32_t *)(reg))
#define CODEC_REG_WRITE_32(reg, value)   (*(volatile uint32_t *)(reg) = (value))

// ******************* gobal ****************
static int s_play_fd = -1;
static int s_cap_fd = -1;
static int s_force_stop = 1;

static void dump_wav_header(wav_header_t *header)
{
	char *ptr = (char *)&header->riffType;
	printf("riffType:     %c%c%c%c\n", ptr[0], ptr[1], ptr[2], ptr[3]);
	ptr = (char *)&header->waveType;
	printf("waveType:     %c%c%c%c\n", ptr[0], ptr[1], ptr[2], ptr[3]);
	printf("channels:     %u\n", header->numChannels);
	printf("rate:         %u\n", header->sampleRate);
	printf("bits:         %u\n", header->bitsPerSample);
	printf("align:        %u\n", header->blockAlign);
	printf("data size:    %u\n", header->dataSize);
}

static int check_wav_header(wav_header_t *header, wav_hw_params_t *hwparams)
{
	if (!header)
		return -1;
	dump_wav_header(header);

	if (header->riffType != WAV_RIFF)
		return -1;
	if (header->waveType != WAV_WAVE)
		return -1;

	hwparams->rate = header->sampleRate;
	hwparams->channels = header->numChannels;
	/* ignore bit endian */
	switch (header->bitsPerSample) {
	case 8:
		hwparams->format = SND_PCM_FORMAT_U8;
		break;
	case 16:
		hwparams->format = SND_PCM_FORMAT_S16_LE;
		break;
	case 24:
		switch (header->blockAlign/header->numChannels) {
			case 4:
				hwparams->format = SND_PCM_FORMAT_S24_LE;
				break;
			case 3:
				/*hwparams->format = SND_PCM_FORMAT_S24_3LE;*/
			default:
				printf("unknown format..\n");
				return -1;
		}
		break;
	case 32:
		hwparams->format = SND_PCM_FORMAT_S32_LE;
		break;
	default:
		printf("unknown sampling depth..\n");
		return -1;
	}

	return 0;
}

static void play_fs_wav(tAudioTrack *at, int count)
{
	wav_header_t wav_header;
	wav_hw_params_t wav_hwparams;
	uint32_t rate, channels;
	snd_pcm_format_t format;
	int size, bits = 16, buf_size;
	uint8_t *buf = NULL;

	read(s_play_fd, &wav_header, sizeof(wav_header_t));

	if (check_wav_header(&wav_header, &wav_hwparams) != 0) {
		printf("check wav header failed\n");
		return;
	}

	rate = wav_hwparams.rate;
	format = wav_hwparams.format;
	channels = wav_hwparams.channels;

	if (format == SND_PCM_FORMAT_S16_LE) {
		bits = 16;
		AudioTrackSetup(at, rate, channels, 16);
	} else if (format == SND_PCM_FORMAT_S32_LE) {
		bits = 32;
		AudioTrackSetup(at, rate, channels, 32);
	}

	buf_size = bits / 8 * channels * rate / 50; /* 20ms buffer */
	buf = malloc(buf_size);
	if (!buf) {
		printf("no memory\n");
		return;
	}

	do {
		uint32_t total = wav_header.dataSize;
		while (total) {
			size = read(s_play_fd, buf, buf_size);
			if (size <= 0)
				break;
			size = AudioTrackWrite(at, (void *)buf, size);
			if (size < 0)
				break;
			total -= size;
		}
	} while(0);
	close(s_play_fd);
	s_play_fd = -1;
	free(buf);

	return;
}

static void at_task(void *arg)
{
	tAudioTrack *at;

	at = AudioTrackCreate("default");
	if (!at) {
		printf("at create failed\n");
	}

	play_fs_wav(at, 1);

	s_force_stop = 0;

	AudioTrackStop(at);
	AudioTrackDestroy(at);

	hal_thread_stop(NULL);
}

// get channal-3 data
static void as_record_get_single_buff(const short *datain, unsigned len, short *dataout)
{
	len /= 6;
	for (int i = 0; i < len; i++) {
		dataout[i] = datain[3 * i + 2];
	}
}

static void ar_task(void *arg)
{
	tAudioRecord  *ar        = NULL;
	unsigned char *ar_buf    = NULL;
	unsigned char *c3_buf    = NULL;
	unsigned int  read_size  = 0;

	ar = AudioRecordCreate("default");
	if (!ar) {
		printf("ar create failed\n");
	}

#if 0
	AudioRecordResampleCtrl(ar, 1);
#endif

	ar_buf = malloc(EQT_CAP_BUFF_SIZE);
	if (!ar_buf) {
		printf("ar_buf malloc fail\n");
	}

	c3_buf = malloc(EQT_CAP_PSIZE);
	if (c3_buf == NULL) {
		printf("c3_buf malloc fail\n");
	}

#if 0
	{
	uint8_t maps[3] = {2, 1, 0};
	AudioRecordChannelMap(ar, maps, sizeof(maps));
	}
#endif
	AudioRecordSetup(ar, EQT_CAP_SAMPLE, EQT_CAP_CANNAL, EQT_CAP_BIT);
	AudioRecordStart(ar);

	unsigned temp_value = CODEC_REG_READ_32(CODEC_AC_DHP_ANA_CTRL3);
	temp_value |= CODEC_ADC3_MIC_MIX;
	CODEC_REG_WRITE_32(CODEC_AC_DHP_ANA_CTRL3, temp_value);

	while (s_force_stop) {
		read_size = AudioRecordRead(ar, ar_buf, EQT_CAP_BUFF_SIZE);
		if (read_size != EQT_CAP_BUFF_SIZE) {
			printf("some err happen!cap stop\n");
			break;
		}
		as_record_get_single_buff((const short *)ar_buf, EQT_CAP_BUFF_SIZE, (short *)c3_buf);
		write(s_cap_fd, c3_buf, EQT_CAP_PSIZE);
	}
	printf("cap stop\n");
	s_force_stop = 1;

	AudioRecordStop(ar);
	AudioRecordDestroy(ar);
	free(ar_buf);
	free(c3_buf);
	close(s_cap_fd);
	s_cap_fd = -1;

	hal_thread_stop(NULL);
}

int cmd_as_eq_test(int argc, char *argv[])
{
	int c = 0;

#ifndef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_EQ
    printf("please open config -- CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_EQ\n");
    return -1;
#endif

	optind = 0;
	printf("play --> %s\n", argv[1]);
	printf("cap --> %s\n", argv[2]);
	s_play_fd = open(argv[1], O_RDONLY);
	if (s_play_fd < 0) {
		printf("play file open fail!\n");
		return -1;
	}

	s_cap_fd = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY);
	if (s_cap_fd < 0) {
		printf("cap file open fail!\n");
		close(s_play_fd);
		s_play_fd = -1;
		return -1;
	}

	s_force_stop = 1;

	hal_thread_create(ar_task, NULL, "eq_cap", 2048, HAL_THREAD_PRIORITY_APP);
	hal_thread_create(at_task, NULL, "eq_pb", 2048, HAL_THREAD_PRIORITY_APP);

	return  0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_as_eq_test, as_eq, audio system eq test);
