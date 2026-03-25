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
#include <getopt.h>
#include <ctype.h>
#include <hal_cmd.h>
#include <aw-tiny-sound-lib/sunxi_tiny_sound_mixer.h>
#include <aw-tiny-sound-lib/sunxi_tiny_sound_version.h>


static void awtinymixer_usage(void)
{
	printf("Usage: awtinymixer <options> [command]\n");
	printf("\nAvailable options:\n");
	printf("\t-D, --card NUMBER : specifies the card number of the mixer\n");
	printf("\t-h, --help        : prints this help message and exists\n");
	printf("\t-v, --version     : prints this version of tinymix and exists\n");
	printf("\nAvailable commands:\n");
	printf("\tget NAME|ID       : prints the values of a control\n");
	printf("\tset NAME|ID VALUE : sets the value of a control\n");
	printf("\tcontrols          : lists controls of the mixer\n");
	printf("\tcontents          : lists controls of the mixer and their contents\n");
}

static void awtinymixer_version(void)
{
	printf("awtinymix version 1.0 (awtinyalsa version %s)\n", AWTINYALSA_VERSION);
}

static void awtinymix_print_enum(struct sunxi_mixer_ctl *ctl)
{
	unsigned int num_enums;
	unsigned int i;
	const char *string;

	num_enums = sunxi_mixer_ctl_get_num_enums(ctl);

	for (i = 0; i < num_enums; i++) {
		string = sunxi_mixer_ctl_get_enum_string(ctl, i);
		printf("%s%s", sunxi_mixer_ctl_get_value(ctl, 0) == (int)i ? "> " : " ",
			string);
	}
}

static void awtinymix_detail_control(struct sunxi_mixer *mixer, const char *control)
{
	struct sunxi_mixer_ctl *ctl;
	enum sunxi_mixer_ctl_type type;
	unsigned int num_values;
	unsigned int i;
	int min, max;

	if (isdigit(control[0]))
		ctl = sunxi_mixer_get_ctl(mixer, atoi(control));
	else
		ctl = sunxi_mixer_get_ctl_by_name(mixer, control);

	if (!ctl) {
		fprintf(stderr, "Invalid mixer control\n");
		return;
	}

	type = sunxi_mixer_ctl_get_type(ctl);
	num_values = sunxi_mixer_ctl_get_num_values(ctl);

	for (i = 0; i < num_values; i++) {
		switch (type)
		{
		case SUNXI_MIXER_CTL_TYPE_INT:
			printf("%d", sunxi_mixer_ctl_get_value(ctl, i));
			break;
		case SUNXI_MIXER_CTL_TYPE_ENUM:
			awtinymix_print_enum(ctl);
			break;
		default:
			printf("unknown");
			break;
		};
		if ((i + 1) < num_values) {
			printf(", ");
		}
	}

	if (type == SUNXI_MIXER_CTL_TYPE_INT) {
		min = sunxi_mixer_ctl_get_range_min(ctl);
		max = sunxi_mixer_ctl_get_range_max(ctl);
		printf(" (range %d->%d)", min, max);
	}

}

static void awtinymix_list_controls(struct sunxi_mixer *mixer, int print_all)
{
	struct sunxi_mixer_ctl *ctl;
	const char *name, *type;
	unsigned int num_ctls, num_values;
	unsigned int i;

	num_ctls = sunxi_mixer_get_num_ctls(mixer);

	printf("Number of controls: %u\n", num_ctls);

	if (print_all)
		printf("ctl\ttype\tnum\t%-40svalue\n", "name");
	else
		printf("ctl\ttype\tnum\t%-40s\n", "name");

	for (i = 0; i < num_ctls; i++) {
		ctl = sunxi_mixer_get_ctl(mixer, i);

		name = sunxi_mixer_ctl_get_name(ctl);
		type = sunxi_mixer_ctl_get_type_string(ctl);
		num_values = sunxi_mixer_ctl_get_num_values(ctl);
		printf("%u\t%s\t%u\t%-40s", i, type, num_values, name);
		if (print_all)
			awtinymix_detail_control(mixer, name);
		printf("\n");
	}
}

