/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
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

/*
 * gpadc power
 */
#include <stdlib.h>
#include <math.h>
#include <sunxi_hal_power.h>
#include <sunxi_hal_power_private.h>
#include "gpadc_power.h"
#include "type.h"
#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#endif
#include <hal_interrupt.h>
#include <hal_atomic.h>
#include <hal_clk.h>
#include <hal_sem.h>
#include <hal_timer.h>
#include <sunxi_hal_gpadc.h>
#include <sunxi_hal_efuse.h>

// #define GPADC_POWER_CHANNEL 8
static gpadc_callback_t s_chg_plg_callback = NULL;
/**
 *   电源       实际测量值 (mV) = gpadc_read_channel_data(GPADC_POWER_CHANNEL) * 3
 * -------------------------------------------------------------------------------
 *  4.20V                       <= 4150 左右
 *  4.00V                       <= 3930 左右
 *  3.70V                       <= 3640 左右
 *  3.20V                       <= 3144 左右
 */
static inline float votage_2_capacity(float votage)
{
    static float res = 0;
    const float coeff[2][12] = {
        // { 3.00, 3.45, 3.68, 3.74, 3.77, 3.79, 3.82, 3.87, 3.92, 3.98, 4.06, 4.20 },
        // { 0.0,  0.05, 0.1,  0.2,  0.3,  0.4,  0.5,  0.6,  0.7,  0.8,  0.9,  1.0 }
        { 3.25, 3.37, 3.65, 3.71, 3.74, 3.76, 3.79, 3.84, 3.89, 3.95, 4.02, 4.08 },
        { 0.0,  0.05, 0.1,  0.2,  0.3,  0.4,  0.5,  0.6,  0.7,  0.8,  0.9,  1.0 }
    };

    if ((votage >= 3.0) && (votage <= 4.3)) {
        int i;
        for (i = 11; (i > 0) && (votage < coeff[0][i]); i--) {
        }
        res = coeff[1][i];
    }
    return res;
}

/* battery ops */
static int gpadc_get_rest_cap(struct power_dev *rdev)
{
    uint32_t vol_data;
#ifdef CONFIG_ARCH_SUN20IW2
    if (hal_efuse_get_chip_ver() <= CHIP_VER_C)
        vol_data = gpadc_read_channel_data_filter(GPADC_POWER_CHANNEL) * 3;
    else
        vol_data = gpadc_read_channel_data(GPADC_POWER_CHANNEL) * 3;
#else
    vol_data = gpadc_read_channel_data(GPADC_POWER_CHANNEL) * 3;
#endif
    float cap = votage_2_capacity((float)vol_data / 1000.0f);
    // pr_info("channel %d vol data is %u cap=%f\n", GPADC_POWER_CHANNEL, vol_data, cap);
    // 0-100
    return (int)(cap*100.0);
}

static int gpadc_get_bat_present(struct power_dev *rdev )
{
	return 1; // battery detected
}

static int gpadc_get_direction(struct power_dev *rdev)
{
    return 1; // battery discharge
}

static int gpadc_get_bat_status(struct power_dev *rdev)
{
	bool bat_det;
	unsigned int rest_cap;

	bat_det = gpadc_get_bat_present(rdev);

	rest_cap = gpadc_get_rest_cap(rdev);

	if (bat_det) {
		if (rest_cap == 100)
			return POWER_SUPPLY_STATUS_FULL;
    }
    return POWER_SUPPLY_STATUS_UNKNOWN;
}

static int gpadc_get_bat_health(struct power_dev *rdev)
{
	return POWER_SUPPLY_HEALTH_GOOD;
}

static int gpadc_cmpfunc(const void *gpadc_a, const void *gpadc_b)
{
    return (*(int *)gpadc_a - *(int *)gpadc_b);
}

