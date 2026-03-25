#include "mcu_at_device.h"
#include "drv_uart_iface.h"
#include "../drv_log.h"
#include "at_command_handler.h"
#include <console.h>
#include "../rtc/dev_rtc.h"
#include <sunxi_hal_rtc.h>
#include "../pm/dev_pm.h"
#include "dev_mcu_upgrade.h"

#define MCU_AT_RETRY_NUM  3                 //mcu指令通讯重试次数   

typedef enum {
    MCU_ERROR		        =   -1,			//mcu通讯错误(跳转到app成功后,mcu通讯错误,可能app无效)
	MCU_INIT				=   0,			//MCU同步中
	MCU_BOOTLOADER			=   1,			//MCU在bootloader
    MCU_APP					=   2,			//MCU在app
    MCU_WAIT_APP			=   3,			//MCU从bootloader跳转到app后,等待app同步
} mcu_sync_t;

static XR_OS_Thread_t s_at_uart_read_tid;
static XR_OS_Thread_t s_at_cmd_parser_tid;
static XR_OS_Thread_t s_at_cmd_irq_tid;
static XR_OS_Semaphore_t s_mcu_at_resp_sem;
static XR_OS_Semaphore_t s_mcu_at_cmd_sem;
static XR_OS_Semaphore_t s_mcu_at_irq_sem;
static XR_OS_Mutex_t s_mcu_at_mutex;
static uint8_t s_mcu_at_uart_is_sleep = 0;
static uint8_t s_mcu_wakeSrc = 0;
static uint8_t s_mcu_at_have_suspend = 0;

static void _at_uart_read_tid()
{
    uint8_t parser_ret = 0;
    uint8_t byte = 0;
    drv_logi("at_uart_read start");
    while (1) {
        if (s_mcu_at_uart_is_sleep) {
            s_mcu_at_have_suspend = 1;
            XR_OS_MSleep(100);
            continue;
        }

        if (1 == drv_mcu_iface_recv_data(&byte, 1, 10)) {
            parser_ret = at_parser_feed(byte);

            if (1 == parser_ret) {
                drv_logd("cmd=%s", at_parser_get_cmd());
                XR_OS_SemaphoreRelease(&s_mcu_at_cmd_sem);
            } else if (2 == parser_ret) {
                drv_logd("resp=%s", at_parser_get_cmd());
                XR_OS_SemaphoreRelease(&s_mcu_at_resp_sem);
            }
        }

        XR_OS_MSleep(1);
        continue;
    }

    XR_OS_SemaphoreDelete(&s_mcu_at_resp_sem);
    XR_OS_SemaphoreDelete(&s_mcu_at_cmd_sem);
    XR_OS_ThreadDelete(&s_at_uart_read_tid);
}

static void _at_cmd_parser_tid()
{
    drv_logi("at_cmd_parser start");

    while (1) {
        if (XR_OS_OK == XR_OS_SemaphoreWait(&s_mcu_at_cmd_sem, 10)) {
            at_process_command(at_parser_get_cmd());
        } else if (XR_OS_OK == XR_OS_SemaphoreWait(&s_mcu_at_resp_sem, 10)) {
            at_process_response(at_parser_get_cmd());
        }

        at_check_response_timeout();
    }

    XR_OS_ThreadDelete(&s_at_cmd_parser_tid);
}

uint8_t mcu_at_uart_is_sleep()
{
    return s_mcu_at_uart_is_sleep;
}

void mcu_at_uart_suspend(void)
{
    s_mcu_at_uart_is_sleep = 1;
    while (!s_mcu_at_have_suspend) {
        XR_OS_MSleep(1);
    }
    drv_mcu_iface_uart_suspend();
}

void mcu_wakeup_pin_out()
{
    dev_pm_clear_exit_wakeup_src(DEV_PM_WAKEUP_SRC_USB);
    hal_gpio_set_pull(MCU_WAKEUP_PIN, GPIO_PULL_DOWN);
    hal_gpio_set_direction(MCU_WAKEUP_PIN, GPIO_DIRECTION_OUTPUT);
    hal_gpio_pinmux_set_function(MCU_WAKEUP_PIN,GPIO_MUXSEL_OUT);
    hal_gpio_set_data(MCU_WAKEUP_PIN, GPIO_DATA_HIGH);

    XR_OS_MSleep(10);

    hal_gpio_set_pull(MCU_WAKEUP_PIN, GPIO_PULL_DOWN_DISABLED);
    hal_gpio_pinmux_set_function(MCU_WAKEUP_PIN,GPIO_PULL_DOWN_DISABLED);
}

