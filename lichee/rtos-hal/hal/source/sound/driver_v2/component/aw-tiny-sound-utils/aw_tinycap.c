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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <hal_cmd.h>
#include <sunxi_hal_sound.h>
#include "common.h"
#include "wav_parser.h"
#include <hal_time.h>
#include <hal_timer.h>
#include <aw-tiny-sound-lib/sunxi_tiny_sound_pcm.h>

static unsigned int g_capture_loop_enable = 0;
static unsigned int g_capture_then_play = 0;
static int g_play_card_num = 0;
static int g_play_device_num = 0;
static int  g_write_file_last_enable = 0;

extern unsigned int g_aw_verbose;
extern int awtinyplay(unsigned int card, unsigned int device, snd_pcm_format_t format, unsigned int rate,
			unsigned int channels, const char *data, unsigned int datalen);

/*
 * arg0: arecord
 * arg1: card
 * arg2: format
 * arg3: rate
 * arg4: channels
 * arg5: data
 * arg6: len
 */
static int awtinycap(unsigned int card, unsigned int device, snd_pcm_format_t format, unsigned int rate,
		   unsigned int channels, const void *data, unsigned int datalen)
{
	int ret = 0;
	struct sunxi_pcm *handle;
	int mode = 0;
	snd_pcm_uframes_t period_frames = 1024, buffer_frames = 4096;

	printf("dump args:\n");
	printf("card:%u device:%u\n", card, device);
	printf("format:    %u\n", format);
	printf("rate:      %u\n", rate);
	printf("channels:  %u\n", channels);
	printf("data:      %p\n", data);
	printf("datalen:   %u\n", datalen);

	/* open card */
	ret = sunxi_pcm_open(&handle, card, device, SND_PCM_STREAM_CAPTURE, mode);
	if (ret < 0) {
		printf("audio open error:%d\n", ret);
		return -1;
	}

	ret = sunxi_set_param(handle, format, rate, channels, period_frames, buffer_frames);
	if (ret < 0)
		goto err1;

	ret = sunxi_pcm_prepare(handle);
	if (ret < 0) {
		printf("sunxi_pcm_prepare failed ret %d!\n", ret);
		goto err1;
	}

	do {
		printf("sunxi_pcm_read start...\n");
		ret = sunxi_pcm_read(handle, data,
			sunxi_sound_pcm_bytes_to_frames(handle, datalen),
			sunxi_sound_pcm_frames_to_bytes(handle, 1));
		if (ret < 0) {
			printf("capture error:%d\n", ret);
			goto err1;
		}
	} while (g_capture_loop_enable);

	ret = sunxi_pcm_drain(handle);
	if (ret < 0)
		printf("stop failed!, return %d\n", ret);

err1:
	/* close card */
	ret = sunxi_pcm_close(handle);
	if (ret < 0) {
		printf("audio close error:%d\n", ret);
		return ret;
	}

	return ret;
}

static int tiny_cap_then_play(sound_mgr_t *sound_mgr)
{
	char *capture_data = NULL;
	unsigned int len = 0;

	printf("duration %u s\n", sound_mgr->capture_duration);

	if (sound_mgr->capture_duration == 0)
		sound_mgr->capture_duration = 5;

	len = sunxi_sound_pcm_format_size(sound_mgr->format,
			sound_mgr->capture_duration * sound_mgr->rate * sound_mgr->channels);
	capture_data = malloc(len);
	if (!capture_data) {
		printf("no memory\n");
		return -1;
	}

	do {
		memset(capture_data, 0, len);

		printf("awtinycap start...\n");
		awtinycap(sound_mgr->card, sound_mgr->device, sound_mgr->format, sound_mgr->rate,
					sound_mgr->channels, capture_data, len);
		if (g_capture_then_play) {
			printf("awtinyplay start...\n");
			/*snd_ctl_set("audiocodec", "LINEOUT volume", 0x1f);*/
			awtinyplay(g_play_card_num, g_play_device_num, sound_mgr->format, sound_mgr->rate,
					sound_mgr->channels, capture_data, len);
		}
	} while (g_capture_loop_enable);

	free(capture_data);
	capture_data = NULL;

	return 0;
}

