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
#include <hal_timer.h>
#include <hal_sem.h>
#include <hal_mutex.h>
#include <aw-tiny-sound-lib/sunxi_tiny_sound_pcm.h>

static unsigned int g_delay_time = 100;
extern unsigned int g_aw_verbose;

typedef struct {
	void *data;
	unsigned int data_len;
	unsigned int data_frames;
	unsigned int rate,channels;
	unsigned int w_ptr, r_ptr;
	hal_sem_t w_sem, r_sem;
	unsigned char w_wait, r_wait;
	unsigned char empty;
	unsigned char exit_flag;
	hal_mutex_t mutex;
	void *private_data[2];
} data_xfer_t;

static data_xfer_t g_data_xfer;

static int data_xfer_create(data_xfer_t *xfer,
		unsigned int rate, unsigned int channels, unsigned int ms)
{
	if (xfer->data) {
		printf("xfer->data already malloc, aloop maybe running?\n");
		return -1;
	}

	xfer->channels = channels;
	xfer->rate = rate;
	xfer->w_ptr = xfer->r_ptr = 0;
	xfer->w_wait = xfer->r_wait = 0;
	xfer->empty = 1;
	xfer->exit_flag = 0;

	xfer->data_frames = xfer->rate * ms / 1000;
	xfer->data_len = xfer->data_frames * xfer->channels * 2;
	xfer->data  = malloc(xfer->data_len);
	if (!xfer->data) {
		printf("no memory\n");
		return -1;
	}

	xfer->w_sem = hal_sem_create(0);
	xfer->r_sem = hal_sem_create(0);
	xfer->mutex = hal_mutex_create();

	return 0;
}

/* write available */
static inline unsigned int data_w_avail(data_xfer_t *xfer)
{
	unsigned int residue;

	if (xfer->w_ptr < xfer->r_ptr)
		residue = xfer->r_ptr - xfer->w_ptr;
	else if (xfer->w_ptr == xfer->r_ptr && xfer->empty != 1)
		residue = 0;
	else
		residue = xfer->data_frames - xfer->w_ptr + xfer->r_ptr;
	return residue;
}

/* read available */
static inline unsigned int data_r_avail(data_xfer_t *xfer)
{
	unsigned int residue;

	if (xfer->w_ptr > xfer->r_ptr)
		residue = xfer->w_ptr - xfer->r_ptr;
	else if (xfer->w_ptr == xfer->r_ptr && xfer->empty == 1)
		residue = 0;
	else
		residue = xfer->data_frames - xfer->r_ptr + xfer->w_ptr;
	return residue;
}

/* read:0; write:1 */
static inline int data_sem_wait(data_xfer_t *xfer, int rw)
{
	unsigned char *wait;
	hal_sem_t sem;
	int ret;

	if (rw) {
		wait = &xfer->w_wait;
		sem = xfer->w_sem;
	} else {
		wait = &xfer->r_wait;
		sem = xfer->r_sem;
	}

	*wait = 1;
	hal_mutex_unlock(xfer->mutex);
	ret = hal_sem_wait(sem);
	hal_mutex_lock(xfer->mutex);
	*wait = 0;

	return ret;
}

static inline int data_sem_emit(data_xfer_t *xfer, int rw)
{
	unsigned char *wait;
	hal_sem_t sem;

	if (rw) {
		wait = &xfer->w_wait;
		sem = xfer->w_sem;
	} else {
		wait = &xfer->r_wait;
		sem = xfer->r_sem;
	}

	if (*wait == 0)
		return 0;
	return hal_sem_post(sem);
}

#define DUMP_DATA_INFO() \
do { \
	printf("[%s](%d) residue=%u, frames=%u, w_p=%u, r_p=%u, empty=%u\n", \
			__func__, __LINE__, \
			residue, frames, xfer->w_ptr, xfer->r_ptr, xfer->empty); \
} while (0)

