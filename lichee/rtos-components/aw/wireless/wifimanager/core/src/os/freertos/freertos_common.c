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

#include <wmg_sta.h>
#include <wifi_log.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

uint8_t char2uint8(char* trs)
{
	uint8_t ret = 0;
	uint8_t tmp_ret[2] = {0};
	int i = 0;
	for(; i < 2; i++) {
		switch (*(trs + i)) {
			case '0' :
				tmp_ret[i] = 0x0;
				break;
			case '1' :
				tmp_ret[i] = 0x1;
				break;
			case '2' :
				tmp_ret[i] = 0x2;
				break;
			case '3' :
				tmp_ret[i] = 0x3;
				break;
			case '4' :
				tmp_ret[i] = 0x4;
				break;
			case '5' :
				tmp_ret[i] = 0x5;
				break;
			case '6' :
				tmp_ret[i] = 0x6;
				break;
			case '7' :
				tmp_ret[i] = 0x7;
				break;
			case '8' :
				tmp_ret[i] = 0x8;
				break;
			case '9' :
				tmp_ret[i] = 0x9;
				break;
			case 'a' :
			case 'A' :
				tmp_ret[i] = 0xa;
				break;
			case 'b' :
			case 'B' :
				tmp_ret[i] = 0xb;
				break;
			case 'c' :
			case 'C' :
				tmp_ret[i] = 0xc;
				break;
			case 'd' :
			case 'D' :
				tmp_ret[i] = 0xd;
				break;
			case 'e' :
			case 'E' :
				tmp_ret[i] = 0xe;
				break;
			case 'f' :
			case 'F' :
				tmp_ret[i] = 0xf;
		break;
	}
	WMG_DEBUG("change num[%d]: %d\n", i, tmp_ret[i]);
	}
	ret = ((tmp_ret[0] << 4) | tmp_ret[1]);
	return ret;
}

void uint8tochar(char *mac_addr_char, uint8_t *mac_addr_uint8)
{
    sprintf(mac_addr_char,"%02x:%02x:%02x:%02x:%02x:%02x",
                        (unsigned char)mac_addr_uint8[0],
                        (unsigned char)mac_addr_uint8[1],
                        (unsigned char)mac_addr_uint8[2],
                        (unsigned char)mac_addr_uint8[3],
                        (unsigned char)mac_addr_uint8[4],
                        (unsigned char)mac_addr_uint8[5]);
    mac_addr_char[17] = '\0';
}
