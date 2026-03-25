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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <io.h>

#include <hal_timer.h>
#include <console.h>

#define CMD_DCDC_GPRACM_DCDC_CTRL0	(0x40050004)
#define CMD_DCDC_GPIO_BASE		(0x4004a400)
#define CMD_DCDC_GPIO_PA_CFG1		(CMD_DCDC_GPIO_BASE + 0x04)
#define PA12_CFG_MASK			(0xf << 16)
#define CMD_DCDC_GPIO_PA_CFG2		(CMD_DCDC_GPIO_BASE + 0x08)
#define PA18_CFG_MASK			(0xf << 8)
#define CMD_DCDC_GPIO_PA_CFG3		(CMD_DCDC_GPIO_BASE + 0x0c)
#define PA28_CFG_MASK			(0xf << 16)
#define CMD_DCDC_GPIO_PA_DATA		(CMD_DCDC_GPIO_BASE + 0x10)

#define SWITCH_TO_INSIDE_DCDC		(0)
#define SWITCH_TO_OUTSIDE_DCDC		(1)

static void usage(void)
{
	char help[] = \
	"Usage:\n"
	"-s: switch DCDC;\n"
	"    0 - switch to inside DCDC, 1- switch to outside dcdc\n"
	"-t: set delay dcdc_time(ms) after enable inside/outside DCDC\n"
	"    0 - 3000 ms\n"
	"\n"
	;

	printf("%s", help);
}

int dcdc_time = 500;
int cmd_dcdc_switch(int argc, char **argv)
{
	int ret;
	int opt;
	int value;
	uint32_t reg_val;

	while((opt = getopt(argc, argv, "s:t:h")) != -1) {
		switch(opt) {
		case 's':
			value = atoi(optarg);
			if (value == SWITCH_TO_INSIDE_DCDC) {
				printf("change to inside dcdc\n");

				reg_val = readl(CMD_DCDC_GPRACM_DCDC_CTRL0);
				if (reg_val & 0x1)
					writel(reg_val & ~(0x3 << 1)
						| 0x4, CMD_DCDC_GPRACM_DCDC_CTRL0);
				else
					printf("inside dcdc is already enabled\n");

				hal_msleep(dcdc_time);

				reg_val = readl(CMD_DCDC_GPIO_PA_DATA);
				writel(reg_val & ~(0x1 << 18), CMD_DCDC_GPIO_PA_DATA);

				reg_val = readl(CMD_DCDC_GPIO_PA_CFG2);
				writel((reg_val & ~PA18_CFG_MASK)
					| 0x1 << 8, CMD_DCDC_GPIO_PA_CFG2);
			} else if (value == SWITCH_TO_OUTSIDE_DCDC) {
				printf("change to outside dcdc\n");
				reg_val = readl(CMD_DCDC_GPIO_PA_DATA);
				writel(reg_val | (0x1 << 18), CMD_DCDC_GPIO_PA_DATA);

				reg_val = readl(CMD_DCDC_GPIO_PA_CFG2);
				writel((reg_val & ~PA18_CFG_MASK)
					| 0x1 << 8, CMD_DCDC_GPIO_PA_CFG2);

				hal_msleep(dcdc_time);
				reg_val = readl(CMD_DCDC_GPRACM_DCDC_CTRL0);
				if (reg_val & 0x1)
					printf("inside dcdc is already disabled\n");
				else
					writel(reg_val | 0x6, CMD_DCDC_GPRACM_DCDC_CTRL0);
			}
			break;
		case 't':
			value = atoi(optarg);
			if ((value >= 0) && (value <= 3000)) {
				dcdc_time = value;
				printf("set delay time %d ms\n", dcdc_time);
			} else {
				printf("invalid time\n");
				ret = -EINVAL;
			}
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}

}
FINSH_FUNCTION_EXPORT_CMD(cmd_dcdc_switch, dcdc_switch, change dcdc source);

static void sys_usage(void)
{
	char help[] = \
	"Usage:\n"
	"-s: switch ctrl io;\n"
	"    1: high level, 0: low level\n"
	"for example, change IO to high level to switch SYS_EN:\n"
	"cmd enter 0x11: sys_switch 17\n"
	"\n"
	;

	printf("%s", help);
}

void switch_sys_en_io(int val)
{
	uint32_t reg_val;
	if (val == 1) {
		printf("switch SYS_EN, PA28: high level\n");

		reg_val = readl(CMD_DCDC_GPIO_PA_DATA);
		writel(reg_val | (0x1 << 28), CMD_DCDC_GPIO_PA_DATA);

		reg_val = readl(CMD_DCDC_GPIO_PA_CFG3);
		writel((reg_val & ~PA28_CFG_MASK)
			| 0x1 << 16, CMD_DCDC_GPIO_PA_CFG3);
	}

	if (val == 0) {
		printf("switch SYS_EN, PA28: low level\n");

		reg_val = readl(CMD_DCDC_GPIO_PA_DATA);
		writel(reg_val & ~(0x1 << 28), CMD_DCDC_GPIO_PA_DATA);

		reg_val = readl(CMD_DCDC_GPIO_PA_CFG3);
		writel((reg_val & ~PA28_CFG_MASK)
			| 0x1 << 16, CMD_DCDC_GPIO_PA_CFG3);
	}
}

void switch_sys_dsp_en_io(int val)
{
	uint32_t reg_val;
	if (val == 1) {
		printf("switch SYS_DSP_EN, PA12: high level\n");

		reg_val = readl(CMD_DCDC_GPIO_PA_DATA);
		writel(reg_val | (0x1 << 12), CMD_DCDC_GPIO_PA_DATA);

		reg_val = readl(CMD_DCDC_GPIO_PA_CFG1);
		writel((reg_val & ~PA12_CFG_MASK)
			| 0x1 << 16, CMD_DCDC_GPIO_PA_CFG1);
	} else if (val == 0) {
		printf("switch SYS_DSP_EN, PA12: low level\n");

		reg_val = readl(CMD_DCDC_GPIO_PA_DATA);
		writel(reg_val & ~(0x1 << 12), CMD_DCDC_GPIO_PA_DATA);

		reg_val = readl(CMD_DCDC_GPIO_PA_CFG1);
		writel((reg_val & ~PA12_CFG_MASK)
			| 0x1 << 16, CMD_DCDC_GPIO_PA_CFG1);
	}
}

int cmd_sys_switch(int argc, char **argv)
{
	int ret;
	int opt;
	int value;
	uint32_t reg_val;

	while((opt = getopt(argc, argv, "s:h")) != -1) {
		switch(opt) {
		case 's':
			value = atoi(optarg);

			if (value == 1)
				switch_sys_en_io(1);
			else if (value == 0)
				switch_sys_en_io(0);
			break;
		case 'h':
		default:
			sys_usage();
			break;
		}
	}
}
FINSH_FUNCTION_EXPORT_CMD(cmd_sys_switch, sys_switch, switch_SYS_EN);

int cmd_sys_dsp_switch(int argc, char **argv)
{
	int ret;
	int opt;
	int value;
	uint32_t reg_val;

	while((opt = getopt(argc, argv, "s:h")) != -1) {
		switch(opt) {
		case 's':
			value = atoi(optarg);

			if (value == 1)
				switch_sys_dsp_en_io(1);
			else if (value == 0)
				switch_sys_dsp_en_io(0);
			break;
		case 'h':
		default:
			sys_usage();
			break;
		}
	}
}
FINSH_FUNCTION_EXPORT_CMD(cmd_sys_dsp_switch, sys_dsp_switch, switch_SYS_DSP_EN);