static int tiny_capture_fs_wav(sound_mgr_t *mgr, const char *path)
{
	int ret = 0, fd = 0, write_size = 0;
	sunxi_sound_wav_header_t header;
	unsigned int written = 0;
	long rest = -1, c = 0;
	char *audiobuf = NULL;
	unsigned int chunk_bytes, frame_bytes = 0;
	int save_fs = 0;
	struct stat statbuf;
	char *temp_wav_file = NULL;
	unsigned int temp_wav_file_off = 0;

	printf("card:%u device:%u\n", mgr->card, mgr->device);
	printf("period_size:	%ld\n", mgr->period_size);
	printf("buffer_size:	%ld\n", mgr->buffer_size);
	printf("duration %u s\n", mgr->capture_duration);

	if (mgr->capture_duration == 0)
		mgr->capture_duration = 5;

	if (path != NULL)
		save_fs = 1;
	/* open card */
	ret = sunxi_pcm_open(&mgr->handle, mgr->card, mgr->device, SND_PCM_STREAM_CAPTURE, 0);
	if (ret < 0) {
		printf("audio open error:%d\n", ret);
		return -1;
	}

	ret = sunxi_set_param(mgr->handle, mgr->format, mgr->rate, mgr->channels,
			mgr->period_size, mgr->buffer_size);
	if (ret < 0)
		goto err;

	ret = sunxi_pcm_prepare(mgr->handle);
	if (ret < 0) {
		printf("sunxi_pcm_prepare failed ret %d!\n", ret);
		goto err;
	}

	frame_bytes = sunxi_sound_pcm_frames_to_bytes(mgr->handle, 1);
	chunk_bytes = sunxi_sound_pcm_frames_to_bytes(mgr->handle, mgr->period_size);
	if (mgr->capture_duration > 0)
		rest = mgr->capture_duration * sunxi_sound_pcm_frames_to_bytes(mgr->handle, mgr->rate);

	sunxi_sound_create_wav(&header, mgr->format, mgr->rate, mgr->channels);
	if (save_fs) {
		if (!stat(path, &statbuf)) {
			if (S_ISREG(statbuf.st_mode))
				remove(path);
		}
		fd = open(path, O_RDWR | O_CREAT, 0644);
		if (fd < 0) {
			printf("create wav file failed\n");
			goto err;
		}
		write(fd, &header, sizeof(header));
	}

	audiobuf = malloc(chunk_bytes);
	if (!audiobuf) {
		printf("no memory...\n");
		goto err;
	}

	if (g_write_file_last_enable) {
		if (rest < 0) {
			printf("please set capture duration..\n");
			goto err;
		}
		temp_wav_file = malloc(rest);
		if (!temp_wav_file) {
			printf("no memory for temp_wav_file\n");
			goto err;
		}
	}

	if (save_fs && fd > 0) {
		if (rest < 0) {
			printf("please set capture duration..\n");
			goto err;
		}
	}
	while ((rest > 0 || g_capture_loop_enable) && !mgr->in_aborting) {
		long f = mgr->period_size;
		if (rest <= chunk_bytes && !g_capture_loop_enable)
			c = rest;
		else
			c = chunk_bytes;
		f = sunxi_pcm_read(mgr->handle, audiobuf, f, frame_bytes);
		if (f < 0) {
			printf("pcm read error, return %ld\n", f);
			break;
		}
		if (save_fs && fd > 0 && !g_write_file_last_enable) {
			ret = write(fd, audiobuf, c);
			if (ret != c) {
				printf("write audiobuf to wav file failed, return %d\n", ret);
				goto err;
			}
		}
		if (g_write_file_last_enable) {
			memcpy(temp_wav_file + temp_wav_file_off, audiobuf, c);
			temp_wav_file_off += c;
		}
		if (rest > 0)
			rest -= c;
		written += c;
	}

	ret = sunxi_pcm_drain(mgr->handle);
	if (ret < 0)
		printf("stop failed!, return %d\n", ret);

err:
	/* close card */
	ret = sunxi_pcm_close(mgr->handle);
	if (ret < 0) {
		printf("audio close error:%d\n", ret);
		return ret;
	}

	if (g_write_file_last_enable) {
		printf("please wait...writing data(%u bytes) into %s\n", temp_wav_file_off, path);
		write_size = write(fd, temp_wav_file, temp_wav_file_off);
		if (write_size != temp_wav_file_off) {
			printf("write temp_wav_file failed. ret:%d, wav_file_off:%d\n",
				write_size, temp_wav_file_off);
			goto err1;
		}
		printf("write finish...\n");
	}

	if (save_fs && fd > 0 && ret == 0) {
		sunxi_sound_resize_wav(&header, written);
		lseek(fd, 0, SEEK_SET);
		write(fd, &header, sizeof(header));
	}

err1:
	if (temp_wav_file)
		free(temp_wav_file);

	if (save_fs) {
		if (fd > 0)
			close(fd);
	}
	if (audiobuf)
		free(audiobuf);

	return ret;
}

