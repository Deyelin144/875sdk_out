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

#include <stdio.h>
#include <stdint.h>
#include <tinatest.h>
#include <pthread.h>
#include <unistd.h>
#include <aw-alsa-lib/pcm.h>


static int tt_playback_open_close(int argc, char *argv[])
{
	int loop_count = 100;
	char *card = "hw:audiocodecdac";
	snd_pcm_t *pcm;
	int ret;
	char string[128];

	if (argc == 2) {
		loop_count = atoi(argv[1]);
	}

	if (loop_count < 0) {
		printf("loop_count error :%d\n", loop_count);
		return -1;
	}

	snprintf(string, sizeof(string),
		"playback stream open close test start(count %d).\n",
		loop_count);
	ttips(string);
	while (loop_count--) {
		ret = snd_pcm_open(&pcm, card, SND_PCM_STREAM_PLAYBACK, 0);
		if (ret != 0) {
			printf("snd_pcm_open failed, return %d\n", ret);
			return -1;
		}
		ret = snd_pcm_close(pcm);
		if (ret != 0) {
			printf("snd_pcm_close failed, return %d\n", ret);
			return -1;
		}
		if (loop_count%10 == 0) {
			printf("remain count %d\n", loop_count);
		}
	}

	ttips("playback stream open close test finish.\n");
	return 0;
}
testcase_init(tt_playback_open_close, audiopoc, playback open and close test);
