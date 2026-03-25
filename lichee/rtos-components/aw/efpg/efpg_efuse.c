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

#include "efpg_i.h"
#include "efpg_debug.h"
#include "sunxi_hal_efuse.h"

static int efpg_boot_hash_cmp(const uint8_t *data, const uint8_t *buf, uint8_t *err_cnt,
                              uint8_t *err_1st_no, uint8_t *err_2nd_no)
{
	uint8_t byte_cnt;
	uint8_t bit_cnt;

	if (efpg_memcmp(data, buf, EFPG_BOOT_BUF_LEN) == 0)
		return 0;

	*err_cnt = 0;
	for (byte_cnt = 0; byte_cnt < EFPG_BOOT_BUF_LEN; byte_cnt++) {
		if (data[byte_cnt] == buf[byte_cnt])
			continue;

		for (bit_cnt = 0; bit_cnt < 8; bit_cnt++) {
			if ((data[byte_cnt] & (0x1 << bit_cnt)) == (buf[byte_cnt] & (0x1 << bit_cnt)))
				continue;

			if (*err_cnt == 0) {
				*err_1st_no = (byte_cnt << 3) + bit_cnt;
				*err_cnt = 1;
			} else if (*err_cnt == 1) {
				*err_2nd_no = (byte_cnt << 3) + bit_cnt;
				*err_cnt = 2;
			} else {
				return -1;
			}
		}
	}

	return 0;
}

typedef enum efpg_region_mode {
	EFPG_REGION_READ,
	EFPG_REGION_WRITE,
} efpg_region_mode_t;

typedef struct efpg_region_info {
	uint16_t flag_start;
	uint16_t flag_bits;		/* MUST less than 32-bit */
	uint16_t data_start;
	uint16_t data_bits;
	uint8_t *buf;			/* temp buffer for write to save read back data */
	uint8_t  buf_len;		/* MUST equal to ((data_bits + 7) / 8) */
} efpg_region_info_t;

#define EFPG_BITS_TO_BYTE_CNT(bits)		(((bits) + 7) / 8)

static uint16_t efpg_read_region(efpg_region_info_t *info, uint8_t *data)
{
	uint8_t idx = 0;
	uint8_t flag;
	uint32_t start_bit;

	/* flag */
	if (hal_efuse_read_ext(info->flag_start, info->flag_bits, (uint8_t *)&flag) != HAL_OK) {
		return EFPG_ACK_RW_ERR;
	}
	flag &= ((1 << info->flag_bits) - 1);
	EFPG_DBG("r start %d, bits %d, flag 0x%x\n", info->flag_start, info->flag_bits, flag);
	if ((flag == 0) || (flag > ((1 << info->flag_bits) - 1))) {
		EFPG_WARN("%s(), flag (%d, %d) = 0x%x is invalid\n", __func__, info->flag_start, info->flag_bits, flag);
		return EFPG_ACK_NODATA_ERR;
	}
	while ((flag & 0x1) != 0) {
		flag = flag >> 1;
		idx++;
	}
	start_bit = info->data_start + (idx - 1) * info->data_bits;

	/* data */
	EFPG_DBG("r data, start %d, bits %d\n", start_bit, info->data_bits);
	if (hal_efuse_read_ext(start_bit, info->data_bits, data) != HAL_OK) {
		return EFPG_ACK_RW_ERR;
	}

	return EFPG_ACK_OK;
}

