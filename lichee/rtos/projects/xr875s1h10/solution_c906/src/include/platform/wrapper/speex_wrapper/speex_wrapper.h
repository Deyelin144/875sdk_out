#ifndef __SPEEX_WRAPPER_H__
#define __SPEEX_WRAPPER_H__

#include "../../gulitesf_config.h"
#include "../../../mod_realize/realize_unit_audio_codec/realize_unit_speex/realize_unit_speex.h"

#if defined(CONFIG_SPEEX_SUPPORT) && !defined(CONFIG_SPEEX_USER_INTERNAL)

/**
 * @brief 初始化编码器
 * @param speex_mode：模式
 * @return 成功则返回句柄，失败返回NULL
 */
typedef void *(*encoder_init_t)(unit_speex_mode_t speex_mode);

/**
 * @brief 设置编码器参数
 * @param state：编码器状态
 * @param request：参数类型
 * @param ptr：要设定的值
 * @return 成功则返回0，失败返回-1
 */
typedef int (*encoder_ctl_t)(void *state, int request, void *ptr);

/**
 * @brief 
 * @param state：
 * @param in：
 * @param bits：
 * @return 成功则返回0，失败返回-1
 */
typedef int (*encode_int_t)(void *state, void *in, void *bits);

/**
 * @brief 销毁编码器
 * @param state：编码器状态
 * @return
 */
typedef void (*encoder_destroy_t)(void *state);


/**
 * @brief 初始化解码器
 * @param speex_mode：模式
 * @return 成功则返回句柄，失败返回NULL
 */
typedef void *(*decoder_init_t)(unit_speex_mode_t speex_mode);

/**
 * @brief 设置解码器参数
 * @param state：编码器状态
 * @param request：参数类型
 * @param ptr：要设定的值
 * @return 成功则返回0，失败返回-1
 */
typedef int (*decoder_ctl_t)(void *state, int request, void *ptr);

/**
 * @brief 
 * @param state：
 * @param in：
 * @param bits：
 * @return 成功则返回0，失败返回-1
 */
typedef int (*decode_int_t)(void *state, void *bits, void *in);

/**
 * @brief 销毁解码器
 * @param state：编码器状态
 * @return
 */
typedef void (*decoder_destroy_t)(void *state);

/**
 * @brief 初始化bits
 * @param bits
 * @return
 */
typedef void (*bits_init_t)(void **bits);

/**
 * @brief 数据处理
 * @param bits：句柄
 * @param bytes：
 * @param max_len：
 * @return 成功则返回0，失败返回-1
 */
typedef int (*bits_write_t)(void *bits, char *bytes, int max_len);

/**
 * @brief 解码把数据读入
 * @param bits：句柄
 * @param bytes：
 * @param max_len：
 * @return void
 */
typedef void (*bits_read_from_t)(void *bits, char *bytes, int max_len);

/**
 * @brief 复位bits
 * @param bits：句柄
 * @return
 */
typedef void (*bits_reset_t)(void *bits);

/**
 * @brief 释放bits
 * @param bits：句柄
 * @return
 */
typedef void (*bits_destroy_t)(void *bits);

typedef struct {
	encoder_init_t encoder_init;
	encoder_ctl_t encoder_ctl;
	encode_int_t encode_int;
	encoder_destroy_t encoder_destroy;
	decoder_init_t decoder_init;
	decoder_ctl_t decoder_ctl;
	decode_int_t decode_int;
	decoder_destroy_t decoder_destroy;
	bits_init_t bits_init;
	bits_write_t bits_write;
	bits_read_from_t bits_read_from;
	bits_reset_t bits_reset;
	bits_destroy_t bits_destroy;
} speex_wrapper_t;

#endif

#endif
