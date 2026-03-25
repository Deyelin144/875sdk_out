#ifndef __REALIZE_UNIT_AAC_H__
#define __REALIZE_UNIT_AAC_H__

#include "gulitesf_config.h"

#ifdef CONFIG_AAC_SUPPORT

/* MPEG版本 */
typedef enum {
    AAC_MPEG2 = 1, // MPEG-2
    AAC_MPEG4 = 0  // MPEG-4
} aac_mpeg_version_t;

/* AAC对象类型 */
typedef enum {
    AAC_OBJECT_MAIN = 1, // Main
    AAC_OBJECT_LOW = 2,  // Low Complexity
    AAC_OBJECT_SSR = 3,  // Scalable Sample Rate
    AAC_OBJECT_LTP = 4   // Long Term Prediction
} aac_object_type_t;

/* 输入样本格式 */
typedef enum {
    AAC_INPUT_NULL = 0,   // 无效输入
    AAC_INPUT_16BIT = 1,  // 16位PCM
    AAC_INPUT_24BIT = 2,  // 24位PCM
    AAC_INPUT_32BIT = 3,  // 32位PCM
    AAC_INPUT_FLOAT = 4   // 浮点格式
} aac_input_format_t;

/* 输出流格式 */
typedef enum {
    AAC_STREAM_RAW = 0,   // RAW流
    AAC_STREAM_ADTS = 1   // ADTS流
} aac_stream_format_t;

/* 块类型控制 */
typedef enum {
    AAC_SHORTCTL_NORMAL = 0, // 正常模式，动态选择短窗口或长窗口
    AAC_SHORTCTL_NOSHORT = 1, // 禁用短窗口
    AAC_SHORTCTL_NOLONG = 2  // 禁用长窗口
} aac_shortctl_t;

typedef enum {
    UNIT_AAC_ENCODE,
    UNIT_AAC_DECODE,
} unit_aac_codec_t;

typedef struct {
    unsigned int codec : 1;                 // 编码类型（0：编码，1：解码）
    unsigned int aac_object_type : 3;       // AAC对象类型（例如 Main, Low Complexity）
    unsigned int sample_rate : 17;          // 采样率（8-96kHz）
    unsigned int quality : 11;              // 编码质量（范围 0-100）
    unsigned int channels : 5;              // 通道数（最大支持 32 个通道）
    unsigned int input_format : 3;          // 输入样本格式（0=valid, 1=16-bit, 2=24-bit, 3=32-bit, 4=float）
    unsigned int stream_format : 1;         // 输出格式（0=RAW, 1=ADTS）
    unsigned int mpeg_version : 1;          // MPEG版本（0=MPEG-4, 1=MPEG-2）
    unsigned int shortctl : 2;              // 块类型控制（0=正常,动态选择, 1=禁用短窗口, 2=禁用长窗口）
    unsigned int bit_rate : 20;             // 比特率  (8-512kbps)
} aac_param_t;

typedef int (*aac_create)(void *ctx, aac_param_t *param, void *userdata);
/*
    aac编码：
        in: 输入数据，PCM格式，长度单个通道为1024个采样点，每个采样点为16bit，即：1024 * 2 * channel字节
*/
typedef int (*aac_codec)(void *ctx, void *in, int in_size, void *out, int *out_len);
typedef void (*aac_destroy)(void *ctx);

/* 
    设置参数使用 JSON 字符串：{"codec":0, "aac_object_type":2, "sample_rate":16000,  \
            "quality":100, "channels":1, "input_format":1, "stream_format":1,    \
            "mpeg_version":0, "shortctl":0, "bit_rate":64000}ma
    启动编码器的默认参数为上述json中的配置，如果与默认参数一致，可以不传入该参数
*/
typedef int (*aac_set_param_using_json)(void *ctx, char *json_str);
typedef int (*aac_get_param)(void *ctx, const char **param);

typedef struct {
    void *ctx;
    aac_create create;
    aac_codec codec;
    aac_destroy destroy;
    aac_set_param_using_json set_param_using_json;
    aac_get_param get_param;
} aac_obj_t;

aac_obj_t *realize_unit_aac_new(void);
void realize_unit_aac_delete(aac_obj_t **aac_obj);

#endif
#endif