void mcu_at_uart_resume(void)
{
#ifdef CONFIG_PROJECT_SOLUTION_C906
    int ret = -1;
    uint8_t cmd = 0;
    
    //叫醒mcu
    mcu_wakeup_pin_out();

    drv_mcu_iface_uart_resume();
    s_mcu_at_have_suspend = 0;
    s_mcu_at_uart_is_sleep = 0;

    s_mcu_wakeSrc = dev_mcu_wakeSrc_get();
    
    drv_loge("mcu wake src: %d\n", s_mcu_wakeSrc);
    if (dev_pm_get_wakeup_src() == WAKEUP_ALARM) {
        cmd = 1;
    }

    for(int i = 0; i < 3; i++) {
        ret = dev_mcu_sleep_set(0,cmd);
        if (ret == 0) {
            break;
        }
    }

    mcu_irq_cb();
#endif
}

uint8_t mcu_wakeSrc_get(void)
{
    return s_mcu_wakeSrc;
}

void mcu_irq_cb(void *arg)
{
    if (1 == XR_OS_SemaphoreIsValid(&s_mcu_at_irq_sem)) {
        XR_OS_SemaphoreRelease(&s_mcu_at_irq_sem);
    }
}
int drv_mcu_iface_irq_init_default()
{
    int ret = drv_mcu_iface_irq_init(&mcu_irq_cb);
    if (ret != 0) {
        drv_loge("irq init fail: %d.", ret);
    }
    return ret;
}

static void _at_cmd_irq_tid()
{
    drv_logi("mcu irq tid start");

    while (1) {
        if (XR_OS_OK != XR_OS_SemaphoreWait(&s_mcu_at_irq_sem, XR_OS_WAIT_FOREVER)) {
            continue;
        } 

        if (s_mcu_at_uart_is_sleep) {
            drv_loge("mcu irq! ignore\n");
            continue;
        }

        drv_logd("mcu irq event ignored in sdk mode");
    }

    XR_OS_ThreadDelete(&s_at_cmd_irq_tid);
}

//rtc唤醒，则拉高引脚，唤醒mcu
void deep_sleep_boot_reason_handler(void)
{
    mcu_wakeup_pin_out();
#ifdef CONFIG_PROJECT_SOLUTION_C906
    uint32_t  boot_info = 0;
	boot_info = app_get_current_boot_reason();
    printf("boot_info = %d\n",boot_info);
    //SUNXI_BOOT_REASON_HIBERNATION_WAKEUP_IO
    //SUNXI_BOOT_REASON_HIBERNATION_RTC_ALARM   
#endif 
}

