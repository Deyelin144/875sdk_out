/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <hal_cmd.h>
#include "efpg_i.h"
#include "efpg.h"

#undef  HEXDUMP_LINE_CHR_CNT
#define HEXDUMP_LINE_CHR_CNT 16

static int hexstr_to_byte(const char* source, uint8_t* dest, int sourceLen)
{
	uint32_t i;
	uint8_t highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2) {
		highByte = toupper(source[i]);
		lowByte  = toupper(source[i + 1]);

		if (highByte < '0' || (highByte > '9' && highByte < 'A' ) || highByte > 'F') {
			printf("input buf[%d] is %c, not in 0123456789ABCDEF\n", i, source[i]);
			return -1;
		}

		if (lowByte < '0' || (lowByte > '9' && lowByte < 'A' ) || lowByte > 'F') {
			printf("input buf[%d] is %c, not in 0123456789ABCDEF\n", i+1, source[i+1]);
			return -1;
		}

		if (highByte > 0x39)
			highByte -= 0x37;
		else
			highByte -= 0x30;


		if (lowByte > 0x39)
			lowByte -= 0x37;
		else
			lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return 0;
}

static int sunxi_hexdump(const unsigned char *buf, int bytes)
{
	unsigned int i;
	const unsigned char *p = buf;
	bytes++;

	for (i = 1; i < bytes; ++i) {
		printf("%02x ", *p++);
		if (i % 16 == 0) {
			printf("\r\n");
		}
	}
	printf("\r\n");
}

static void efpg_log(uint8_t *buf, uint8_t size)
{
	int i;

	for (i = 0; i < size; i++)
		printf("[%02d] 0x%02x\n", i, buf[i]);
	printf("\n");
}

static void efpg_ua_log(uint8_t *buf, int len_bits)
{
	int len_bytes;

	len_bytes = len_bits / 8;
	sunxi_hexdump(buf, len_bytes);
}

static int efpg_read_example()
{
	uint16_t ret;
	int len_bits;
	uint8_t tmp_data[3] = {0};
	uint8_t buf[32];
	uint8_t ua_buf[128];

	efpg_layout();

	ret = efpg_read_mac(EFPG_MAC_WLAN, buf);
	if (ret == EFPG_ACK_OK) {
		printf("efpg read wlan mac success.\n");
		efpg_log(buf, 6);
	} else {
		printf("efpg read wlan mac fail.ret:%u\n", ret);
	}

	ret = efpg_read_mac(EFPG_MAC_BT, buf);
	if (ret == EFPG_ACK_OK) {
		printf("efpg read bt mac success.\n");
		efpg_log(buf, 6);
	} else {
		printf("efpg read bt mac fail.ret:%u\n", ret);
	}

	ret = efpg_read_dcxo(buf);
	if (ret == EFPG_ACK_OK) {
		printf("efpg read dcxo success.\n");
		efpg_log(buf, 1);
	} else {
		printf("efpg read dcxo fail.ret:%u\n", ret);
	}

	ret = efpg_read_pout(EFPG_POUT_WLAN, buf);
	if (ret == EFPG_ACK_OK) {
		printf("efpg read wlan pout success.\n");
		printf("converted data:\n");
		efpg_log(buf, 3);
		printf("raw data:\n");
		tmp_data[0] = (buf[0] & 0x7f);
		tmp_data[1] = ((buf[0] & 0x80) >> 7) | ((buf[1] & 0x3f) << 1);
		tmp_data[2] = ((buf[1] & 0xc0) >> 6) | ((buf[2] & 0x1f) << 2);
		efpg_log(tmp_data, 3);
	} else {
		printf("efpg read wlan pout fail.ret:%u\n", ret);
	}

	ret = efpg_read_pout(EFPG_POUT_BT, buf);
	if (ret == EFPG_ACK_OK) {
		printf("efpg read bt pout success.\n");
		printf("converted data:\n");
		efpg_log(buf, 3);
		printf("raw data:\n");
		tmp_data[0] = (buf[0] & 0x7f);
		tmp_data[1] = ((buf[0] & 0x80) >> 7) | ((buf[1] & 0x3f) << 1);
		tmp_data[2] = ((buf[1] & 0xc0) >> 6) | ((buf[2] & 0x1f) << 2);
		efpg_log(tmp_data, 3);
	} else {
		printf("efpg read bt pout fail.ret:%u\n", ret);
	}

	len_bits = EFPG_USER_AREA_NUM;
	ret = efpg_read_user_area(0, len_bits, ua_buf);
	if (ret == EFPG_ACK_OK) {
		printf("efpg read user area success.\n");
		efpg_ua_log(ua_buf, len_bits);
	} else {
		printf("efpg read user area fail.ret:%u\n", ret);
	}

	return 0;
}