static uint16_t efpg_write_region(efpg_region_info_t *info, uint8_t *data)
{
	uint8_t idx = 0;
	uint8_t flag;
	uint32_t start_bit;
	uint8_t w_tmp;

	/*read flag */
	if (hal_efuse_read_ext(info->flag_start, info->flag_bits, (uint8_t *)&flag) != HAL_OK) {
		return EFPG_ACK_RW_ERR;
	}
	flag &= ((1 << info->flag_bits) - 1);
	EFPG_DBG("w start %d, bits %d, flag 0x%x\n", info->flag_start, info->flag_bits, flag);
	efpg_memset(info->buf, 0, info->buf_len);
	if (flag < (1 << info->flag_bits)) {
		if (flag == 0) {
			idx = 0;
			w_tmp = 1 << idx;
			if ((hal_efuse_write_ext(info->flag_start, info->flag_bits, &w_tmp) != HAL_OK) ||
			    (hal_efuse_read_ext(info->flag_start, info->flag_bits, info->buf) != HAL_OK) ||
			    (w_tmp != (info->buf[0] & w_tmp))) {
				return EFPG_ACK_RW_ERR;
			}
		} else {
			while ((flag & 0x1) != 0) {
				flag = flag >> 1;
				idx++;
			}
			idx--;
		}

		while (idx < info->flag_bits) {
			EFPG_DBG("idx:%d\n", idx);
			/* will try compare old data with new */
			start_bit = info->data_start + idx * info->data_bits;
			efpg_memset(info->buf, 0, info->buf_len);
			if (hal_efuse_read_ext(start_bit, info->data_bits, info->buf) == HAL_OK) {
				if (info->data_bits % 8) {
					info->buf[info->buf_len - 1] &= ((1 << (info->data_bits % 8)) - 1);
				}
				int i;
				for (i = 0; i < info->buf_len; i++) {
					if (((info->buf[i] ^ data[i]) & info->buf[i]) != 0) {
						break;
					}
				}
				if (i == info->buf_len) {
					efpg_memset(info->buf, 0, info->buf_len);
					EFPG_DBG("w start %d, bits %d\n", start_bit, info->data_bits);
					if ((hal_efuse_write_ext(start_bit, info->data_bits, data) == HAL_OK) &&
					    (hal_efuse_read_ext(start_bit, info->data_bits, info->buf) == HAL_OK) &&
					    (efpg_memcmp(data, info->buf, info->buf_len) == 0)) {
						return EFPG_ACK_OK;
					}
				}
			}

			if (++idx > info->flag_bits - 1) {
				EFPG_WARN("region has no space\n");
				return EFPG_ACK_DI_ERR;
			}
			/* update flag as next */
			w_tmp = 1 << idx;
			efpg_memset(info->buf, 0, info->buf_len);
			EFPG_DBG("w start %d, bits %d, w_tmp 0x%x\n", info->flag_start, info->flag_bits, w_tmp);
			if ((hal_efuse_write_ext(info->flag_start, info->flag_bits, &w_tmp) != HAL_OK) ||
			    (hal_efuse_read_ext(info->flag_start, info->flag_bits, info->buf) != HAL_OK) ||
			    (w_tmp != (info->buf[0] & w_tmp))) {
				EFPG_WARN("fail to write flag\n");
				return EFPG_ACK_RW_ERR;
			}
		}
	}

	EFPG_WARN("flag (%d, %d) = 0x%x is invalid\n", info->flag_start, info->flag_bits, flag);
	return EFPG_ACK_NODATA_ERR;
}

static uint16_t efpg_rw_dcxo(efpg_region_mode_t mode, uint8_t *data)
{
#if (EFPG_DCXO_TRIM_NUM == 0)
	EFPG_DBG("no DCXO TRIM defined!\n");
	return EFPG_ACK_NODATA_ERR;
#else
	efpg_region_info_t info;

	info.flag_start = EFPG_DCXO_TRIM_FLAG_START;
	info.flag_bits = EFPG_DCXO_TRIM_FLAG_NUM;
	info.data_start = EFPG_DCXO_TRIM1_START;
	info.data_bits = EFPG_DCXO_TRIM1_NUM;

	if (mode == EFPG_REGION_WRITE) {
		uint8_t buf[EFPG_BITS_TO_BYTE_CNT(EFPG_DCXO_TRIM_LEN)];
		info.buf = buf;
		info.buf_len = sizeof(buf);
		if (info.data_bits == (info.buf_len * 8))
			return efpg_write_region(&info, data);
		else {
			EFPG_ERR("Enter wrong data! correct data length:%d bit\n", info.data_bits);
			return EFPG_ACK_RW_ERR;
		}
	} else {
		info.buf = NULL;
		info.buf_len = 0;
		return efpg_read_region(&info, data);
	}
#endif
}

uint16_t efpg_read_dcxo(uint8_t *data)
{
	EFPG_DBG("%s()\n", __func__);
	return efpg_rw_dcxo(EFPG_REGION_READ, data);
}

uint16_t efpg_write_dcxo(uint8_t *data)
{
	EFPG_DBG("%s()\n", __func__);
	return efpg_rw_dcxo(EFPG_REGION_WRITE, data);
}

