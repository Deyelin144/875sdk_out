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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <hal_cmd.h>
#include <hal_mem.h>
#include <sunxi_hal_efuse.h>


#define EFUSE_BURN_RET_OK 0
#define EFUSE_BURN_RET_ARG_ERR -1
#define EFUSE_BURN_RET_MALLOC_ERR -2
#define EFUSE_BURN_RET_WRITE_ERR -3
#define EFUSE_BURN_RET_CMP_ERR -4
#define CHIPID_KEYLEN 16

static void efuse_dump(char *str,unsigned char *data, int len, int align)
{
        int i = 0;
        if(str)
                printf("\n%s: ",str);
        for(i = 0; i<len; i++)
        {
                if((i%align) == 0)
                {
                        printf("\n");
                        printf("%p: ", data + i);
                }
                printf("%02x ",*(data++));
        }
        printf("\n");
}

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


int cmd_efuse_burn_id(int argc, char **argv)
{
	int ret = EFUSE_BURN_RET_OK;
	int i = 0;
	char *key_hex_string = NULL;
	uint32_t buf_len = 0;
	char *write_key_data = NULL;
	char *read_key_data = NULL;
	efuse_key_map_new_t *chipid_keymap = NULL;

	if (argc != 2) {
		printf("Argument Error!\n");
		printf("Usage: efuse_burn_id <key_hex_string>\n");
		return EFUSE_BURN_RET_ARG_ERR;
	}

	printf("== burn id begin ==\n");

	/* preprate to burn chipid */
	key_hex_string = argv[1];
	buf_len = strlen(argv[1]);
	printf("chipid_hex_string: %s, len: %d\n", key_hex_string, buf_len);
	if (buf_len % 2 != 0) {
		printf("chipid_hex_string len: %d is error!\n", buf_len);
		return EFUSE_BURN_RET_ARG_ERR;
	}

	/* chipid length should be right */
	if ((buf_len / 2) != CHIPID_KEYLEN) {
		printf("error: chipid key len is error, chipid_len: %d, buf_len / 2: %d\n", CHIPID_KEYLEN, buf_len / 2);
		return EFUSE_BURN_RET_ARG_ERR;
	}

	write_key_data = malloc(CHIPID_KEYLEN);
	if (!write_key_data) {
		printf("malloc write buffer %d bytes error!", CHIPID_KEYLEN);
		return EFUSE_BURN_RET_MALLOC_ERR;
	}

	hexstr_to_byte(key_hex_string, write_key_data, buf_len);

	// 1. write chipid to oem3
    ret = hal_efuse_write_ext(1664, 128, write_key_data);
	if (ret) {
		printf("efuse write chipid error: %d\n", ret);
		ret = EFUSE_BURN_RET_WRITE_ERR;
		goto out;
	} else {
		printf("efuse write chipid end\n");
	}

	// 2. read chipid
	read_key_data = malloc(CHIPID_KEYLEN);
	if (!read_key_data) {
		printf("malloc read buffer %d bytes error!", CHIPID_KEYLEN);
		ret = EFUSE_BURN_RET_MALLOC_ERR;
		goto out;
	}

	for(i = 0; i < CHIPID_KEYLEN / 4; i++) {
		*((uint32_t *)read_key_data + i) = efuse_reg_read_key(0xD0 + i * 4);
	}

	printf("efuse reg read  oem3 id success\n");

	// 3. compare write/read chipid
	if (memcmp(write_key_data, read_key_data, CHIPID_KEYLEN)) {
		printf("error: id read data is not equal to chipid write data\n");
		efuse_dump("id write:", write_key_data, CHIPID_KEYLEN, 16);
		efuse_dump("id read:", read_key_data, CHIPID_KEYLEN, 16);
		ret = EFUSE_BURN_RET_CMP_ERR;
		goto out;
	} else {
		printf("compare id write/data chipid success\n");
		efuse_dump("id:", write_key_data, CHIPID_KEYLEN, 16);
	}

	printf("== burn chipid to oem3 success ==\n");

out:
	if (write_key_data)
		free(write_key_data);
	if (read_key_data)
		free(read_key_data);

	return ret;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_efuse_burn_id, efuse_burn_id, efuse burn id);


