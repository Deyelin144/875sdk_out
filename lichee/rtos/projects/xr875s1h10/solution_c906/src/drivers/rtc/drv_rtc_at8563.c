#include "drv_rtc.h"

#define VERB 0
#define DBUG 1
#define INFO 1
#define WARN 1
#define EROR 1

#if VERB
#define drv_rtc_at8563_verb(fmt, ...)  drv_logv(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_at8563_verb(fmt, ...)
#endif

#if DBUG
#define drv_rtc_at8563_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_at8563_debug(fmt, ...)
#endif

#if INFO
#define drv_rtc_at8563_info(fmt, ...)  drv_logi(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_at8563_info(fmt, ...)
#endif

#if WARN
#define drv_rtc_at8563_warn(fmt, ...)  drv_logw(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_at8563_warn(fmt, ...)
#endif

#if EROR
#define drv_rtc_at8563_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_at8563_error(fmt, ...)
#endif

#define IS_LEAP_YEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400))) // 判断是否为闰年, 1:闰年

typedef enum {
    AT8563_CTRL_STATUS_REG1  = 0x00, // 控制/状态寄存器 1
    AT8563_CTRL_STATUS_REG2  = 0x01, // 控制/状态寄存器 2
    AT8563_SECOND_REG        = 0x02, // 秒寄存器
    AT8563_MINUTE_REG        = 0x03, // 分钟寄存器
    AT8563_HOUR_REG          = 0x04, // 小时寄存器
    AT8563_DATA_REG          = 0x05, // 日期寄存器
    AT8563_WEEK_REG          = 0x06, // 星期寄存器
    AT8563_MONTH_REG         = 0x07, // 月份/世纪寄存器
    AT8563_YEAR_REG          = 0x08, // 年寄存器
    AT8563_MINUTE_ALERT_REG  = 0x09, // 分钟报警寄存器
    AT8563_HOUR_ALERT_REG    = 0x0a, // 小时报警寄存器
    AT8563_DATA_ALERT_REG    = 0x0b, // 日期报警寄存器
    AT8563_WEEK_ALERT_REG    = 0x0c, // 星期报警寄存器
    AT8563_CLK_FREQ_REG      = 0x0d, // clkout 频率寄存器
    AT8563_TIMER_CTRL_REG    = 0x0e, // 定时器控制寄存器
    AT8563_TIMER_COUNT_REG   = 0x0f, // 定时器倒计数寄存器
} at8563_reg_t;

static drv_rtc_param_t sg_at8563_i2c_param = {
    .twi_id   = 0,
    .twi_freq = 100 * 1000,
    .dev_addr = 0x51,
};

/**
 * @brief  将符合 AT8563 要求的二进制数转换成十进制数
 * @param  val 二进制
 * @return 十进制
 */
static uint8_t bcd_to_dec(uint8_t val)
{
    uint8_t temp = 0;
    temp = (val >> 4) * 10;
    return (temp + (val & 0x0f));
}

/**
 * @brief  将十进制数转换成符合 AT8563 要求的二进制数
 * @param  val 十进制
 * @return 二进制
 */
static uint8_t dec_to_bcd(uint8_t val)
{
    uint8_t temp = 0;
    while (val >= 10) {
        temp++;
        val -= 10;
    }
    return (uint8_t)((temp << 4) | val);
}

/**
 * @brief  获取 AT8563 的实际时间
 * @param  fmt 所要获取的时间 
 * @return DRV_OK:成功,
           DRV_ERROR:失败,时间不准确
           DRV_INVALID:传入参数无效
 */
static drv_status_t drv_rtc_at8563_get_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t status = DRV_ERROR;
    uint8_t time[7] = {0};

    if (NULL == fmt) {
        drv_rtc_at8563_error("invalid param.");
        status = DRV_INVALID;
        goto exit;
    }

    for (uint8_t i = 0; i < 7; i++) {
        drv_rtc_iface_i2c_recv_with_addr(&sg_at8563_i2c_param, AT8563_SECOND_REG + i, &time[i], sizeof(uint8_t));
    }

    /* 1:电源过低无法保证时钟数据准确 */
    if (1 == (0x80 & time[0])) {
        printf("rtc maybe not right.\n");
        fmt->year = 2025;
        fmt->month = 1;
        fmt->day = 1;
        fmt->week = 3;
        fmt->hour = 0;
        fmt->minute = 0;
        fmt->second = 0;
        goto exit;
    }

    if (0 == time[5] || 0 == time[3]) {//月份和天数如果为0，就是异常的值
        printf("rtc don't have time.");
        fmt->year = 2025;
        fmt->month = 1;
        fmt->day = 1;
        fmt->week = 3;
        fmt->hour = 0;
        fmt->minute = 0;
        fmt->second = 0;
        goto exit;
    }

    fmt->second = bcd_to_dec(0x7f & time[0]);
    fmt->minute = bcd_to_dec(0x7f & time[1]);
    fmt->hour = bcd_to_dec(0x3f & time[2]);
    fmt->day = bcd_to_dec(0x3f & time[3]);
    fmt->week = bcd_to_dec(0x07 & time[4]);
    fmt->month = bcd_to_dec(0x1f & time[5]);
    fmt->year = bcd_to_dec(0xff & time[6]);

    if (0x80 & time[5]) {
        // TODO 年份是 19 开头的, 即 19<year>, 假如 year = 95 => 1995
        fmt->year = 1900 + fmt->year;
    } else {
        // TODO 年份是 20 开头的, 即 20<year>, 假如 year = 23 => 2023
        fmt->year = 2000 + fmt->year;
    }

    printf("get time fmt from 8563: %d-%d-%d %d %d:%d:%d\n", fmt->year, fmt->month, fmt->day, fmt->week, fmt->hour, fmt->minute, fmt->second);
    
    status = DRV_OK;
