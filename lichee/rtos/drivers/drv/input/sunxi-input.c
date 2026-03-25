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

#include <FreeRTOS.h>
#include <FreeRTOSHal.h>
#include <queue.h>
#include <semphr.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <spinlock.h>
#include "aw_list.h"
#include "sunxi-input.h"
#include "spinlock.h"
#include <hal_osal.h>

#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#endif

#include "tp/config/tp_board_config.h"
#include "tp/axs5106/tp_axs5106.h"
#include "tp/axs15231/tp_axs15231.h"

#define input_err(fmt, args...)  printf("%s()%d - "fmt, __func__, __LINE__, ##args)

#define INPUT_SEM_MAX_COUNT 0xFFFFFFFFUL

static LIST_HEAD(input_dev_list);
static LIST_HEAD(evdev_list);

static unsigned evdev_cnt = 0;
freert_spinlock_t input_lock;

static inline int is_event_support(unsigned int code,
					unsigned long *bm, unsigned int max)
{
	return code <= max && input_test_bit(code, bm);
}

static struct sunxi_input_dev *find_input_dev_by_name(const char *name)
{
	struct sunxi_input_dev *dev;

	list_for_each_entry(dev, &input_dev_list, node) {
		if(!strcmp(dev->name, name))
			return dev;
	}

	return NULL;
}

static struct sunxi_evdev *find_evdev_by_name(const char *name)
{
	struct sunxi_evdev *evdev = NULL;

	list_for_each_entry(evdev, &evdev_list, node) {
		if(!strcmp(evdev->name, name))
			return evdev;
	}

	return NULL;
}
static struct sunxi_evdev *find_evdev_by_fd(int fd)
{
	struct sunxi_evdev *evdev = NULL;

	list_for_each_entry(evdev, &evdev_list, node) {
		if(evdev->fd == fd)
			return evdev;
	}

	return NULL;
}

static void evdev_pass_event(struct sunxi_evdev *evdev, struct sunxi_input_event *event)
{
	int ret;

	evdev->buffer[evdev->head++] = *event;
	evdev->head &= EVENT_BUFFER_SIZE - 1;

	if(evdev->head == evdev->tail) {
		evdev->tail = (evdev->head - 1) & (EVENT_BUFFER_SIZE - 1);
		evdev->packet_head = evdev->tail;
	}

	if (event->type == INPUT_EVENT_SYN && event->code == INPUT_SYNC_REPORT) {
		evdev->packet_head = evdev->head;
		ret = hal_sem_post(evdev->sem);
		if (ret != HAL_OK) {
			input_err(" evdev give semaphore err\n");
		}
		if (!hal_interrupt_get_nest() && pdPASS == ret) {
			taskYIELD();
		}
	}
}

static void input_pass_event(struct sunxi_input_dev *dev, struct sunxi_input_event *event)
{
	struct sunxi_evdev *evdev = NULL;

	//report input event to all evdev (all task that read).
	list_for_each_entry(evdev, &evdev_list, node) {
		if(!strcmp(evdev->name, dev->name))
		{
			evdev_pass_event(evdev, event);
		}
	}

}

static void input_handle_event(struct sunxi_input_dev *dev,
				unsigned int type, unsigned int code, unsigned int value)
{
	bool report = false;
	struct sunxi_input_event event;

	switch (type) {
		case INPUT_EVENT_SYN :
			report = true;
			break;
		case INPUT_EVENT_KEY :
			if(is_event_support(code, dev->keybit, INPUT_KEY_MAX))
				report = true;
			break;
		case INPUT_EVENT_ABS :
			if(is_event_support(code, dev->absbit, INPUT_ABS_MAX))
				report = true;
			break;
		case INPUT_EVENT_REL :
			if(is_event_support(code, dev->relbit, INPUT_RELA_MAX))
				report = true;
			break;
		case INPUT_EVENT_MSC :
			if(is_event_support(code, dev->mscbit, INPUT_MISC_MAX))
				report = true;
			break;
		default :
			break;
	}

	if (report) {
		event.type = type;
		event.code = code;
		event.value = value;
		input_pass_event(dev, &event);
	}

}

void sunxi_input_event(struct sunxi_input_dev *dev,
			unsigned int type, unsigned int code, unsigned int value)
{
	if(is_event_support(type, dev->evbit, INPUT_EVENT_MAX)) {
		input_handle_event(dev, type, code, value);
	}
}

static int sunxi_fetch_next_event(struct sunxi_evdev *evdev, struct sunxi_input_event *event)
{
	int have_event;
	uint32_t level;

	level = freert_spin_lock_irqsave(&input_lock);

	have_event = evdev->packet_head != evdev->tail;
	if (have_event) {
		*event = evdev->buffer[evdev->tail++];
		evdev->tail &= EVENT_BUFFER_SIZE - 1;
	}

	freert_spin_unlock_irqrestore(&input_lock, level);

	return have_event;
}


/*------------------------------DRIVER API-------------------------*/
void input_set_capability(struct sunxi_input_dev *dev, unsigned int type, unsigned int code)
{
	switch (type) {
		case INPUT_EVENT_KEY:
			input_set_bit(code, dev->keybit);
			break;

		case INPUT_EVENT_REL:
			input_set_bit(code, dev->relbit);
			break;

		case INPUT_EVENT_ABS:
			input_set_bit(code, dev->absbit);
			break;

		case INPUT_EVENT_MSC:
			input_set_bit(code, dev->mscbit);
			break;

		default:
			input_err("input_set_capability: unknown type %u (code %u)\n",
					type, code);
			return;
	}

	input_set_bit(type, dev->evbit);
}

