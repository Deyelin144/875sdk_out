#ifndef __TP_AXS15231_H__
#define __TP_AXS15231_H__

#define TP_AXS15231_NAME "axs15231"

/**
 * @brief  axs15231 初始化
 * @return 0:成功, -1:失败 
 */
int tp_axs15231_init(void);

/**
 * @brief  axs15231 挂起一段时间, 在这段时间内 axs15231 无响应
 * @return 0:成功, -1:失败 
 */
int tp_axs15231_hang_up(void);

#endif /* __TP_AXS15231_H__ */