static int data_push(data_xfer_t *xfer, sound_mgr_t *mgr, void *data, unsigned int frames)
{
	int ret = 0;
	unsigned int bytes, ofs;
	unsigned int residue;

	hal_mutex_lock(xfer->mutex);
	residue = data_w_avail(xfer);
	/*DUMP_DATA_INFO();*/
	while (residue < frames) {
		data_sem_wait(xfer, 1);
		residue = data_w_avail(xfer);
		if (xfer->exit_flag) {
			hal_mutex_unlock(xfer->mutex);
			return -1;
		}
	}

	bytes = sunxi_sound_pcm_frames_to_bytes(mgr->handle, frames);
	ofs = sunxi_sound_pcm_frames_to_bytes(mgr->handle, xfer->w_ptr);
	residue = xfer->data_len - ofs;
	if (bytes > residue) {
		memcpy(xfer->data + ofs, data, residue);
		memcpy(xfer->data, data + residue, bytes - residue);
		xfer->w_ptr = sunxi_sound_pcm_bytes_to_frames(mgr->handle, bytes - residue);
	} else {
		memcpy(xfer->data + ofs, data, bytes);
		xfer->w_ptr += frames;
	}
	xfer->empty = 0;

	ret = data_sem_emit(xfer, 0);
	hal_mutex_unlock(xfer->mutex);

	return ret;
}

static int data_pop(data_xfer_t *xfer, sound_mgr_t *mgr, void *data, unsigned int frames)
{
	int ret = 0;
	unsigned int bytes, ofs;
	unsigned int residue;

	hal_mutex_lock(xfer->mutex);
	residue = data_r_avail(xfer);
	/*DUMP_DATA_INFO();*/
	while (residue < frames) {
		data_sem_wait(xfer, 0);
		residue = data_r_avail(xfer);
		if (xfer->exit_flag) {
			hal_mutex_unlock(xfer->mutex);
			return -1;
		}
	}

	/*TODO: rate,ch,format not equal */
	bytes = sunxi_sound_pcm_frames_to_bytes(mgr->handle, frames);
	ofs = sunxi_sound_pcm_frames_to_bytes(mgr->handle, xfer->r_ptr);
	residue = xfer->data_len - ofs;
	if (bytes > residue) {
		memcpy(data, xfer->data + ofs, residue);
		memcpy(data + residue, xfer->data, bytes - residue);
		xfer->r_ptr = sunxi_sound_pcm_bytes_to_frames(mgr->handle, bytes - residue);
	} else {
		memcpy(data, xfer->data + ofs, bytes);
		xfer->r_ptr += frames;
	}
	if (xfer->r_ptr == xfer->w_ptr)
		xfer->empty = 1;

	ret = data_sem_emit(xfer, 1);

	hal_mutex_unlock(xfer->mutex);

	return ret;
}

static int data_wait_enough_data(data_xfer_t *xfer, unsigned int frames)
{
	unsigned int residue;

	hal_mutex_lock(xfer->mutex);
	residue = data_r_avail(xfer);
	/*DUMP_DATA_INFO();*/
	while (residue < frames) {
		data_sem_wait(xfer, 0);
		residue = data_r_avail(xfer);
		if (xfer->exit_flag) {
			hal_mutex_unlock(xfer->mutex);
			return -1;
		}
	}
	hal_mutex_unlock(xfer->mutex);
	return 0;
}