int mcu_at_device_init(void)
{
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;
    if (XR_OS_ThreadIsValid(&s_at_uart_read_tid)) {
        drv_loge("mcu exist!\n");
        return - 1;
    }

    ret = drv_mcu_iface_uart_init();
    if (ret != 0) {
        drv_loge("uart init fail: %d.", ret);
        goto exit;
    }
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    ret = mcu_recovery_at_thread_start();
    if (ret) {
        goto exit;
    }
    return 0;
#else
    deep_sleep_boot_reason_handler();

    //同步系统,获取版本号
    dev_mcu_sync();

    if ((XR_OS_OK != (status = XR_OS_SemaphoreCreate(&s_mcu_at_resp_sem, 0, 2))) 
        || (XR_OS_OK != (status = XR_OS_SemaphoreCreate(&s_mcu_at_cmd_sem, 0, 2)))
        || (XR_OS_OK != (status = XR_OS_SemaphoreCreate(&s_mcu_at_irq_sem, 0, 1)))
        || (XR_OS_OK != (status = XR_OS_MutexCreate(&s_mcu_at_mutex)))) {
        drv_loge("semaphore or mutex create fail: %d.", status);
        goto exit;
    }

    if (XR_OS_OK != XR_OS_ThreadCreate(&s_at_cmd_parser_tid,
                                "at_cmd_parser_tid",
                                _at_cmd_parser_tid,
                                NULL,
                                XR_OS_PRIORITY_NORMAL,
                                1024 * 2)) {
        drv_loge("create task err.....");
        goto exit;
    }

    if (XR_OS_OK != XR_OS_ThreadCreate(&s_at_uart_read_tid,
                                "at_uart_read_tid",
                                _at_uart_read_tid,
                                NULL,
                                XR_OS_PRIORITY_NORMAL,
                                1024 * 2)) {
        drv_loge("create task err.....");
        goto exit;
    }

    if (XR_OS_OK != XR_OS_ThreadCreate(&s_at_cmd_irq_tid,
                                "_at_cmd_irq_tid",
                                _at_cmd_irq_tid,
                                NULL,
                                XR_OS_PRIORITY_NORMAL,
                                1024 * 2)) {
        drv_loge("create task err.....");
        goto exit;
    }

    //此处阻塞等待mcu回复
    dev_mcu_sleep_set(0,0);
#endif
    
    ret = 0;
exit:
    return ret;   
}

int mcu_at_mutex_lock(char *fun, int line)
{
    // printf("mcu_at_mutex_lock %s:%d\n", fun, line);
    return XR_OS_MutexLock(&s_mcu_at_mutex, XR_OS_WAIT_FOREVER);
}

int mcu_at_mutex_unlock(char *fun, int line)
{
    // printf("mcu_at_mutex_unlock %s:%d\n", fun, line);
    return XR_OS_MutexUnlock(&s_mcu_at_mutex);
}

int dev_mcu_ext_vcc_set(uint8_t val)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return 0;
#else
    char cmd[8] = {0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_EXT_VCC_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_EXT_VCC_SET, cmd, NULL,0);
    return 0;
#endif
}

int dev_mcu_sys_vcc_set(uint8_t val)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return 0;
#else
    char cmd[8] = {0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_SYS_VCC_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_SYS_VCC_SET, cmd, NULL,0);
    return 0;
#endif
}

int dev_mcu_lcd_reset_set(uint8_t val)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    char cmd[8] = {0};
    cmd[0] = AT_REQ_CMD_LCD_RESET_SET,
    cmd[1] = val;
    return mcu_recovery_at_control(AT_REQ_CMD_LCD_RESET_SET, cmd,2);
#else
    char cmd[8] = {0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_LCD_RESET_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_LCD_RESET_SET, cmd, NULL,0);
    return 0;
#endif
}

int dev_mcu_tp_reset_set(uint8_t val,uint8_t block)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    char cmd[8] = {0};
    cmd[0] = AT_REQ_CMD_TP_RESET_SET,
    cmd[1] = val;
    return mcu_recovery_at_control(AT_REQ_CMD_TP_RESET_SET, cmd,2);
#else
    char cmd[8]={0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_TP_RESET_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_TP_RESET_SET, cmd, NULL,block);
    return 0;
#endif
}

int dev_mcu_cam_pwdn_set(uint8_t val)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return 0;
#else
    char cmd[8]={0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_CAMERA_PWDN_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_CAMERA_PWDN_SET, cmd, NULL,0);
    return 0;
#endif
}

int dev_mcu_pa_set(uint8_t val)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    char cmd[8] = {0};
    cmd[0] = AT_REQ_CMD_PA_SET,
    cmd[1] = val;
    return mcu_recovery_at_control(AT_REQ_CMD_PA_SET, cmd,2);
#else
    printf("dev_mcu_pa_set start val = %d.\n", val);
    char cmd[8]={0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_PA_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_PA_SET, cmd, NULL, 1);
    printf("dev_mcu_pa_set end val = %d.\n", val);
    return 0;
#endif
}

//显示低功耗
int dev_mcu_lcd_bl_lp_set(uint8_t val)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    char cmd[8] = {0};
    cmd[0] = AT_REQ_CMD_LCD_BL_LP_SET,
    cmd[1] = val;
    return mcu_recovery_at_control(AT_REQ_CMD_LCD_BL_LP_SET, cmd,2);
