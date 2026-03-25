#include "compiler.h"
#include "kernel/os/os.h"
#include "dev_extio.h"

extern void drv_extio_aw9523b_ops_register(drv_extio_ops_t *ops);

#define EXTIO_OPS_REGISTER_MAP          \
    {                                   \
        drv_extio_aw9523b_ops_register, \
    }

typedef void (*drv_extio_ops_arrays_t)(drv_extio_ops_t *ops);

typedef struct {
    drv_extio_ops_t ops;
} dev_extio_t;

static dev_extio_t s_dev_extio;

int dev_extvcc_enable(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_EXTVCC_EN, DEV_EXTIO_GPIO_LEVEL_HIGH);
    return 0;
}

int dev_extvcc_disable(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_EXTVCC_EN, DEV_EXTIO_GPIO_LEVEL_LOW);
    return 0;
}


int dev_extio_init(void)
{
    static int inited = 0;
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;
    drv_extio_ops_arrays_t extio_ops_map[] = EXTIO_OPS_REGISTER_MAP;

    if (inited) {
        printf("extio initd\n");
        return 0;
    }

    extio_ops_map[0](&s_dev_extio.ops);

    ret = s_dev_extio.ops.init(NULL);
    if (0 != ret) {
        printf("extio device init fail: %d.\n", ret);
        goto exit;
    }
    
    ret = 0;
    inited = 1;

exit:
    return ret;
}

void dev_extio_deinit(void)
{
    s_dev_extio.ops.deinit();
}
