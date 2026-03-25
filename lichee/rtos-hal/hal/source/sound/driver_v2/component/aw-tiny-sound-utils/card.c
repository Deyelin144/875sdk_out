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
#include <unistd.h>
#include <fcntl.h>
#include <hal_cmd.h>
#include <sunxi_hal_sound.h>


static void sunxi_card_list(void)
{
	char info[128];
	struct sunxi_sound_info_buffer buf;

	memset(info, 0 , sizeof(info));
	buf.buffer = info;
	buf.len = sizeof(info);

	sunxi_sound_cards_info(&buf);
	printf("======== Sound Card list ========\n");
	printf("%s", buf.buffer);
	printf("\n");
}

static void sunxi_pcm_list(void)
{
	char info[320];
	struct sunxi_sound_info_buffer buf;

	memset(info, 0 , sizeof(info));
	buf.buffer = info;
	buf.len = sizeof(info);

	sunxi_sound_pcm_read(&buf);
	printf("======== pcm info list ========\n");
	printf("%s", buf.buffer);
	printf("\n");
}

static int sunxi_pcm_dataflow_info(unsigned int  card_num, unsigned int device_num, unsigned int sub_device_num, int stream)
{
	int ret;
	char info[384];
	struct sunxi_sound_info_buffer buf;

	memset(info, 0, sizeof(info));
	buf.buffer = info;
	buf.len = sizeof(info);

	ret = sunxi_sound_dataflow_info_read(card_num, device_num, sub_device_num, stream, &buf);
	if (ret != 0) {
		printf("sunxi_sound_dataflow_info_read failed ret %d\n", ret);
		return ret;
	}
	printf("======== pcm dataflow info list ========\n");
	printf("%s", buf.buffer);
	printf("\n");

	return ret;
}

static void usage(void)
{
	printf("Usage: soundcard [option]\n");
	printf("-D,          sound card number\n");
	printf("-d,          sound card device number\n");
	printf("-l,          sound card list\n");
	printf("-i,          sound card info\n");
	printf("-s,          sound card pcm stream info, 0-playback; 1-capture\n");
	printf("awsoundcard -l,          list cards\n");
	printf("awsoundcard -i,          list card pcms\n");
	printf("awsoundcard -D 1 -d 0 -u 0 -s 1,      list capture stream info\n");
	printf("awsoundcard -D 0 -d 0 -u 0 -s 0,      list playback stream info\n");
	printf("\n");
}

int cmd_awsoundcard(int argc, char *argv[])
{
	int c;
	int card_num = 0;
	int device_num = 0;
	int sub_device_num = 0;
	int stream = -1;

	while ((c = getopt(argc, argv, "D:d:u:lis:h")) != -1) {
		switch (c) {
		case 'D':
			card_num = atoi(optarg);
			break;
		case 'd':
			device_num = atoi(optarg);
			break;
		case 'u':
			sub_device_num = atoi(optarg);
			break;
		case 'l':
			sunxi_card_list();
			return 0;
		case 'i':
			sunxi_pcm_list();
			return 0;
		case 's':
			stream = atoi(optarg);
			if (stream != 0 && stream != 1) {
				printf("unknown stream:%d\n", stream);
			}
			return sunxi_pcm_dataflow_info(card_num, device_num, sub_device_num, stream);
		default:
			usage();
			return 0;
		}
	}

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_awsoundcard, awsoundcard, soundcard info);