struct sunxi_input_dev *sunxi_input_allocate_device()
{
	struct sunxi_input_dev *dev;

	dev = pvPortMalloc(sizeof(struct sunxi_input_dev));
	if (dev)
		memset(dev, 0 , sizeof(struct sunxi_input_dev));

	return dev;

}

int sunxi_input_register_device(struct sunxi_input_dev *dev)
{
	if (NULL == dev->name) {
		input_err("err : input device must have a name\n");
		return -1;
	}

	/* Every input device generates INPUT_EVENT_SYN/INPUT_SYNC_REPORT events. */
	input_set_bit(INPUT_EVENT_SYN, dev->evbit);

	list_add_tail(&dev->node, &input_dev_list);
	return 0;
}

struct sunxi_input_dev *sunxi_input_get_current_device(void)
{
    /**
     * 注意:
     * 1. 由于目前的 sunxi_input_init 只允许注册一个设备, 所以所注册的设备就是当前设备
     * 2. 如果后面的注册机制有变动, 该函数也需要进行修改
     */
    struct sunxi_input_dev *dev = NULL;

    list_for_each_entry(dev, &input_dev_list, node)
    {
        if (NULL != dev) {
            break;
        }
    }

    return dev;
}

#ifdef CONFIG_COMPONENTS_PM

static int sunxi_input_suspend(struct pm_device *dev, suspend_mode_t mode)
{
	input_err("suspend");
	return hal_dev_gpio_standby_config(TP_NODE_NAME, -1, 1);
}

static int sunxi_input_resume(struct pm_device *dev, suspend_mode_t mode)
{
	input_err("resume");
	return hal_dev_gpio_standby_config(TP_NODE_NAME, -1, 0);
}

static struct pm_devops sunxi_input_devops = {
    .suspend = sunxi_input_suspend,
    .resume = sunxi_input_resume,
};

static struct pm_device sunxi_input_pm = {
    .name = "sunxi_input",
    .ops = &sunxi_input_devops,
};
#endif

/*---------------------------USER API-------------------------*/
int sunxi_input_init(void)
{
	int ret = 0;
    tp_board_config_parse();

#ifdef CONFIG_COMPONENTS_PM
    pm_devops_register(&sunxi_input_pm);
#endif
#ifdef CONFIG_DRIVERS_TOUCHSCREEN_TLSC6X
int tlsc6x_init(void);
	ret |= tlsc6x_init();
#endif
#ifdef CONFIG_DRIVERS_TOUCHSCREEN_GT911
int gt911_init(void);
	ret |= gt911_init();
#endif
#ifdef CONFIG_DRIVERS_TOUCHSCREEN_CST226SE
int cst226se_init(void);
	ret |= cst226se_init();
#endif
#ifdef CONFIG_DRIVERS_GPIO_KEY_NORMAL
int sunxi_normal_key_init(void);
	ret |= sunxi_normal_key_init();
#endif
#ifdef CONFIG_DRIVERS_TOUCHSCREEN_AXS15231
    if (0 == strcmp(TP_AXS15231_NAME, tp_board_config_drv_name_get())) {
        ret |= tp_axs15231_init();
    }
#endif
#ifdef CONFIG_DRIVERS_TOUCHSCREEN_AXS5106
    if (0 == strcmp(TP_AXS5106_NAME, tp_board_config_drv_name_get())) {
        ret |= tp_axs5106_init();
    }
#endif

	return ret;
}

int sunxi_input_open(const char *name)
{
	struct sunxi_input_dev *dev = NULL;
	struct sunxi_evdev *evdev;

	dev = find_input_dev_by_name(name);
	if (NULL == dev) {
		input_err("input dev %s is not exist\n", name);
		return -1;
	}

	evdev = pvPortMalloc(sizeof(struct sunxi_evdev));
	if (evdev) {
		memset(evdev, 0 ,sizeof(struct sunxi_evdev));
		evdev->name = dev->name;
		evdev->fd = evdev_cnt++;
		evdev->sem = hal_sem_create(0);
		if(NULL == evdev->sem)
			input_err("init Semaphore fail\n");
		list_add_tail(&evdev->node, &evdev_list);

		return evdev->fd;
	}

	return -1;
}

int sunxi_input_read(int fd, void *buffer, unsigned int size)
{
	unsigned int count = 0;
	struct sunxi_evdev *evdev = NULL;
	struct sunxi_input_event event;

	evdev = find_evdev_by_fd(fd);
	if(NULL == evdev) {
		input_err("input read fd err\n");
		return -1;
	}

	while(count + sizeof(struct sunxi_input_event) <= size
			&& sunxi_fetch_next_event(evdev, &event)) {
		memcpy(buffer + count, &event, sizeof(struct sunxi_input_event));
		count += sizeof(struct sunxi_input_event);
	}

	return count;
}

int sunxi_input_readb(int fd, void *buffer, unsigned int size)
{
	int i = 0;
	unsigned int count = 0;
	int ret;
	struct sunxi_evdev *evdev = NULL;
	struct sunxi_input_event event;

	evdev = find_evdev_by_fd(fd);
	if(NULL == evdev) {
		input_err("input read fd err\n");
		return -1;
	}

	if (evdev->packet_head == evdev->tail) {
		ret = hal_sem_wait(evdev->sem);
		if (ret != HAL_OK) {
			input_err("input take semaphore err\n");
			return -1;
		}
	}

	while(count + sizeof(struct sunxi_input_event) <= size
			&& sunxi_fetch_next_event(evdev, &event)) {
		memcpy(buffer + count, &event, sizeof(struct sunxi_input_event));
		count += sizeof(struct sunxi_input_event);
	}

	return count;
}