static void usage(void)
{
	printf("Usage: awtinycap [option]\n");
	printf("-D,          pcm card number\n");
	printf("-d,          pcm device number\n");
	printf("-HD,         play pcm card number\n");
	printf("-Hd,         play pcm device number\n");
	printf("-r,          sample rate\n");
	printf("-f,          sample bits\n");
	printf("-c,          channels\n");
	printf("-p,          period size\n");
	printf("-b,          buffer size\n");
	printf("-s,          capture duration(second)\n");
	printf("-k,          kill last record\n");
	printf("-t,          record and then play\n");
	printf("-l,          loop record n");
	printf("-v,          show pcm setup\n");
	printf("-m,          write file at last\n");
	printf("-h,          show usage\n");
	printf("cap wav,       awtinycap -D 1 -d 0 -c 2 -s 8 -p 320 -b 1280 /usb_msc/test_s16_2ch.wav\n");
	printf("cap and play   awtinycap -D 1 -d 0 -HD 0 -Hd 0 -c 2 -r 16000 -s 8 -t\n");
	printf("\n");
}

static sound_mgr_t *g_last_sound_mgr;
int cmd_awtinycap(int argc, char ** argv)
{
	sound_mgr_t *sound_mgr = NULL;
	g_aw_verbose = 0;
	g_capture_then_play = 0;
	g_write_file_last_enable = 0;
	char *file_path = NULL;

	sound_mgr = sunxi_sound_mgr_create();
	if (!sound_mgr)
		return -1;

	/* default param */
	sound_mgr->rate = 16000;
	sound_mgr->channels = 3;

	g_play_card_num = -1;
	g_play_device_num = -1;

	argv += 1;
	while (*argv) {
		if (strcmp(*argv, "-D") == 0) {
			argv++;
			if (*argv)
				sound_mgr->card = atoi(*argv);
		}
		else if (strcmp(*argv, "-d") == 0) {
			argv++;
			if (*argv)
				sound_mgr->device = atoi(*argv);
		} else if (strcmp(*argv, "-HD") == 0) {
			argv++;
			if (*argv)
				g_play_card_num = atoi(*argv);
		} else if (strcmp(*argv, "-Hd") == 0) {
			argv++;
			if (*argv)
				g_play_device_num = atoi(*argv);
		} else if (strcmp(*argv, "-p") == 0) {
			argv++;
			if (*argv)
				sound_mgr->period_size = atoi(*argv);
		} else if (strcmp(*argv, "-b") == 0) {
			argv++;
			if (*argv)
				sound_mgr->buffer_size = atoi(*argv);
		} else if (strcmp(*argv, "-c") == 0) {
			argv++;
			if (*argv)
				sound_mgr->channels = atoi(*argv);
		} else if (strcmp(*argv, "-r") == 0) {
			argv++;
			if (*argv)
				sound_mgr->rate = atoi(*argv);
		} else if (strcmp(*argv, "-f") == 0) {
			argv++;
			if (*argv) {
				sound_mgr->format_bits = atoi(*argv);
				switch (sound_mgr->format_bits) {
				case 16:
					sound_mgr->format = SND_PCM_FORMAT_S16_LE;
					break;
				case 24:
					sound_mgr->format = SND_PCM_FORMAT_S24_LE;
					break;
				case 32:
					sound_mgr->format = SND_PCM_FORMAT_S32_LE;
					break;
				default:
					printf("%u bits not supprot\n", sound_mgr->format_bits);
				return -1;
				}
			}
		} else if (strcmp(*argv, "-s") == 0) {
			argv++;
			if (*argv)
				sound_mgr->capture_duration = atoi(*argv);
			printf("input capture_duration %u \n", sound_mgr->capture_duration);
		} else if (strcmp(*argv, "-t") == 0) {
			g_capture_then_play = 1;
		} else if (strcmp(*argv, "-l") == 0) {
			if (!g_last_sound_mgr)
				g_last_sound_mgr = sound_mgr;
			g_capture_loop_enable = 1;
		} else if (strcmp(*argv, "-v") == 0) {
			g_aw_verbose = 1;
		} else if (strcmp(*argv, "-k") == 0) {
			if (g_last_sound_mgr)
				g_last_sound_mgr->in_aborting = 1;
			g_capture_loop_enable = 0;
			goto err;
		} else if (strcmp(*argv, "-m") == 0) {
			g_write_file_last_enable = 1;
		}else if (strcmp(*argv, "-h") == 0) {
			usage();
			goto err;
		} else {
			file_path = *argv;
		}

		if (*argv)
			argv++;
	}


	if (file_path) {
		tiny_capture_fs_wav(sound_mgr, file_path);
	} else {
		if (g_capture_then_play)
			tiny_cap_then_play(sound_mgr);
		else
			tiny_capture_fs_wav(sound_mgr, NULL);
	}

err:
	sunxi_sound_mgr_release(sound_mgr);
	g_last_sound_mgr = NULL;

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_awtinycap, awtinycap, capture audio);