static void playback_task(void *arg)
{
	sound_mgr_t *mgr = (sound_mgr_t *)arg;

	int ret = 0;
	char *audiobuf = NULL;
	unsigned int chunk_bytes, frame_bytes = 0;
	unsigned int frames_loop = 512;

	printf("[%s] line:%d playback start\n", __func__, __LINE__);
	g_data_xfer.private_data[0] = mgr;
	/* open card */
	ret = sunxi_pcm_open(&mgr->handle, mgr->card, mgr->device, SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		printf("sunxi_pcm_open error:%d\n", ret);
		goto pb_exit;
	}

	ret = sunxi_set_param(mgr->handle, mgr->format, mgr->rate, mgr->channels,
			mgr->period_size, mgr->buffer_size);
	if (ret < 0) {
		printf("audio set pcm param error:%d\n", ret);
		goto pb_exit;
	}

	ret = sunxi_pcm_prepare(mgr->handle);
	if (ret < 0) {
		printf("sunxi_pcm_prepare failed ret %d!\n", ret);
		goto pb_exit;
	}

	frame_bytes = sunxi_sound_pcm_frames_to_bytes(mgr->handle, 1);
	chunk_bytes = sunxi_sound_pcm_frames_to_bytes(mgr->handle, frames_loop);

	audiobuf = malloc(chunk_bytes);
	if (!audiobuf) {
		printf("no memory...\n");
		goto pb_exit;
	}
	/* wait enough data */
	if (data_wait_enough_data(&g_data_xfer, g_data_xfer.data_frames / 2) < 0)
		goto pb_exit;

	while (!mgr->in_aborting) {
		long f = frames_loop;

		/* read data from data buffer */
		data_pop(&g_data_xfer, mgr, audiobuf, f);
		f = sunxi_pcm_write(mgr->handle, audiobuf, f, frame_bytes);
		if (f < 0) {
			printf("pcm read error, return %ld\n", f);
			break;
		}
	}

	ret = sunxi_pcm_drain(mgr->handle);
	if (ret < 0)
		printf("stop failed!, return %d\n", ret);

pb_exit:
	if (audiobuf)
		free(audiobuf);
	if (mgr->handle != NULL) {
		/* close card */
		ret = sunxi_pcm_close(mgr->handle);
		if (ret < 0) {
			printf("audio close error:%d\n", ret);
		}
		mgr->handle = NULL;
	}
	vTaskDelete(NULL);
}

static void capture_task(void *arg)
{
	sound_mgr_t *mgr = (sound_mgr_t *)arg;

	int ret = 0;
	char *audiobuf = NULL;
	unsigned int chunk_bytes, frame_bytes = 0;
	unsigned int frames_loop = 512;

	printf("[%s] line:%d capture start\n", __func__, __LINE__);
	g_data_xfer.private_data[1] = mgr;
	/* open card */
	ret = sunxi_pcm_open(&mgr->handle, mgr->card, mgr->device, SND_PCM_STREAM_CAPTURE, 0);
	if (ret < 0) {
		printf("audio open error:%d\n", ret);
		goto cap_exit;
	}

	ret = sunxi_set_param(mgr->handle, mgr->format, mgr->rate, mgr->channels,
			mgr->period_size, mgr->buffer_size);
	if (ret < 0)
		goto cap_exit;

	ret = sunxi_pcm_prepare(mgr->handle);
	if (ret < 0) {
		printf("sunxi_pcm_prepare failed ret %d!\n", ret);
		goto cap_exit;
	}


	frame_bytes = sunxi_sound_pcm_frames_to_bytes(mgr->handle, 1);
	chunk_bytes = sunxi_sound_pcm_frames_to_bytes(mgr->handle, frames_loop);

	audiobuf = malloc(chunk_bytes);
	if (!audiobuf) {
		printf("no memory...\n");
		goto cap_exit;
	}

	while (!mgr->in_aborting) {
		long f = frames_loop;
		f = sunxi_pcm_read(mgr->handle, audiobuf, f, frame_bytes);
		if (f < 0) {
			printf("pcm read error, return %ld\n", f);
			break;
		}
		/* fill into buffer and notify */
		data_push(&g_data_xfer, mgr, audiobuf, f);
	}

	ret = sunxi_pcm_drain(mgr->handle);
	if (ret < 0)
		printf("stop failed!, return %d\n", ret);

cap_exit:
	if (audiobuf)
		free(audiobuf);
	if (mgr->handle != NULL) {
		/* close card */
		ret = sunxi_pcm_close(mgr->handle);
		if (ret < 0) {
			printf("audio close error:%d\n", ret);
		}
		mgr->handle = NULL;
	}
	vTaskDelete(NULL);
}

static int cap_task_create(sound_mgr_t *mgr)
{
	TaskHandle_t cap_task_handle = NULL;

	xTaskCreate(capture_task, "AL_Cap", 4096, mgr, 15, &cap_task_handle);
	if (!cap_task_handle) {
		printf("AL_Cap task create failed\n");
		return -1;
	}
	return 0;
}