int cmd_efuse_burn_mac(int argc, char **argv)
{
	int ret = EFUSE_BURN_RET_OK;
	int i = 0;
	char *key_hex_string = NULL;
	uint32_t buf_len = 0;
	char *write_key_data = NULL;
	char *read_key_data = NULL;

	if (argc != 2) {
		printf("Argument Error!\n");
		printf("Usage: burn_mac <key_hex_string>\n");
		return EFUSE_BURN_RET_ARG_ERR;
	}

	printf("== burn mac begin ==\n");

	/* preprate to burn mac */
	key_hex_string = argv[1];
	buf_len = strlen(argv[1]);
	printf("mac_hex_string: %s, len: %d\n", key_hex_string, buf_len);
	if (buf_len % 2 != 0) {
		printf("mac_hex_string len: %d is error!\n", buf_len);
		return EFUSE_BURN_RET_ARG_ERR;
	}

	/* mac length should be right */
	if ((buf_len / 2) != 6) {
		printf("error: mac key len is not 6 bytes\n");
		return EFUSE_BURN_RET_ARG_ERR;
	}

	write_key_data = malloc(6);
	if (!write_key_data) {
		printf("malloc write buffer 6 bytes error!\n");
		return EFUSE_BURN_RET_MALLOC_ERR;
	}

	hexstr_to_byte(key_hex_string, write_key_data, buf_len);

	// 1. write mac
    ret = hal_efuse_write_ext(134, 48, write_key_data);
	if (ret) {
		printf("efuse write mac error: %d\n", ret);
		ret = EFUSE_BURN_RET_WRITE_ERR;
		goto out;
	} else {
		printf("efuse write mac end\n");
	}

	// 2. read mac
	read_key_data = malloc(6);
	if (!read_key_data) {
		printf("malloc read buffer 6 bytes error!\n");
		ret = EFUSE_BURN_RET_MALLOC_ERR;
		goto out;
	}

	for(i = 0; i < 8 / 4; i++) {
		*((uint32_t *)read_key_data + i) = efuse_reg_read_key(0x86 + i * 4);
	}

	printf("efuse reg read mac success\n");

	// 3. compare write/read mac
	if (memcmp(write_key_data, read_key_data, 6)) {
		printf("error: mac read data is not equal to mac write data\n");
		efuse_dump("mac write:", write_key_data, 6, 16);
		efuse_dump("mac read:", read_key_data, 6, 16);
		ret = EFUSE_BURN_RET_CMP_ERR;
		goto out;
	} else {
		printf("compare mac write/data mac success\n");
		efuse_dump("mac:", write_key_data, 6, 16);
	}

	printf("== burn mac success ==\n");
out:
	if (write_key_data)
		free(write_key_data);
	if (read_key_data)
		free(read_key_data);

	return ret;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_efuse_burn_mac, efuse_burn_mac, efuse burn mac);


int cmd_efuse_read_id(int argc, char **argv)
{
	char buffer[128] = {0};
	int ret = 0;

	ret = hal_efuse_read_ext(1664, 128, buffer);
	if (ret < 0) {
		printf("read efuse fail:%d\n", ret);
		return ret;
	}
	printf("read chipid data:\n");
	efuse_dump("chipid:", buffer, CHIPID_KEYLEN, 16);

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_efuse_read_id, efuse_read_id, efuse read id from oem3)

int cmd_efuse_read_mac(int argc, char **argv)
{
	char buffer[48] = {0};
	int ret = 0;

	ret = hal_efuse_read_ext(134, 48, buffer);
	if (ret) {
		printf("read efuse fail:%d\n", ret);
		return ret;
	}
	printf("read mac data:\n");
	efuse_dump("mac:", buffer, 6, 16);

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_efuse_read_mac, efuse_read_mac, efuse read mac)

// #define XR_EFUSE_DEBUG
enum state_efuse {
	STATE_IDLE = 0,
	STATE_CONNECTED,
};
// unsigned Flag_efuse_finish = 0;

int cmd_ococci_connect(int argc, char **argv)
{
	// if (Flag_efuse_finish == 1)
	// 	return 0;

	printf("hello_too\r\n");
	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ococci_connect, hello_xr, efuse connect)

