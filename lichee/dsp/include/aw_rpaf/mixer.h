/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
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

#ifndef _AW_RPAF_MIXER_H_
#define _AW_RPAF_MIXER_H_

#include <aw_rpaf/common.h>

/*
 * param[0] = MSGBOX_SOC_DSP_AUDIO_COMMAND->MSGBOX_SOC_DSP_AUDIO_MIXER_COMMAND
 * param[1] = *snd_soc_dsp_mixer
 * param[2] = SND_SOC_DSP_*_COMMAND
 * param[3] = *params/NULL
 */
struct snd_soc_dsp_mixer {
	uint32_t id;
	uint8_t used;

	uint32_t cmd_val;
	uint32_t params_val;

	/* eg:0 sndcodec; 1 snddmic; 2 snddaudio0; 3 snddaudio1 */
	int32_t card;
	int32_t device;
	/*
	 * 根据名字匹配:
	 * 0: maudiocodec; 1: msnddmic; 2: msnddaudio0; 3: msnddaudio1;
	 */
	char driver[32];

	/* ctl name length */
	char ctl_name[44];
	uint32_t value;

	/*API调用完毕之后需要判断该值 */
	int32_t ret_val;

	struct list_head list;
};

struct snd_dsp_hal_mixer_ops {
	//int32_t (*open)(struct snd_dsp_hal_mixer *mixer);
	int32_t (*open)(void *mixer);
	int32_t (*close)(void *mixer);
	int32_t (*read)(void *mixer);
	int32_t (*write)(void *mixer);
};

struct snd_dsp_hal_mixer {
	/* dsp声卡名字和编号 */
	const char *name;
	uint32_t id;

	/* 从共享内存共享过来 */
	struct snd_soc_dsp_mixer *soc_mixer;

	xTaskHandle *taskHandle;

	/* 用于AudioMixerTask和audioserver通信，回调操作ops */
//	xQueueHandle *ServerReceQueue;
//	xQueueHandle *ServerSendQueue;
//	struct snd_dsp_hal_queue_item CmdItem;

	/* 可以统一实现或者根据声卡具体对应实现 */
	struct snd_dsp_hal_mixer_ops *mixer_ops;

	/*API调用完毕之后需要判断该值 */
	int32_t ret_val;

	void *private_data;

	struct list_head list;
};

int32_t snd_dsp_hal_mixer_process(void *argv);

#endif

