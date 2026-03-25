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
#include <hal_gpio.h>
#include "sunxi-input.h"
#include "gpio_key_polled.h"

#define INPUT_DEV_NAME		"gpio-key-polled"
#define DEBOUNCE_INTERVAL	10
#define POLL_INTERVAL		50

//just for test.
static struct gpio_key_config key_config[] = {
	{GPIOH(4), 114, ACTIVE_LOW},
};

//driver data
struct gpio_button_data
{
	int gpio;
	unsigned int code;
	int active_low;
	char last_state;
};

struct gpio_key_drvdata
{
	int nbuttons;
	struct sunxi_input_dev *input_dev;
	struct gpio_button_data *bdata;
};
struct gpio_key_drvdata *key_drvdata;


void gpio_key_polled_poll(void *param)
{
	int i;
	char state;
	struct gpio_key_drvdata *key_drvdata;

	key_drvdata = (struct gpio_key_drvdata *)param;

	while(1) {
		for (i = 0; i < key_drvdata->nbuttons; i++) {
			hal_gpio_get_data(key_drvdata->bdata[i].gpio, (gpio_data_t *)&state);
			if (state != key_drvdata->bdata[i].last_state) {
				vTaskDelay(DEBOUNCE_INTERVAL/portTICK_RATE_MS);
				hal_gpio_get_data(key_drvdata->bdata[i].gpio, (gpio_data_t *)&state);
				if (state != key_drvdata->bdata[i].last_state) {
					input_report_key(key_drvdata->input_dev,
							key_drvdata->bdata[i].code, !!(state ^ key_drvdata->bdata[i].active_low));
					input_sync(key_drvdata->input_dev);

					key_drvdata->bdata[i].last_state = state;
				}
			}
		}
		vTaskDelay(POLL_INTERVAL/portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

static int gpio_key_polled_get_config(struct gpio_key_drvdata *key_drvdata)
{
	int i;
	int nbuttons;
	struct gpio_button_data *bdata;

	nbuttons = sizeof(key_config)/sizeof(key_config[0]);
	bdata = pvPortMalloc(sizeof(struct gpio_button_data) * nbuttons);
	if (NULL == bdata) {
		printf("button data malloc err\n");
		return -1;
	}

	for (i = 0; i < nbuttons; i++) {
		bdata[i].gpio = key_config[i].gpio;
		bdata[i].code = key_config[i].code;
		if (key_config[i].active_flag == ACTIVE_LOW)
			bdata[i].active_low = 1;
		else
			bdata[i].active_low = 0;
	}

	key_drvdata->nbuttons = nbuttons;
	key_drvdata->bdata = bdata;

	return 0;
}

int gpio_key_polled_init(void)
{
	int i;
	char state;
	int ret = -1;
	struct gpio_button_data *bdata;
	struct sunxi_input_dev *input_dev;

	key_drvdata = pvPortMalloc(sizeof(struct gpio_key_drvdata));
	if (NULL == key_drvdata) {
		printf("button data malloc err\n");
		return -1;
	}

	ret = gpio_key_polled_get_config(key_drvdata);
	if (ret < 0)
		goto err1;

	for (i = 0; i < key_drvdata->nbuttons; i++) {
		hal_gpio_set_direction(key_drvdata->bdata[i].gpio, GPIO_DIRECTION_INPUT);
		hal_gpio_get_data(key_drvdata->bdata[i].gpio, (gpio_data_t *)&state);
		key_drvdata->bdata[i].last_state = state;
	}

	//input dev init
	input_dev = sunxi_input_allocate_device();
	if (NULL == input_dev) {
		printf("allocate input_dev err\n");
		goto err2;
	}
	input_dev->name = INPUT_DEV_NAME;
	for (i = 0; i < key_drvdata->nbuttons; i++)
		input_set_capability(input_dev, EV_KEY, key_drvdata->bdata[i].code);

	key_drvdata->input_dev = input_dev;
	sunxi_input_register_device(key_drvdata->input_dev);


	portBASE_TYPE task_ret;
	task_ret = xTaskCreate(gpio_key_polled_poll, (signed portCHAR *) "gpio_key_poll", 256, key_drvdata, configTIMER_TASK_PRIORITY - 1, NULL);
	if (task_ret != pdPASS) {
		printf("Error creating gpio_key_polled_poll task, status was %d\n", task_ret);
		return -1;
	}

	return 0;

err2:
	vPortFree(key_drvdata->bdata);

err1:
	vPortFree(key_drvdata);
	return ret;
}