#if 0
static int gpadc_get_vbat(struct power_dev *rdev)
{
    uint32_t vol_data;
#ifdef CONFIG_ARCH_SUN20IW2
    if (hal_efuse_get_chip_ver() <= CHIP_VER_C)
        vol_data = gpadc_read_channel_data_filter(GPADC_POWER_CHANNEL) * 3;
    else
        vol_data = gpadc_read_channel_data(GPADC_POWER_CHANNEL) * 3;
#else
    vol_data = gpadc_read_channel_data(GPADC_POWER_CHANNEL) * 3;
#endif
    // pr_info("channel %d vol data is %u\n", GPADC_POWER_CHANNEL, vol_data);
    return (int)vol_data;
}
#else
static int gpadc_get_vbat(struct power_dev *rdev)
{
    uint32_t adc_data;
	int compensate;
	static uint32_t adc_data_t[5], cnt = 0;
	adc_data = gpadc_read_channel_adc_double_filter(GPADC_POWER_CHANNEL);
	adc_data_t[cnt] = adc_data;
	cnt++;
	if(cnt >= 5){
		cnt = 0;
		qsort(adc_data_t, 5, sizeof(uint32_t), gpadc_cmpfunc);
		if(adc_data_t[4] - adc_data_t[0] >= 100){
			compensate = (adc_data_t[2] * 5) - (adc_data_t[0] + adc_data_t[1] + adc_data_t[2] + adc_data_t[3] + adc_data_t[4]);
			return adc_data_t[4] + compensate;
		}
	}
    // pr_info("channel %d vol data is %u\n", GPADC_POWER_CHANNEL, vol_data);
    return (int)adc_data;
}
#endif

static int gpadc_get_bat_temp(struct power_dev *rdev)
{
	return 26;
}

/* usb ops */
static int gpadc_read_vbus_state(struct power_dev *rdev)
{
    gpio_data_t data;

    /**
     * 返回值
     * 0: USB 充电线未插入
     * 1: USB 充电线已插入
     */
#if (0xffff != VBUS_DETECT_IO)
    if (0 == hal_gpio_get_data(VBUS_DETECT_IO, &data)) {
        return (GPIO_DATA_HIGH == data) ? POWER_SUPPLY_STATUS_CHARGING : POWER_SUPPLY_STATUS_NOT_CHARGING;
    } else {
        return POWER_SUPPLY_STATUS_NOT_CHARGING;
    }
#elif (0xffff != VBUS_DETECT_ADC)
    uint32_t adc = 0;
    uint16_t cnt = 5;

    for (uint16_t i = 0; i < cnt; i++) {
        adc += gpadc_read_channel_data(VBUS_DETECT_ADC);
        hal_msleep(10);
    }    
    adc = adc / cnt;
    // printf("vbus detect adc: %d.\n", adc);

    if (abs(adc - VBUS_DETECT_NO_CHARGE_VAL) <= 60) {
        return POWER_SUPPLY_STATUS_NOT_CHARGING;
    } else if (abs(adc - VBUS_DETECT_FULL_VAL) <= 60) {
        return POWER_SUPPLY_STATUS_FULL;
    } else {
        return POWER_SUPPLY_STATUS_CHARGING;
    }
#else
    return POWER_SUPPLY_STATUS_NOT_CHARGING;
#endif
}

struct bat_power_ops gpadc_bat_power_ops = {
	.get_rest_cap         = gpadc_get_rest_cap, //0-100
	.get_coulumb_counter  = NULL,
	.get_bat_present      = gpadc_get_bat_present, // use batery on[1] off[0]
	.get_bat_online       = gpadc_get_direction,
	.get_bat_status       = gpadc_get_bat_status, // ok
	.get_bat_health       = gpadc_get_bat_health, // ok
	.get_vbat             = gpadc_get_vbat, // 3300 - 4200mV
	.get_ibat             = NULL,
	.get_disibat          = NULL,
	.get_temp             = gpadc_get_bat_temp, // 26°
	.get_temp_ambient     = gpadc_get_bat_temp, // 26°
	.set_chg_cur          = NULL,
	.set_chg_vol          = NULL,
	.set_batfet           = NULL,
};

