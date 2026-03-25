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

extern int cmd_fork(int argc, char ** argv);
extern int cmd_as_test(int argc, char ** argv);

static void do_playback(void)
{
	int ret;
	char *playback_argv[] = {
		"fork", "as_test",
		"-s", "2",
		"-d", "3",
	};
	int playback_argc = sizeof(playback_argv) / sizeof(char *);

	cmd_fork(playback_argc, playback_argv);
}

static void do_capture(void)
{
	int ret;
	char *capture_argv[] = {
		"as_test",
		"-t", /* capture then play */
		"-s", "1",
		"-d", "4",
	};
	int capture_argc = sizeof(capture_argv) / sizeof(char *);

	cmd_as_test(capture_argc, capture_argv);
}

static int tt_playback_and_capture(int argc, char *argv[])
{
	int ret;

	ttips("Starting playing with speaker\n");
	do_playback();
	usleep(500*1000);
	ttips("Starting recording\n");
	do_capture();
	ret = ttrue("Finish playing. Can you hear the sound from speaker?\n");
	if (ret < 0) {
		printf("enter no\n");
		return -1;
	}

	return 0;
}
testcase_init(tt_playback_and_capture, audiopc, playback and then caputre);