static void usage(void)
{
	printf("usage: efpg_efuse write|read <efuse_region> <key_string>\n");
	printf(" efpg_efuse write <efuse_region> <key_string> \t \n");
	printf(" efpg_efuse read <efuse_region>/all \t \n");
	printf(" <efuse_region>: mac_wlan/mac_bt/dcxo/pout_wlan/pout_bt/user_area\n");
	printf("eg:\n");
	printf("	efpg_efuse write mac_wlan XX:XX:XX:XX:XX:XX\n");
	printf("	efpg_efuse write pout_wlan 0xXX 0xXX 0xXX\n");
}

int cmd_efpg_test(int argc, char **argv)
{
	uint16_t ret;
	int len_bits;
	uint8_t buf[32] = { 0 };
	char *data = NULL;
	uint8_t tmp_data[3] = {0};
	uint8_t ua_buf[128] = { 0 };

	if (argc != 4 && argc != 3 && argc != 6) {
		printf("wrong argc\n");
		usage();
		return -1;
	}

	if (argc == 3 && !strncmp("read", argv[1], strlen("read"))) {
		if (!strncmp("all", argv[2], strlen("all"))) {
			efpg_read_example();
			return EFPG_ACK_OK;
		} else if (!strncmp("mac_wlan", argv[2], strlen("mac_wlan"))) {
			ret = efpg_read_mac(EFPG_MAC_WLAN, buf);
			if (ret == EFPG_ACK_OK) {
				printf("efpg read wlan mac success.\n");
				efpg_log(buf, 6);
				return EFPG_ACK_OK;
			} else {
				printf("efpg read wlan mac fail.ret:%u\n", ret);
			}
		} else if (!strncmp("mac_bt", argv[2], strlen("mac_bt"))) {
			ret = efpg_read_mac(EFPG_MAC_BT, buf);
			if (ret == EFPG_ACK_OK) {
				printf("efpg read bt mac success.\n");
				efpg_log(buf, 6);
				return EFPG_ACK_OK;
			} else {
				printf("efpg read bt mac fail.ret:%u\n", ret);
			}
		} else if (!strncmp("dcxo", argv[2], strlen("dcxo"))) {
			ret = efpg_read_dcxo(buf);
			if (ret == EFPG_ACK_OK) {
				printf("efpg read dcxo success.\n");
				efpg_log(buf, 1);
				return EFPG_ACK_OK;
			} else {
				printf("efpg read dcxo fail.ret:%u\n", ret);
			}
		} else if (!strncmp("pout_wlan", argv[2], strlen("pout_wlan"))) {
			ret = efpg_read_pout(EFPG_POUT_WLAN, buf);
			if (ret == EFPG_ACK_OK) {
				printf("efpg read wlan pout success.\n");
				printf("converted data:\n");
				efpg_log(buf, 3);
				printf("raw data:\n");
				tmp_data[0] = (buf[0] & 0x7f);
				tmp_data[1] = ((buf[0] & 0x80) >> 7) | ((buf[1] & 0x3f) << 1);
				tmp_data[2] = ((buf[1] & 0xc0) >> 6) | ((buf[2] & 0x1f) << 2);
				efpg_log(tmp_data, 3);
				return EFPG_ACK_OK;
			} else {
				printf("efpg read wlan pout fail.ret:%u\n", ret);
			}
		} else if (!strncmp("pout_bt", argv[2], strlen("pout_bt"))) {
			ret = efpg_read_pout(EFPG_POUT_BT, buf);
			if (ret == EFPG_ACK_OK) {
				printf("efpg read bt pout success.\n");
				printf("converted data:\n");
				efpg_log(buf, 3);
				printf("raw data:\n");
				tmp_data[0] = (buf[0] & 0x7f);
				tmp_data[1] = ((buf[0] & 0x80) >> 7) | ((buf[1] & 0x3f) << 1);
				tmp_data[2] = ((buf[1] & 0xc0) >> 6) | ((buf[2] & 0x1f) << 2);
				efpg_log(tmp_data, 3);
				return EFPG_ACK_OK;
			} else {
				printf("efpg read bt pout fail.ret:%u\n", ret);
			}
		} else if (!strncmp("user_area", argv[2], strlen("user_area"))) {
			len_bits = EFPG_USER_AREA_NUM;
			ret = efpg_read_user_area(0, len_bits, ua_buf);
			if (ret == EFPG_ACK_OK) {
				printf("efpg read user area success.\n");
				efpg_ua_log(ua_buf, len_bits);
				return EFPG_ACK_OK;
			} else {
				printf("efpg read user area fail.ret:%u\n", ret);
			}
		} else {
			printf("please input right para: read mac_wlan/mac_bt/dcxo/pout_wlan/pout_bt/user_area.\n");
			return -1;
		}
	}


	if ((argc == 4 || argc == 6) && !strncmp("write", argv[1], strlen("write"))) {

		data = malloc(strlen(argv[3]));
		if (!data) {
			printf("malloc write buffer %d bytes error!", strlen(argv[3]));
			return -1;
		}

		if ((!strncmp("mac_wlan", argv[2], strlen("mac_wlan"))) ||
					(!strncmp("mac_bt", argv[2], strlen("mac_bt")))) {

			char result[13] = {0};

			sscanf(argv[3], "%2s:%2s:%2s:%2s:%2s:%2s", &result[0],
				&result[2], &result[4], &result[6], &result[8], &result[10]);
			printf("iunput strings:%s\n", result);
			hexstr_to_byte(result, data, strlen(result));
			printf("write data:\n");
			sunxi_hexdump(data, strlen(result)/2);
		} else if ((!strncmp("pout_wlan", argv[2], strlen("pout_wlan")))||
					(!strncmp("pout_bt", argv[2], strlen("pout_bt")))) {
			char result[6] = {0};
			uint32_t pout_data = 0;

			sscanf(argv[3], "0x%2s", &result[0]);
			sscanf(argv[4], "0x%2s", &result[2]);
			sscanf(argv[5], "0x%2s", &result[4]);
			hexstr_to_byte(result, tmp_data, strlen(result));
			printf("raw data      :0x%x  0x%x  0x%x\n",tmp_data[0], tmp_data[1], tmp_data[2]);

			data[0] = (tmp_data[0] & 0x7f) | ((tmp_data[1] & 0x01) << 7);
			data[1] = ((tmp_data[1] & 0x7e) >> 1) | ((tmp_data[2] & 0x03) << 6);
			data[2] = ((tmp_data[2] & 0x7c) >> 2);
			printf("converted data:\n");
			sunxi_hexdump(data, strlen(result)/2);
		} else {
			hexstr_to_byte(argv[3], data, strlen(argv[3]));
			printf("write data:\n");
			sunxi_hexdump(data, strlen(argv[3])/2);
		}

		if (!strncmp("mac_wlan", argv[2], strlen("mac_wlan"))) {
			ret = efpg_write_mac(EFPG_MAC_WLAN, data);
			if (ret == EFPG_ACK_OK) {
				printf("efpg write wlan mac success.\n");
			} else {
				printf("efpg write wlan mac fail.ret:%u\n", ret);
			}
		} else if (!strncmp("mac_bt", argv[2], strlen("mac_bt"))) {
			ret = efpg_write_mac(EFPG_MAC_BT, data);
			if (ret == EFPG_ACK_OK) {
				printf("efpg write bt mac success.\n");
			} else {
				printf("efpg write bt mac fail.ret:%u\n", ret);
			}
		} else if (!strncmp("dcxo", argv[2], strlen("dcxo"))) {
			ret = efpg_write_dcxo(data);
			if (ret == EFPG_ACK_OK) {
				printf("efpg write dcxo success.\n");
			} else {
				printf("efpg write dcxo fail.ret:%u\n", ret);
			}
		} else if (!strncmp("pout_wlan", argv[2], strlen("pout_wlan"))) {
			ret = efpg_write_pout(EFPG_POUT_WLAN, data);
			if (ret == EFPG_ACK_OK) {
				printf("efpg write wlan pout success.\n");
			} else {
				printf("efpg write wlan pout fail.ret:%u\n", ret);
			}
		} else if (!strncmp("pout_bt", argv[2], strlen("pout_bt"))) {
			ret = efpg_write_pout(EFPG_POUT_BT, data);
			if (ret == EFPG_ACK_OK) {
				printf("efpg write bt pout success.\n");
			} else {
				printf("efpg write bt pout fail.ret:%u\n", ret);
			}
		} else if (!strncmp("user_area", argv[2], strlen("user_area"))) {
			len_bits = EFPG_USER_AREA_NUM;
			ret = efpg_write_user_area(0, len_bits, data);
			if (ret == EFPG_ACK_OK) {
				printf("efpg write user area success.\n");
			} else {
				printf("efpg write user area fail.ret:%u\n", ret);
			}
		} else {
			printf("please input right para: write mac_wlan/mac_bt/dcxo/pout_wlan/pout_bt/user_area.\n");
			return -1;
		}
	} else {
			printf("input wrong para\n");
			usage();
			return -1;
	}




	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_efpg_test, efpg_efuse, efpg read/write efuse OEM test);