static void awtinymix_set_value(struct sunxi_mixer *mixer, const char *control,
                              char **values, unsigned int num_values)
{
	struct sunxi_mixer_ctl *ctl;
	enum sunxi_mixer_ctl_type type;
	unsigned int num_ctl_values;
	unsigned int i;

	if (isdigit(control[0]))
		ctl = sunxi_mixer_get_ctl(mixer, atoi(control));
	else
		ctl = sunxi_mixer_get_ctl_by_name(mixer, control);

	if (!ctl) {
		fprintf(stderr, "Invalid mixer control\n");
		return;
	}

	type = sunxi_mixer_ctl_get_type(ctl);
	num_ctl_values = sunxi_mixer_ctl_get_num_values(ctl);

	if (is_number(values[0])) {
		if (num_values == 1) {
			/* Set all values the same */
			int value = atoi(values[0]);

			for (i = 0; i < num_ctl_values; i++) {
				if (sunxi_mixer_ctl_set_value(ctl, i, value)) {
					fprintf(stderr, "Error: invalid value\n");
					return;
				}
			}
		} else {
			/* Set multiple values */
			if (num_values > num_ctl_values) {
				fprintf(stderr,
						"Error: %u values given, but control only takes %u\n",
						num_values, num_ctl_values);
				return;
			}
			for (i = 0; i < num_values; i++) {
				if (sunxi_mixer_ctl_set_value(ctl, i, atoi(values[i]))) {
					fprintf(stderr, "Error: invalid value for index %u\n", i);
					return;
				}
			}
		}
	} else {
		if (type == SUNXI_MIXER_CTL_TYPE_ENUM) {
			if (num_values != 1) {
				fprintf(stderr, "Enclose strings in quotes and try again\n");
				return;
			}
			if (sunxi_mixer_ctl_set_enum_by_string(ctl, values[0]))
				fprintf(stderr, "Error: invalid enum value\n");
		} else {
			fprintf(stderr, "Error: only enum types can be set with strings\n");
		}
	}
}

static int awtinymixer(int argc, char *argv[])
{
	int card = 0;
	struct sunxi_mixer *mixer;
	char *cmd;
	int morehelp = 0;
	int ret;

	const struct option long_option[] = {
		{"help", 0, NULL, 'h'},
		{"version", 0, NULL, 'v'},
		{NULL, 0, NULL, 0},
	};

	optind = 0;
	while (1) {
		int c;

		if ((c = getopt_long(argc, argv, "D:hv", long_option, NULL)) < 0)
			break;
		switch (c) {
		case 'D':
			card = atoi(optarg);
			break;
		case 'h':
			awtinymixer_usage();
			return 0;
		case 'v':
			awtinymixer_version();
			return 0;
		default:
			fprintf(stderr, "Invalid switch or option needs an argument.\n");
			morehelp++;
			break;
		}
	}

	if (morehelp) {
		awtinymixer_usage();
		return 1;
	}

	mixer = sunxi_mixer_open(card);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -ENOENT;
	}

	cmd = argv[optind];
	if (cmd == NULL) {
		fprintf(stderr, "no command specified (see --help)\n");
		ret = -1;
		goto err;
	} else if (strcmp(cmd, "get") == 0) {
		 if ((optind + 1) >= argc) {
			fprintf(stderr, "no control specified\n");
			ret = -1;
			goto err;
		}
		awtinymix_detail_control(mixer, argv[optind + 1]);
		printf("\n");
	} else if (!strcmp("set", argv[optind])) {
		if ((optind + 1) >= argc) {
			fprintf(stderr, "no control specified\n");
			ret = -1;
			goto err;
		}
		if ((optind + 2) >= argc) {
			fprintf(stderr, "no value(s) specified\n");
			ret = -1;
			goto err;
		}
		awtinymix_set_value(mixer, argv[optind + 1], &argv[optind + 2], argc - optind - 2);
	} else if (strcmp(cmd, "controls") == 0) {
		awtinymix_list_controls(mixer, 0);
	} else if (strcmp(cmd, "contents") == 0) {
		awtinymix_list_controls(mixer, 1);
	} else {
		fprintf(stderr, "unknown command '%s' (see --help)\n", cmd);
		awtinymixer_usage();
	}

err:
	sunxi_mixer_close(mixer);

	return ret;
}

int cmd_awtinymixer(int argc, char ** argv)
{
	awtinymixer(argc, argv);
	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_awtinymixer, awtinymixer, awtinymixer utils);