static int pb_task_create(sound_mgr_t *mgr)
{
	TaskHandle_t pb_task_handle = NULL;

	xTaskCreate(playback_task, "AL_Pb", 4096, mgr, 15, &pb_task_handle);
	if (!pb_task_handle) {
		printf("AL_Pb task create failed\n");
		return -1;
	}
	return 0;
}

static void killall_task(void)
{
	sound_mgr_t *cap_mgr, *pb_mgr;
	data_xfer_t *xfer = &g_data_xfer;
	int timeout = 2000;

	if (!xfer->data)
		return;
	pb_mgr = xfer->private_data[0];
	cap_mgr = xfer->private_data[1];
	if (!pb_mgr || !cap_mgr)
		return;

	cap_mgr->in_aborting = 1;
	pb_mgr->in_aborting = 1;
	xfer->exit_flag = 1;

	while (timeout > 0) {
		int ms = 100;

		data_sem_emit(xfer, 0);
		data_sem_emit(xfer, 1);
		vTaskDelay(pdMS_TO_TICKS(ms));
		timeout -= ms;
		if (!cap_mgr->handle && !pb_mgr->handle) {
			printf("cap,pb all released\n");
			break;
		}
	}
	if (timeout <= 0)
		printf("kill cap,pb task timeout\n");

	free(pb_mgr);
	free(cap_mgr);
	free(xfer->data);
	hal_sem_delete(xfer->w_sem);
	hal_sem_delete(xfer->r_sem);
	hal_mutex_delete(xfer->mutex);
	memset(xfer, 0, sizeof(data_xfer_t));
}

static void usage(void)
{
	printf("Usage: awtinyloop [option]\n");
	printf("-D,          capture pcm card number\n");
	printf("-d,          capture pcm device number\n");
	printf("-HD,         playback pcm card number\n");
	printf("-Hd,         playback pcm device number\n");
	printf("-R,          capture sample rate\n");
	printf("-r,          playback sample rate\n");
	printf("-C,          capture channels\n");
	printf("-c,          playback channels\n");
	printf("-F,          capture format\n");
	printf("-f,          playback format\n");
	printf("-P,          capture pcm period size\n");
	printf("-p,          playback pcm period size\n");
	printf("-B,          capture pcm buffer size\n");
	printf("-b,          playback pcm buffer size\n");
	printf("-t,          delay time\n");
	printf("-k,          kill all task\n");
	printf("-v,          enable pcm dump\n");
	printf("-h,          cmd help\n");
	printf("\n");
}