exit:
    return status;
}

/**
 * @brief  设置 AT8563 的时间
 * @param  fmt 所要设置的时间 
 * @return DRV_OK:成功, 否则失败
 */
static drv_status_t drv_rtc_at8563_set_time(drv_rtc_time_fmt_t *fmt)
{
    struct tm t = {0};
    time_t clock = {0};
    struct timeval tv = {0};

    drv_status_t status = DRV_OK;
    uint8_t data = 0x00;
    uint8_t default_time[7] = {0, 0, 15, 1, 4, 6, 23}; // 格式为:秒-分-时-日-星期-月-年 => 23.6.1 四 15:00:00
    uint8_t days_of_month[2][12] = {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, /* 非闰年 */
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  /* 闰年 */
    };

    if (NULL != fmt) {
        uint8_t is_leap_year = IS_LEAP_YEAR(fmt->year);
        /**
         * 示例:
         * 1. 设置的时间为 2023.06.01 4 24:00:00
         *    真正的时间为 2023.06.02 5 00:00:00
         * 
         * 2. 设置的时间为 2023.02.29 3 20:00:00
         *    真正的时间为 2023.03.01 3 20:00:00
         * 
         * 3. 设置的时间为 2023.12.32 0 09:00:00
         *    真正的时间为 2024.01.01 0 09:00:00  
         */
        if (fmt->hour > 23) {
            fmt->hour -= 24;
            fmt->day += 1;
            fmt->week += 1;
            if (fmt->week > 6)
                fmt->week = 0;
        }

        if (fmt->day > days_of_month[is_leap_year][fmt->month - 1]) {
            fmt->day = 1;
            fmt->month += 1;
            if (fmt->month > 12) {
                fmt->month = 1;
                fmt->year += 1;
            }
        }

        /**
         * 对于 AT8563_MONTH_REG, 第 7 bit 决定当前是哪个世纪
         * bit[7] = 1 表示为 20 世纪, 即 19xx 年
         * bit[7] = 0 表示为 21 世纪, 即 20xx 年
         */
        default_time[0] = fmt->second;
        default_time[1] = fmt->minute;
        default_time[2] = fmt->hour;
        default_time[3] = fmt->day;
        default_time[4] = fmt->week;
        if (fmt->year >= 2000) {
            default_time[5] = fmt->month;
        } else {
            default_time[5] = fmt->month | 0x80;
        }
        default_time[6] = fmt->year % 100;
    }

    /*设置系统时间*/
    memset(&t, 0, sizeof(t));
	t.tm_year = fmt->year - CMD_YEAR0;
	t.tm_mon = fmt->month - 1;
	t.tm_mday = fmt->day;
	t.tm_hour = fmt->hour;
	t.tm_min  = fmt->minute;
	t.tm_sec  = fmt->second;

    clock = mktime(&t);
    localtime_r(&clock, &t);
    tv.tv_sec = clock;
	tv.tv_usec = 0;
    status = settimeofday(&tv, NULL);

    for (uint8_t i = 0; i < 7; i++) {
        data = dec_to_bcd(default_time[i]);
        drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_SECOND_REG + i, &data, sizeof(uint8_t));
        XR_OS_MSleep(10);
        drv_rtc_iface_i2c_recv_with_addr(&sg_at8563_i2c_param, AT8563_SECOND_REG + i, &data, sizeof(uint8_t));
        if (data != dec_to_bcd(default_time[i])) {
            data = dec_to_bcd(default_time[i]);
            drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_SECOND_REG + i, &data, sizeof(uint8_t));
        }
    }

    printf("set time fmt to 8563: %d-%d-%d %d %d:%d:%d", default_time[6], default_time[5], default_time[3], default_time[4], default_time[2], default_time[1], default_time[0]);

    return status;
}

