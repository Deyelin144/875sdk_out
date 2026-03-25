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

#ifndef _PAL_EFPG_H
#define _PAL_EFPG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	PAL_EFPG_FIELD_HOSC = 0,    /* data buffer size: 1  byte  */
	PAL_EFPG_FIELD_BOOT,        /* data buffer size: 32 bytes */
	PAL_EFPG_FIELD_MAC_WLAN,    /* data buffer size: 6  bytes */
	PAL_EFPG_FIELD_DCXO,        /* data buffer size: 1  byte  */
	PAL_EFPG_FIELD_POUT_WLAN,   /* data buffer size: 3  bytes */
	PAL_EFPG_FIELD_CHIPID,      /* data buffer size: 16 bytes */
	PAL_EFPG_FIELD_UA,          /* data buffer size: V1(1447~2047) : V2(765~1023 bit) : V3(954~1022 bit)*/
	PAL_EFPG_FIELD_POUT_BT,     /* data buffer size: 3  bytes */
	PAL_EFPG_FIELD_MAC_BT,      /* data buffer size: 6  bytes */
	PAL_EFPG_FIELD_SECRETKEY,   /* data buffer size: 16 bytes */
	PAL_EFPG_FIELD_SECURESWD,   /* data buffer size: 1  bytes */
	PAL_EFPG_FIELD_ALL,         /* data buffer size: 128 bytes */
	PAL_EFPG_FIELD_NUM,
} pal_efpg_field_t;

int PalEfpgRead(pal_efpg_field_t field, uint8_t *data);

int PalEfpgReadUa(uint32_t start, uint32_t num, uint8_t *data);

int PalEfpgReadAll(uint32_t start, uint32_t num, uint8_t *data);

#ifdef __cplusplus
};
#endif

#endif

