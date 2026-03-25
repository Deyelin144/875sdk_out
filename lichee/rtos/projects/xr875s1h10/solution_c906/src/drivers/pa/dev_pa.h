#ifndef DEV_PA_H__
#define DEV_PA_H__

/**
 * @brief  功放引脚初始化
 * @return 0:成功, -1:失败
 */
int dev_pa_pin_init(void);

/**
 * @brief  功放使能控制
 * @param enable: 1:使能, 0:禁止
 */
void dev_pa_pin_control(int enable);

#endif /* DEV_PA_H__ */