int cmd_awtinyloop(int argc, char ** argv)
{
	int ret;
	sound_mgr_t *cap_mgr = NULL;
	sound_mgr_t *pb_mgr = NULL;

	g_aw_verbose = 0;

	if (argc < 2) {
		usage();
		return 0;
	}

	cap_mgr = sunxi_sound_mgr_create();
	if (!cap_mgr)
		goto error_exit;

	pb_mgr = sunxi_sound_mgr_create();
	if (!pb_mgr)
		goto error_exit;

	/* default setting */
	cap_mgr->rate = 48000;
	cap_mgr->channels = 2;
	cap_mgr->period_size = cap_mgr->rate * 10 / 1000;
	cap_mgr->buffer_size = cap_mgr->period_size * 4;

	pb_mgr->rate = 48000;
	pb_mgr->channels = 2;
	pb_mgr->period_size = pb_mgr->rate * 10 / 1000;
	pb_mgr->buffer_size = pb_mgr->period_size * 4;

	argv += 1;
	while (*argv) {
		if (strcmp(*argv, "-D") == 0) {
			argv++;
			if (*argv)
				cap_mgr->card = atoi(*argv);
		}
		else if (strcmp(*argv, "-d") == 0) {
			argv++;
			if (*argv)
				cap_mgr->device = atoi(*argv);
		} else if (strcmp(*argv, "-HD") == 0) {
			argv++;
			if (*argv)
				pb_mgr->card = atoi(*argv);
		} else if (strcmp(*argv, "-Hd") == 0) {
			argv++;
			if (*argv)
				pb_mgr->device = atoi(*argv);
		} else if (strcmp(*argv, "-P") == 0) {
			argv++;
			if (*argv)
				cap_mgr->period_size = atoi(*argv);
		} else if (strcmp(*argv, "-p") == 0) {
			argv++;
			if (*argv)
				pb_mgr->period_size = atoi(*argv);
		} else if (strcmp(*argv, "-B") == 0) {
			argv++;
			if (*argv)
				cap_mgr->buffer_size = atoi(*argv);
		} else if (strcmp(*argv, "-b") == 0) {
			argv++;
			if (*argv)
				pb_mgr->buffer_size = atoi(*argv);
		} else if (strcmp(*argv, "-C") == 0) {
			argv++;
			if (*argv)
				cap_mgr->channels = atoi(*argv);
		} else if (strcmp(*argv, "-c") == 0) {
			argv++;
			if (*argv)
				pb_mgr->channels = atoi(*argv);
		} else if (strcmp(*argv, "-R") == 0) {
			argv++;
			if (*argv)
				cap_mgr->rate = atoi(*argv);
		} else if (strcmp(*argv, "-r") == 0) {
			argv++;
			if (*argv)
				pb_mgr->rate = atoi(*argv);
		} else if (strcmp(*argv, "-F") == 0) {
			argv++;
			if (*argv) {
				cap_mgr->format_bits = atoi(*argv);
				switch (cap_mgr->format_bits) {
				case 16:
					cap_mgr->format = SND_PCM_FORMAT_S16_LE;
					break;
				case 24:
					cap_mgr->format = SND_PCM_FORMAT_S24_LE;
					break;
				case 32:
					cap_mgr->format = SND_PCM_FORMAT_S32_LE;
					break;
				default:
					printf("%u bits not supprot\n", cap_mgr->format_bits);
				return -1;
				}
			}
		} else if (strcmp(*argv, "-f") == 0) {
			argv++;
			if (*argv) {
				pb_mgr->format_bits = atoi(*argv);
				switch (pb_mgr->format_bits) {
				case 16:
					pb_mgr->format = SND_PCM_FORMAT_S16_LE;
					break;
				case 24:
					pb_mgr->format = SND_PCM_FORMAT_S24_LE;
					break;
				case 32:
					pb_mgr->format = SND_PCM_FORMAT_S32_LE;
					break;
				default:
					printf("%u bits not supprot\n", pb_mgr->format_bits);
				return -1;
				}
			}
		}else if (strcmp(*argv, "-t") == 0) {
			g_delay_time = 1;
		} else if (strcmp(*argv, "-v") == 0) {
			g_aw_verbose = 1;
		} else if (strcmp(*argv, "-k") == 0) {
			killall_task();
			goto error_exit;
		} else if (strcmp(*argv, "-h") == 0) {
			usage();
			goto error_exit;
		} else {
			usage();
			goto error_exit;
		}

		if (*argv)
			argv++;
	}


	if(cap_mgr->rate != pb_mgr->rate) {
		printf("not support rate convert\n");
		goto error_exit;
	}

	ret = data_xfer_create(&g_data_xfer, cap_mgr->rate, cap_mgr->channels, g_delay_time);
	if (ret != 0)
		goto error_exit;
	ret = cap_task_create(cap_mgr);
	if (ret != 0)
		goto task_exit;
	ret = pb_task_create(pb_mgr);
	if (ret != 0)
		goto task_exit;

	return 0;

task_exit:
	cap_mgr->in_aborting = 1;
	pb_mgr->in_aborting = 1;
	vTaskDelay(pdMS_TO_TICKS(500));
error_exit:
	if (cap_mgr)
		sunxi_sound_mgr_release(cap_mgr);
	if (pb_mgr)
		sunxi_sound_mgr_release(pb_mgr);

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_awtinyloop, awtinyloop, capture and play);