static uint16_t efpg_rw_pout(efpg_region_mode_t reg_mode, efpg_pout_cal_mode_t pout_mode,uint8_t *data)
{
	efpg_region_info_t info;
	if (pout_mode == EFPG_POUT_WLAN) {
#if (EFPG_POUT_CAL_FLAG_NUM == 0)
		EFPG_DBG("no POUT CAL defined!\n");
		return EFPG_ACK_NODATA_ERR;
#else
		info.flag_start = EFPG_POUT_CAL_FLAG_START;
		info.flag_bits = EFPG_POUT_CAL_FLAG_NUM;
		info.data_start = EFPG_POUT_CAL1_START;
		info.data_bits = EFPG_POUT_CAL1_NUM;
#endif
	} else if (pout_mode == EFPG_POUT_BT) {
#if (EFPG_BT_POUT_CAL_FLAG_NUM == 0)
		EFPG_DBG("no BT POUT CAL defined!\n");
		return EFPG_ACK_NODATA_ERR;
#else
		info.flag_start = EFPG_BT_POUT_CAL_FLAG_START;
		info.flag_bits = EFPG_BT_POUT_CAL_FLAG_NUM;
		info.data_start = EFPG_BT_POUT_CAL1_START;
		info.data_bits = EFPG_BT_POUT_CAL1_NUM;
#endif
	} else {
		EFPG_WARN("%s(), mode = %d is invalid\n", __func__, pout_mode);
		return EFPG_ACK_RW_ERR;
	}

	if (reg_mode == EFPG_REGION_WRITE) {
		uint8_t buf[EFPG_BITS_TO_BYTE_CNT(EFPG_POUT_CAL_LEN)];
		info.buf = buf;
		info.buf_len = sizeof(buf);
		return efpg_write_region(&info, data);
	} else {
		info.buf = NULL;
		info.buf_len = 0;
		return efpg_read_region(&info, data);
	}
}

uint16_t efpg_read_pout(efpg_pout_cal_mode_t pout_mode, uint8_t *data)
{
	EFPG_DBG("%s(), mode:%d\n", __func__, pout_mode);
	return efpg_rw_pout(EFPG_REGION_READ, pout_mode, data);
}

uint16_t efpg_write_pout(efpg_pout_cal_mode_t pout_mode, uint8_t *data)
{
	EFPG_DBG("%s(), mode:%d\n", __func__, pout_mode);
	return efpg_rw_pout(EFPG_REGION_WRITE, pout_mode, data);
}

static uint16_t efpg_rw_mac(efpg_region_mode_t reg_mode, efpg_mac_mode_t mac_mode,uint8_t *data)
{
	efpg_region_info_t info;
	if (mac_mode == EFPG_MAC_WLAN) {
#if (EFPG_MAC_WLAN_ADDR_NUM == 0)
		EFPG_DBG("no MAC WLAN ADDR defined!\n");
		return EFPG_ACK_NODATA_ERR;
#else
		info.flag_start = EFPG_MAC_WLAN_ADDR_FLAG_START;
		info.flag_bits = EFPG_MAC_WLAN_ADDR_FLAG_NUM;
		info.data_start = EFPG_MAC_WLAN_ADDR1_START;
		info.data_bits = EFPG_MAC_WLAN_ADDR1_NUM;
#endif
	} else if (mac_mode == EFPG_MAC_BT) {
#if (EFPG_MAC_BT_ADDR_NUM == 0)
		EFPG_DBG("no MAC BT ADDR defined!\n");
		return EFPG_ACK_NODATA_ERR;
#else
		info.flag_start = EFPG_MAC_BT_ADDR_FLAG_START;
		info.flag_bits = EFPG_MAC_BT_ADDR_FLAG_NUM;
		info.data_start = EFPG_MAC_BT_ADDR1_START;
		info.data_bits = EFPG_MAC_BT_ADDR1_NUM;
#endif
	} else {
		EFPG_WARN("%s(), mode = %d is invalid\n", __func__, mac_mode);
		return EFPG_ACK_RW_ERR;
	}

	if (reg_mode == EFPG_REGION_WRITE) {
		uint8_t buf[EFPG_BITS_TO_BYTE_CNT(EFPG_MAC_WLAN_ADDR_LEN)];
		info.buf = buf;
		info.buf_len = sizeof(buf);
		if (info.data_bits == (info.buf_len * 8))
			return efpg_write_region(&info, data);
		else {
			EFPG_ERR("Enter wrong data! correct data length:%d bit\n", info.data_bits);
			return EFPG_ACK_RW_ERR;
		}
	} else {
		info.buf = NULL;
		info.buf_len = 0;
		return efpg_read_region(&info, data);
	}
}

uint16_t efpg_read_mac(efpg_mac_mode_t mac_mode, uint8_t *data)
{
	uint16_t ret;
	EFPG_DBG("%s(), mode:%d\n", __func__, mac_mode);
	ret = efpg_rw_mac(EFPG_REGION_READ, mac_mode, data);
	/*
	if ((mac_mode == EFPG_MAC_WLAN) && (ret == EFPG_ACK_NODATA_ERR)) {
		return efpg_rw_first_wlan_mac(EFPG_REGION_READ, data);
	}*/
	return ret;
}