#else
    char cmd[8]={0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_LCD_BL_LP_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_LCD_BL_LP_SET, cmd, NULL,0);
    return 0;
#endif
}

//机器人舵机供电设置
int dev_mcu_robot_vcc_set(uint8_t val)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return 0;
#else
    char cmd[8]={0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_ROBOT_VCC_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_ROBOT_VCC_SET, cmd, NULL,0);
    return 0;
#endif
}

/***************************************************************
  *  @brief     MCU 电量信息获取,一个接口实现,减少通讯次数
  *  @param     @isCharge: 是否充电
  *             @isFull: 是否充满
  *  @note      
  *  @Sample usage: 
 **************************************************************/
int dev_mcu_charge_info_get(char *isCharge, char *isFull)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    char cmd[8] = {0};
    cmd[0] = AT_REQ_CMD_BATTERY_CHARGE_GET,
    mcu_recovery_at_control_for_reply(AT_REQ_CMD_BATTERY_CHARGE_GET, cmd,1);
    return cmd[2];
#else
    int val[2];
    char cmd[8]={0};
    sprintf(cmd, "%d\r\n",AT_REQ_CMD_BATTERY_CHARGE_GET);
    for (int i = 0; i < MCU_AT_RETRY_NUM; i++) {
        memset(val, -1, sizeof(val));
        AT_SEND_REQ(AT_REQ_CMD_BATTERY_CHARGE_GET, cmd, &val,1);
        if ( (val[0] != -1 ) && (val[1] != -1) ) {
            *isCharge = val[0];
            *isFull = val[1];
            return 0;
        } 
    }

    drv_loge("get mcu charge info failed\n");
    return -1;
#endif
}

/*充电检测引脚*/
uint8_t dev_mcu_charge_det_get(void)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    char cmd[8] = {0};
    cmd[0] = AT_REQ_CMD_BATTERY_CHARGE_GET,
    mcu_recovery_at_control_for_reply(AT_REQ_CMD_BATTERY_CHARGE_GET, cmd,1);
    return cmd[2];
#else
    char ischarge = 0;
    char isFull = 0;
    dev_mcu_charge_info_get(&ischarge, &isFull);
    
    return ischarge;
#endif
}

/*充满检测引脚*/
uint8_t dev_mcu_charge_full_det_get(void)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return 0;
#else
    int val = 0;
    char cmd[8]={0};
    sprintf(cmd, "%d\r\n",AT_REQ_CMD_BATTERY_CHARGE_FULL_GET);
    AT_SEND_REQ(AT_REQ_CMD_BATTERY_CHARGE_FULL_GET, cmd, &val,1);
    return val;
#endif
}

//主中断设置
int dev_mcu_irq_set(uint8_t val)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return 0;
#else
    char cmd[8]={0};
    sprintf(cmd, "%d,%d\r\n",AT_REQ_CMD_IRQ_SET,val);
    AT_SEND_REQ(AT_REQ_CMD_IRQ_SET, cmd, NULL,1);
    return 0;
#endif
}

//休眠设置
int dev_mcu_sleep_set(int mode,int arg)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return 0;
#endif
    int val = -1;
    char cmd[8]={0};
    sprintf(cmd, "%d,%d\r\n",mode,arg);
    AT_SEND_REQ(AT_REQ_CMD_SLEEP, cmd, &val,1);
    return val;
}

//获取唤醒源
int dev_mcu_wakeSrc_get(void)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return 0;
#else
    int val = -1;
    char cmd[8]={0};
    sprintf(cmd, "%d\r\n",AT_REQ_CMD_WAKE_SRC_GET);
    for (int i = 0; i < MCU_AT_RETRY_NUM; i++) {
        AT_SEND_REQ(AT_REQ_CMD_WAKE_SRC_GET, cmd, &val,1);
        if (val != -1) {
            break;
        }
    }
    if (val < 0) {
        printf("get mcu wakeSrc failed\n");
        val = 0;
    }
    return val;
#endif
}

/***************************************************************
  *  @brief     MCU 版本获取
  *  @param  version: 版本二级指针   内部申请,外部释放
  *  @note      
  *  @Sample usage: 
 **************************************************************/