/**
 * @brief  获取所设置的 AT8563 报警时间
 * @param  fmt 所要获取的时间
 * @return DRV_OK:成功, 否则失败
 */
drv_status_t drv_rtc_at8563_get_alert_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t status = DRV_OK;
    uint8_t time[4] = {0};

    if (NULL == fmt) {
        drv_rtc_at8563_error("invalid param.");
        status = DRV_INVALID;
        goto exit;
    }

    for (uint8_t i = 0; i < 4; i++) {
        drv_rtc_iface_i2c_recv_with_addr(&sg_at8563_i2c_param, AT8563_MINUTE_ALERT_REG + i, &time[i], sizeof(uint8_t));
    }

    fmt->minute = bcd_to_dec(0x7f & time[0]);
    fmt->hour = bcd_to_dec(0x3f & time[1]);
    fmt->day = bcd_to_dec(0x3f & time[2]);
    fmt->week = bcd_to_dec(0x07 & time[3]);

    printf("alert time get:%d-%d-%d-%d\n",fmt->day, fmt->hour,fmt->minute, fmt->week);
exit:
    return status;
}

/**
 * @brief  设置 AT8563 的报警时间
 * @param  fmt 所要设置的时间
 * @return DRV_OK:成功, 否则失败
 */
static drv_status_t drv_rtc_at8563_set_alert_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t status = DRV_OK;
    uint8_t data = 0x00;
    uint8_t alert_time[4] = {0}; // 格式为:分-时-日-星期

    if ((NULL == fmt)
        || (fmt->minute > 59)
        || (fmt->hour > 23)
        || (fmt->day > 31)
        || (fmt->week > 6)) {
        drv_rtc_at8563_error("invalid param.");
        status = DRV_INVALID;
        goto exit;
    }

    alert_time[0] = fmt->minute;
    alert_time[1] = fmt->hour;
    alert_time[2] = fmt->day;
    alert_time[3] = 0x80;//禁用星期报警

    printf("alert time set:%d-%d-%d-%d\n",alert_time[2], alert_time[1], alert_time[0], alert_time[3]);

    for (uint8_t i = 0; i < 4; i++) {
        data = dec_to_bcd(alert_time[i]);
        drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_MINUTE_ALERT_REG + i, &data, sizeof(uint8_t));
        XR_OS_MSleep(10);
    }
    
    // 开启报警中断功能
    data = 0x12;
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_CTRL_STATUS_REG2, &data, sizeof(uint8_t));

exit:
     return status;
}

/**
 * @brief  停止 AT8563 的报警时间
 * @param  fmt 所要设置的时间
 * @return DRV_OK:成功, 否则失败
 */
static drv_status_t drv_rtc_at8563_clear_alert_time(void)
{
    drv_status_t status = DRV_OK;
    // 关闭分钟报警
    uint8_t data = 0x80;
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_MINUTE_ALERT_REG, &data, sizeof(uint8_t));
    // 关闭小时报警
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_HOUR_ALERT_REG, &data, sizeof(uint8_t));
    // 关闭日期报警
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_DATA_ALERT_REG, &data, sizeof(uint8_t));
    // 关闭星期报警
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_WEEK_ALERT_REG, &data, sizeof(uint8_t));
    // 关闭报警中断功能
    data = 0x00;
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_CTRL_STATUS_REG2, &data, sizeof(uint8_t));

    return status;
}

/**
 * @brief 清除中断标志
 * @return DRV_OK:成功, 否则失败 
 */
void drv_rtc_at8563_clear_af(void)
{
	uint8_t data = 0x10;
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_CTRL_STATUS_REG2, &data, sizeof(uint8_t));
    data = 0x80;
    for (uint8_t i = 0; i < 4; i++) {
        drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_MINUTE_ALERT_REG + i, &data, sizeof(uint8_t));
        XR_OS_MSleep(10);
    }
}

/**
 * @brief 获取中断标志
 * @return DRV_OK:成功, 否则失败 
 */
static uint8_t drv_rtc_at8563_get_af()
{
	uint8_t data = 0;
    drv_rtc_iface_i2c_recv_with_addr(&sg_at8563_i2c_param, AT8563_CTRL_STATUS_REG2, &data, sizeof(uint8_t));
//    drv_rtc_at8563_debug("rtc af:%x\n",data);
    printf("rtc af:%x\n",data);

    return data & 0x08;
}


#if 0
/**
 * @brief  AT8563 报警中断回调
 * @param  fmt 所要设置的时间
 * @return DRV_OK:成功, 否则失败
 * @note 中断函数里操作寄存器会死机
 */