struct usb_power_ops gpadc_usb_power_ops = {
	.get_usb_status  = gpadc_read_vbus_state,
	.get_usb_ihold   = NULL,
	.get_usb_vhold   = NULL,
	.set_usb_ihold   = NULL,
	.set_usb_vhold   = NULL,

#if defined(CONFIG_AXP2585_TYPE_C)
	.get_cc_status     = NULL,
#endif
};

extern void gpadc_channel_enable_lowirq(hal_gpadc_channel_t channal);
extern void gpadc_channel_disable_lowirq(hal_gpadc_channel_t channal);

static int vbus_adc_irq_cb(uint32_t data_type, uint32_t data)
{
//    printf("data_type = %d, cur adc: %d => %d mV.", data_type, data, (((2500000UL) / 4096) * data) / 1000);

    if ((data_type == GPADC_UP) || (0 == data)) {
        gpadc_key_disable_highirq(VBUS_DETECT_ADC);
        gpadc_channel_enable_lowirq(VBUS_DETECT_ADC);
    } else {
        gpadc_channel_disable_lowirq(VBUS_DETECT_ADC);
        gpadc_key_enable_highirq(VBUS_DETECT_ADC);
#ifdef CONFIG_COMPONENTS_PM
        hal_gpadc_report_wakeup_event(VBUS_DETECT_ADC);
#endif
    }
    if (NULL != s_chg_plg_callback) {
        s_chg_plg_callback(data_type, data);
    }
    return 0;
}

/* 设置充电插拔回调 */
int gpadc_set_vbus_chg_plg_cb(gpadc_callback_t user_callback)
{
//    printf("[%s] vbus chg plg cb register.\n",__FUNCTION__);
    s_chg_plg_callback = user_callback;
    return 0;
}

/* init chip & power setting*/
int gpadc_init_power(struct power_dev *rdev)
{
    int ret;
    ret = hal_gpadc_init();
    if (ret) {
        pr_err("pmu type simple gpadc!\n");
        return -1;
    }
    hal_gpadc_channel_init(GPADC_POWER_CHANNEL);
	pr_info("gpadc init finished !\n");

    /* vbus detect io init */
#if (0xffff != VBUS_DETECT_IO)
    hal_gpio_pinmux_set_function(VBUS_DETECT_IO, GPIO_MUXSEL_IN);
    hal_gpio_set_direction(VBUS_DETECT_IO, GPIO_DIRECTION_INPUT);
    hal_gpio_set_pull(VBUS_DETECT_IO, GPIO_PULL_DOWN);
#endif

#if (0xffff != VBUS_DETECT_ADC)
    if (GPADC_OK == hal_gpadc_channel_init(VBUS_DETECT_ADC)) {
        gpadc_channel_compare_lowdata(VBUS_DETECT_ADC, VBUS_ADC_COMPARE_LOWDATA);
        gpadc_channel_compare_highdata(VBUS_DETECT_ADC, VBUS_ADC_COMPARE_HIGDATA);
        hal_msleep(4);
    }
    hal_gpadc_register_callback(VBUS_DETECT_ADC, vbus_adc_irq_cb);
#endif

    return 0;
}

/* get power ops*/
int gpadc_get_power(struct power_dev *rdev)
{
	int err;
	u8 chip_id = 0;
	static int pmu_type = -1;

	if (pmu_type == -1) {
        pmu_type = AXP2585_UNKNOWN;
        pr_info("pmu type simple gpadc!\n");
	}

	// rdev->config->pmu_version = pmu_type;

	rdev->bat_ops = &gpadc_bat_power_ops;
	rdev->usb_ops = &gpadc_usb_power_ops;

	return GPADC_POWER;
}