int dev_mcu_version_get(char **version)
{
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
    return -1;
#else
    int val = 0;
    *version = calloc(1,32);
    if (NULL == *version) {
        drv_loge("calloc err\n");
        return -1;
    }

    AT_SEND_REQ(AT_REQ_CMD_ATI, NULL, *version,1);
    if (0 == strcmp(*version,"")){
        return -1;
    }
    return 0;
#endif
}

/***************************************************************
  *  @brief     C906下 ,mcu 状态同步
  *  @param  
  *  @note      mcu为bootload时发送AT指令会卡死,所以用bootload指令判断同步状态
  *  @Sample usage: 
 **************************************************************/
int dev_mcu_sync()
{
    uint32_t bootload_version;
    mcu_sync_t mcu_state = MCU_INIT;
    int ret = -1;
    char *data = calloc(1,128);
    if (data == NULL) {
        drv_loge("calloc failed\n");
        return -1;
    }

    for (int i = 0; i < 3; i++) {
        // if (mcu_state == MCU_INIT || mcu_state == MCU_WAIT_APP) {
            memset(data,0,128);
            drv_mcu_iface_recv_data(data, 128, 150);
            drv_logw("filter recv data:%s\n",data);
            drv_mcu_iface_send_data("AT+ATI\r\n",strlen("AT+ATI\r\n"));
            drv_mcu_iface_recv_data(data, 128, 150);
            drv_logw("recv data:%s\n",data);
            
            if (strstr(data, "mcu_version") != NULL) {
                mcu_state = MCU_APP;
                drv_loge("c906 sync suc,%s\n",data);  //mcu在 app,正常启动
                goto exit;
            } else {
                drv_loge("c906 mcu version get failed");
            }
        // }

        if (mcu_state == MCU_INIT || mcu_state == MCU_BOOTLOADER) {
            memset(data,0,128);
            drv_mcu_iface_recv_data(data, 128, 150);
            drv_logw("filter data:%s\n",data);
            if (bootload_get_version(&bootload_version)) {        //mcu在 bootloader,尝试切换到app
                if (mcu_state != MCU_BOOTLOADER){
                    mcu_state = MCU_BOOTLOADER;
                    i = -1;         //切换状态,重置重试次数
                }

                if (mcu_jump_to_app(1)) {     //切换成功,开始检查app是否有效
                    drv_loge("c906 jump to app success,wait sync");
                    mcu_state = MCU_WAIT_APP;
                    i = -1;         //切换状态,重置重试次数
                    XR_OS_MSleep(1000);
                }
            }
        }
    }

    drv_loge("mcu sync error in state %d\n",mcu_state);
    drv_loge("mcu sync failed,please call developer!!!!!!\n");
    drv_loge("mcu sync failed,please call developer!!!!!!\n");
    drv_loge("mcu sync failed,please call developer!!!!!!\n");

    // if (MCU_WAIT_APP == mcu_state) {        //等待app同步阶段异常,可能app损坏,切到recovery
    //     drv_loge("maybe mcu app error,reboot to recovery\n");
    //     native_app_ota_reboot_to_recovery(0);
    // }

    mcu_state = MCU_ERROR;
    while (1) {
        XR_OS_MSleep(5000);
        drv_loge("mcu sync failed,please call developer!!!!!!\n");
    }

exit:
    drv_safe_free(data);
    return ret;
}

/***************************************************************
  *  @brief     MCU ota升级
  *  @param  
  *  @note      
  *  @Sample usage: 
 **************************************************************/
int dev_mcu_upgrade()
{
    int success = -1;
    AT_SEND_REQ(AT_REQ_CMD_IAP_UPGRADE, NULL, &success,1);
    printf("send upgrade success:%d\n",success);
    return success;
}

#if 0
int dev_my_test(int argc, const char **argv)
{
    int i = 0;
    while (1) {
        dev_mcu_charge_det_get();
        // XR_OS_MSleep(10);
        dev_mcu_charge_full_det_get();
        dev_mcu_version_get();

        // if (i++ >4) {
            break;
        // }
    }
    return 0;
}


FINSH_FUNCTION_EXPORT_CMD(dev_my_test, my_test, mcu options);
#endif
