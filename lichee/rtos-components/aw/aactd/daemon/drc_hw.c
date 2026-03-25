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

#include "drc_hw.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "aactd/common.h"
#include "aactd/communicate.h"
#include "local.h"
#ifdef CONFIG_DRIVERS_SOUND
#include "snd_sunxi_alg_cfg.h"
#elif CONFIG_DRIVERS_SOUND_V2
#include "sunxi_sound_alg_cfg.h"
#endif

#if defined(CONFIG_ARCH_SUN20IW2)

static struct aactd_com_drc_hw_long_data local_data;

#elif defined(CONFIG_ARCH_SUN8IW18) || defined(CONFIG_ARCH_SUN8IW19) || defined(CONFIG_ARCH_SUN8IW20)

static struct aactd_com_drc_hw_data local_data;

#endif

int drc_hw_local_init(void)
{
    int ret;

    local_data.reg_num = 0;
    local_data.reg_args = malloc(com_buf_len_max);
    if (!local_data.reg_args) {
        aactd_error("No memory\n");
        ret = -ENOMEM;
        goto err_out;
    }

    return 0;

free_local_data:
    free(local_data.reg_args);
    local_data.reg_args = NULL;
err_out:
    return ret;
}

int drc_hw_local_release(void)
{
    if (local_data.reg_args) {
        free(local_data.reg_args);
        local_data.reg_args = NULL;
    }
    return 0;
}

#if defined(CONFIG_ARCH_SUN20IW2)

static int drc_hw_data_set(const struct aactd_com_drc_hw_long_data *data)
{
    int ret = -1;
	int i;
	struct alg_cfg_reg_domain *domain;
	enum SUNXI_ALG_CFG_DOMAIN domain_sel;
	int reg_1db_cnt = 0;
	int reg_3db_cnt = 0;


	for (i = 0; i < data->reg_num; ++i) {

		if (data->reg_args[i].offset >= DRC1B_REG_MIN &&
			data->reg_args[i].offset <= DRC1B_REG_MAX) {
			reg_1db_cnt++;
		} else if (data->reg_args[i].offset >= DRC3B_REG_MIN &&
			data->reg_args[i].offset <= DRC3B_REG_MAX) {
			reg_3db_cnt++;
		} else {
			aactd_info("reg: 0x%04lx", (long)data->reg_args[i].offset);
			continue;
		}
	}


	if (reg_1db_cnt == data->reg_num)
		domain_sel = SUNXI_ALG_CFG_DOMAIN_1BDRC;
	else if (reg_3db_cnt == data->reg_num)
		domain_sel = SUNXI_ALG_CFG_DOMAIN_3BDRC;
	else {
		aactd_error("reg_1db_cnt %d is not equal to reg_num %d", reg_1db_cnt, data->reg_num);
		aactd_error("reg_3db_cnt %d is not equal to reg_num %d", reg_3db_cnt, data->reg_num);
		goto err;
	}

#ifdef CONFIG_DRIVERS_SOUND
		ret = sunxi_alg_cfg_domain_get(&domain, domain_sel);
		if (ret) {
			aactd_error("alg_cfg domain get failed");
			goto err;
		}

		for (i = 0; i < data->reg_num; ++i) {
			sunxi_alg_cfg_set(data->reg_args[i].offset, data->reg_args[i].value, domain);
		}

#elif CONFIG_DRIVERS_SOUND_V2
		ret = sunxi_sound_alg_cfg_domain_get(&domain, domain_sel);
		if (ret) {
			aactd_error("alg_cfg domain get failed");
			goto err;
		}

		for (i = 0; i < data->reg_num; ++i) {
			sunxi_sound_alg_cfg_set(data->reg_args[i].offset, data->reg_args[i].value, domain);
		}
#endif



err:

    return ret;
}


#elif defined(CONFIG_ARCH_SUN8IW18) || defined(CONFIG_ARCH_SUN8IW19) || defined(CONFIG_ARCH_SUN8IW20)


static int drc_hw_data_set(const struct aactd_com_drc_hw_data *data)
{
    int ret = 0;
    int i;
    uint32_t reg_addr;
	int reg_dac_drc_cnt = 0;
	int reg_adc_drc_cnt = 0;

	for (i = 0; i < data->reg_num; ++i) {

		if (data->reg_args[i].offset >= DAC_DRC_REG_MIN &&
			data->reg_args[i].offset <= DAC_DRC_REG_MAX) {
			reg_dac_drc_cnt++;
		} else if (data->reg_args[i].offset >= ADC_DRC_REG_MIN &&
			data->reg_args[i].offset <= ADC_DRC_REG_MIN) {
			reg_adc_drc_cnt++;
		} else {
			aactd_info("reg: 0x%04lx", (long)data->reg_args[i].offset);
			continue;
		}
	}

	if (reg_dac_drc_cnt != data->reg_num && reg_adc_drc_cnt != data->reg_num)
	{
		aactd_error("reg_dac_drc_cnt %d is not equal to reg_num %d", reg_dac_drc_cnt, data->reg_num);
		aactd_error("reg_adc_drc_cnt %d is not equal to reg_num %d", reg_adc_drc_cnt, data->reg_num);
	}


    for (i = 0; i < data->reg_num; ++i) {

        reg_addr = DRC_HW_REG_BASE_ADDR + data->reg_args[i].offset;

		AACTD_DEBUG(2, "reg_base -> 0x%x, reg -> 0x%04x, reg_val -> 0x%08x\n",
				DRC_HW_REG_BASE_ADDR, reg_addr, data->reg_args[i].value);

		drchw_writel(data->reg_args[i].value, reg_addr);
    }

out:
    return ret;
}
#endif

int drc_hw_write_com_to_local(const struct aactd_com *com)
{
    int ret;

#if defined(CONFIG_ARCH_SUN20IW2)
	if (com->header.flag == AACTD_COM_HEADER_FLAG_V2) {
	    ret = aactd_com_drc_hw_buf_to_long_data(com->data, &local_data);
	    if (ret < 0) {
	        aactd_error("aactd_com_drc_hw_buf_to_data failed\n");
	        goto out;
	    }

	    ret = drc_hw_data_set(&local_data);
	    if (ret < 0) {
	        aactd_error("Failed to set DRC data\n");
	        goto out;
	    }
	} else {
		aactd_error("Header flag %x is not match\n", com->header.flag);
		goto out;
	}
#elif defined(CONFIG_ARCH_SUN8IW18) || defined(CONFIG_ARCH_SUN8IW19) || defined(CONFIG_ARCH_SUN8IW20)
	if (com->header.flag == AACTD_COM_HEADER_FLAG) {
	    ret = aactd_com_drc_hw_buf_to_data(com->data, &local_data);
	    if (ret < 0) {
	        aactd_error("aactd_com_drc_hw_buf_to_data failed\n");
	        goto out;
	    }

	    ret = drc_hw_data_set(&local_data);
	    if (ret < 0) {
	        aactd_error("Failed to set DRC data\n");
	        goto out;
	    }
	} else {
		aactd_error("Header flag %x is not match\n", com->header.flag);
		goto out;
	}
#endif
    ret = 0;
out:
    return ret;
}

