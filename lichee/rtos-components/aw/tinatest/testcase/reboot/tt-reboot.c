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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tinatest.h>
#include <hal_timer.h>

#define REBOOT_STATUS	CONFIG_TESTCASE_REBOOT_PATH "/tinatest.reboot.status"
#define REBOOT_TIMES CONFIG_REBOOT_TIMES

extern int cmd_reboot(int argc, char ** argv);

int auto_runreboot_test(int argc, char **argv)
{
	FILE *fp = NULL;
	char buf[4] = {0}, recv[4] = {0};
	char fcnt[] = REBOOT_TIMES;
	int tmp = 0;

	fp = fopen(REBOOT_STATUS, "r+");
	if (fp == NULL) {
		return -1;
	}

	printf("=============Test for reboot==============\n");

	fseek(fp, 0, SEEK_SET);
	fread(buf, strlen(fcnt) + 1, 1, fp);
	fclose(fp);

	tmp = strtol(buf, NULL, 0);
	if (tmp == 0) {
		printf("Tinatest Reboot successful!\n");
		remove(REBOOT_STATUS);
		return 0;
	}
	printf("=====reboot times is %d\n", (strtol(fcnt, NULL, 0) - tmp + 1));

	tmp = tmp - 1;
	printf("=====reboot left %d times\n", tmp);
	sprintf(recv, "%d", tmp);

	fp = fopen(REBOOT_STATUS, "w+");
	fwrite(recv, strlen(recv) + 1, 1, fp);
	fseek(fp, 0, SEEK_SET);
	fread(buf, strlen(recv) + 1, 1, fp);

	fclose(fp);

	if (tmp == 0) {
		remove(REBOOT_STATUS);
	}

	sleep(3);
	cmd_reboot(1, NULL);

	return 0;
}

//tt reboottester
int tt_reboottest(int argc, char **argv)
{
	FILE *fp = NULL;
	char buf[4] = {0}, recv[4] = {0};
	char fcnt[] = REBOOT_TIMES;
	int tmp = 0;

	printf("=============Test for reboot==============\n");

	fp = fopen(REBOOT_STATUS, "r+");
	if (fp == NULL) {
		printf("open file failed, touch a new one\n");
		fp = fopen(REBOOT_STATUS, "w+");
		fwrite(fcnt, strlen(fcnt) + 1, 1, fp);
	}

	fseek(fp, 0, SEEK_SET);
	fread(buf, strlen(fcnt) + 1, 1, fp);
	fclose(fp);

	tmp = strtol(buf, NULL, 0);
	if (tmp == 0) {
		printf("Tinatest Reboot successful!\n");
		remove(REBOOT_STATUS);
		return 0;
	}
	printf("=====reboot times is %d\n", (strtol(fcnt, NULL, 0) - tmp + 1));

	tmp = tmp - 1;
	printf("=====reboot left %d times\n", tmp);
	sprintf(recv, "%d", tmp);

	fp = fopen(REBOOT_STATUS, "w+");
	fwrite(recv, strlen(recv) + 1, 1, fp);
	fseek(fp, 0, SEEK_SET);
	fread(buf, strlen(recv) + 1, 1, fp);

	fclose(fp);
	cmd_reboot(1, NULL);
	while(1);
	return 0;
}
testcase_init(tt_reboottest, reboottester, reboottester for tinatest);
//auto_testcase_init(auto_runreboot_test, auto_reboot, auto reboottester for tinatest);