static void drv_rtc_at8563_alert_int_cb(void* arg)
{
    drv_rtc_at8563_debug("==================alert init================\n");
//    if (0 == drv_rtc_at8563_get_af()) {
//        drv_rtc_at8563_clear_af();
//    }
    
//    if (1 == drv_rtc_iface_gpio_read()) {
        printf("gpio read :%d",drv_rtc_iface_gpio_read());
//    }
}
#endif

/**
 * @brief  初始化 AT8563
 * @param  fmt 所要设置的时间, 可为 NULL
 * @return DRV_OK:成功, 否则失败
 */
static drv_status_t drv_rtc_at8563_init(drv_rtc_time_fmt_t *fmt, rtc_irq_callback cb)
{
    drv_status_t status = DRV_OK;
    uint8_t data = 0x00;
    drv_rtc_time_fmt_t cur_time_fmt;

    drv_rtc_iface_i2c_init(&sg_at8563_i2c_param);

    // 初始化之前先获取当前的时间, 避免时间丢失
    status = drv_rtc_at8563_get_time(&cur_time_fmt);
    
    // 启动时钟，开启掉电检测
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_CTRL_STATUS_REG1, &data, sizeof(uint8_t));

    // 关闭分钟报警
    data = 0x80;
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_MINUTE_ALERT_REG, &data, sizeof(uint8_t));
    // 关闭小时报警
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_HOUR_ALERT_REG, &data, sizeof(uint8_t));
    // 关闭日期报警
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_DATA_ALERT_REG, &data, sizeof(uint8_t));
    // 关闭星期报警
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_WEEK_ALERT_REG, &data, sizeof(uint8_t));
    // 关闭报警中断功能
    data = 0x00;
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_CTRL_STATUS_REG2, &data, sizeof(uint8_t));
    // 关闭 clkout 输出
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_CLK_FREQ_REG, &data, sizeof(uint8_t));
    // 关闭定时器
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_TIMER_CTRL_REG, &data, sizeof(uint8_t));
    // 倒时计数
    drv_rtc_iface_i2c_send_with_addr(&sg_at8563_i2c_param, AT8563_TIMER_COUNT_REG, &data, sizeof(uint8_t));

    drv_rtc_at8563_clear_af();// 清除中断标志

    if (NULL != fmt) {
        printf("rtc time invalid, set user time.\n");
        drv_rtc_at8563_set_time(fmt);
    } else {
        printf("rtc time valid, set current time.\n");
        drv_rtc_at8563_set_time(&cur_time_fmt);
    }

    drv_rtc_at8563_debug("init done.");
    
    return status;
}

/**
 * @brief  注销 AT8563
 */
static void drv_rtc_at8563_deinit(void)
{

}

static drv_status_t drv_rtc_at8563_suspend(void)
{
    return DRV_OK;
}

static drv_status_t drv_rtc_at8563_resume(void)
{
    return DRV_OK;
}

void drv_rtc_at8563_get_ops(drv_rtc_ops_t* ops)
{
    ops->init           = drv_rtc_at8563_init;
    ops->deinit         = drv_rtc_at8563_deinit;
    ops->suspend        = drv_rtc_at8563_suspend;
    ops->resume         = drv_rtc_at8563_resume;
    ops->get_time       = drv_rtc_at8563_get_time;
    ops->set_time       = drv_rtc_at8563_set_time;
    ops->get_alert_time = drv_rtc_at8563_get_alert_time;
    ops->set_alert_time = drv_rtc_at8563_set_alert_time;
    ops->get_alert_flag = drv_rtc_at8563_get_af;
    ops->clear_alert_flag = drv_rtc_at8563_clear_af;
    ops->clear_alert_time = drv_rtc_at8563_clear_alert_time;
}


/*
 * @brief chip ic 检测
 * @return DRV_OK 检测到是at8563 IC, DRV_ERROR 不是at8563 IC或IC空片
 * @note at8563没有寄存器识别ID，通过能否接收I2C数据方式来确定是否该IC，能接收代表就是该ID
 */
drv_status_t drv_rtc_at8563_chip_check()
{
    uint8_t data = 0;
    
    drv_rtc_iface_i2c_init(&sg_at8563_i2c_param);
    
    drv_status_t ret = drv_rtc_iface_i2c_recv_with_addr(&sg_at8563_i2c_param, AT8563_CTRL_STATUS_REG1, &data, sizeof(uint8_t));
    if (ret != DRV_OK) {
        printf("no at8563!\n");
    }
    drv_rtc_iface_i2c_deinit(&sg_at8563_i2c_param);
    
    return ret;
}