int cmd_ococci_readid(int argc, char **argv)
{
	unsigned char chipid[16] = {0};
	hal_efuse_get_chipid(chipid);
#ifndef XR_EFUSE_DEBUG
	printf("xrchipID=0x");
	for(int i = 0; i < 16; i++) {
		printf("%02x", chipid[i]);
	}
	printf("\n");
#else
	printf("xrchipID=0x00\n");
#endif
	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ococci_readid, xr_get_chipID, efuse read id)
/**
 * 设置chipid
*/
int cmd_ococci_setid(int argc, char **argv)
{
	int ret = EFUSE_BURN_RET_OK;
	int i = 0;
	char *key_hex_string = NULL;
	uint32_t buf_len = 0;
	char *write_key_data = NULL;
	char *read_key_data = NULL;
	efuse_key_map_new_t *chipid_keymap = NULL;

	if (argc != 2) {
		printf("Argument Error!\n");
		printf("Usage: efuse_burn_id <key_hex_string>\n");
		return EFUSE_BURN_RET_ARG_ERR;
	}

	printf("== burn id begin ==\n");

	/* preprate to burn chipid */
	key_hex_string = argv[1];
	buf_len = strlen(argv[1]);
	printf("chipid_hex_string: %s, len: %d\n", key_hex_string, buf_len);
	if (buf_len % 2 != 0) {
		printf("chipid_hex_string len: %d is error!\n", buf_len);
		return EFUSE_BURN_RET_ARG_ERR;
	}

	/* chipid length should be right */
	if ((buf_len / 2) != CHIPID_KEYLEN) {
		printf("error: chipid key len is error, chipid_len: %d, buf_len / 2: %d\n", CHIPID_KEYLEN, buf_len / 2);
		return EFUSE_BURN_RET_ARG_ERR;
	}

	write_key_data = malloc(CHIPID_KEYLEN);
	if (!write_key_data) {
		printf("malloc write buffer %d bytes error!", CHIPID_KEYLEN);
		return EFUSE_BURN_RET_MALLOC_ERR;
	}

	hexstr_to_byte(key_hex_string, write_key_data, buf_len);

	// 1. write chipid to oem3
#ifndef XR_EFUSE_DEBUG
    ret = hal_efuse_write_ext(1664, 128, write_key_data);
	if (ret) {
		printf("efuse write chipid error: %d\n", ret);
		ret = EFUSE_BURN_RET_WRITE_ERR;
		goto out;
	} else {
		printf("efuse write chipid end\n");
	}
#endif

	// 2. read chipid
	read_key_data = malloc(CHIPID_KEYLEN);
	if (!read_key_data) {
		printf("malloc read buffer %d bytes error!", CHIPID_KEYLEN);
		ret = EFUSE_BURN_RET_MALLOC_ERR;
		goto out;
	}
#ifndef XR_EFUSE_DEBUG
	for(i = 0; i < CHIPID_KEYLEN / 4; i++) {
		*((uint32_t *)read_key_data + i) = efuse_reg_read_key(0xD0 + i * 4);
	}
#else
	memcpy(read_key_data, write_key_data, CHIPID_KEYLEN);
#endif

	printf("new xrchipID=0x");
	for(int i = 0; i < CHIPID_KEYLEN; i++) {
		printf("%02x", read_key_data[i]);
	}
	printf("\n");

out:
	if (write_key_data)
		free(write_key_data);
	if (read_key_data)
		free(read_key_data);

	return ret;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ococci_setid, xr_set_chipID, efuse read mac)


/**
 * 获取MAC地址
*/
int cmd_ococci_readmac(int argc, char **argv)
{
	unsigned char read_mac_buf[HEXMAC_BYTE] = {0};
	hal_efuse_read_mac(read_mac_buf, 1);
	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ococci_readmac, xr_get_mac, efuse read mac)

/**
 * 设置MAC地址
*/
int cmd_ococci_setmac(int argc, char **argv)
{
	if (argc != 2) {
		printf("input error!\n");
		return EFUSE_BURN_RET_ARG_ERR;
	}
	return hal_efuse_write_mac(argv[1], 1);
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ococci_setmac, xr_set_mac, efuse read mac)