uint16_t efpg_write_mac(efpg_mac_mode_t mac_mode, uint8_t *data)
{
	uint16_t ret;
	EFPG_DBG("%s(), mode:%d\n", __func__, mac_mode);
	if (mac_mode == EFPG_MAC_WLAN) {
		uint8_t r_mac[6];
		ret = efpg_rw_mac(EFPG_REGION_READ, mac_mode, r_mac);
		/* do not write mac area if we already have data in wlan mac area. */
		/*
		if ((ret == EFPG_ACK_NODATA_ERR) &&
			(EFPG_ACK_OK == efpg_rw_first_wlan_mac(EFPG_REGION_WRITE, data))) {
			return EFPG_ACK_OK;
		}*/
	}
	return efpg_rw_mac(EFPG_REGION_WRITE, mac_mode, data);
}

uint16_t efpg_read_all_field(uint16_t start, uint16_t num, uint8_t *data)
{
	if (hal_efuse_read_ext(start, num, data) != HAL_OK)
		return EFPG_ACK_RW_ERR;

	return EFPG_ACK_OK;
}

uint16_t efpg_read_user_area(uint16_t start, uint16_t num, uint8_t *data)
{
	if ((start >= EFPG_USER_AREA_NUM) ||
	    (num == 0) ||
	    (num > EFPG_USER_AREA_NUM) ||
	    (start + num > EFPG_USER_AREA_NUM)) {
		EFPG_ERR("start %d, num %d\n", start, num);
		return EFPG_ACK_RW_ERR;
	}

	if (hal_efuse_read_ext(start + EFPG_USER_AREA_START, num, data) != HAL_OK) {
		EFPG_ERR("eFuse read failed\n");
		return EFPG_ACK_RW_ERR;
	}

	return EFPG_ACK_OK;
}

uint16_t efpg_write_user_area(uint16_t start, uint16_t num, uint8_t *data)
{
	if ((start >= EFPG_USER_AREA_NUM) ||
	    (num == 0) ||
	    (num > EFPG_USER_AREA_NUM) ||
	    (start + num > EFPG_USER_AREA_NUM)) {
		EFPG_ERR("start %d, num %d\n", start, num);
		return EFPG_ACK_RW_ERR;
	}

	if (hal_efuse_write_ext(start + EFPG_USER_AREA_START, num, data) != HAL_OK) {
		EFPG_ERR("eFuse write failed\n");
		return EFPG_ACK_RW_ERR;
	}

	return EFPG_ACK_OK;
}

uint16_t efpg_read_field(efpg_field_t field, uint8_t *data, uint16_t start_bit_addr, uint16_t bit_len)
{
	switch (field) {
	case EFPG_FIELD_DCXO:
		return efpg_read_dcxo(data);
	case EFPG_FIELD_POUT_WLAN:
		return efpg_read_pout(EFPG_POUT_WLAN, data);
	case EFPG_FIELD_POUT_BT:
		return efpg_read_pout(EFPG_POUT_BT, data);
	case EFPG_FIELD_MAC_WLAN:
		return efpg_read_mac(EFPG_MAC_WLAN, data);
	case EFPG_FIELD_MAC_BT:
		return efpg_read_mac(EFPG_MAC_BT, data);
	case EFPG_FIELD_ALL:
		return efpg_read_all_field(start_bit_addr, bit_len, data);
	case EFPG_FIELD_UA:
		return efpg_read_user_area(start_bit_addr, bit_len, data);
	default:
		EFPG_WARN("%s(), %d, read field %d\n", __func__, __LINE__, field);
		return EFPG_ACK_RW_ERR;
	}
}


uint16_t efpg_write_field(efpg_field_t field, uint8_t *data, uint16_t start_bit_addr, uint16_t bit_len)
{
	switch (field) {
	case EFPG_FIELD_POUT_WLAN:
		return efpg_write_pout(EFPG_POUT_WLAN, data);
	case EFPG_FIELD_POUT_BT:
		return efpg_write_pout(EFPG_POUT_BT, data);
	case EFPG_FIELD_MAC_WLAN:
		return efpg_write_mac(EFPG_MAC_WLAN, data);
	case EFPG_FIELD_MAC_BT:
		return efpg_write_mac(EFPG_MAC_BT, data);
	case EFPG_FIELD_UA:
		return efpg_write_user_area(start_bit_addr, bit_len, data);
	default:
		EFPG_WARN("%s(), %d, write field %d\n", __func__, __LINE__, field);
		return EFPG_ACK_RW_ERR;
	}
}
