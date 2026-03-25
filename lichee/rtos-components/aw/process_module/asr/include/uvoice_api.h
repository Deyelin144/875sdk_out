#if !defined(UVOICE_API_DEF_H)

#define UVOICE_API_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_MICIN_NUM 8
#define MAX_REFIN_NUM 2

// #include "uvoice_sdk_define.h"

typedef enum uvoice_api_return_code {
    UVOICE_API_OK = 0,
    UVOICE_API_NULL_ERROR = -1,
    UVOICE_API_STREAMING_ERROR = -2,
    UVOICE_API_RECOGNIZE_ERROR = -3
} uvoice_api_code;

typedef struct uvoice_audio_buf {
    short *audioin[MAX_MICIN_NUM];
    short *audioref[MAX_REFIN_NUM];
}uvoice_stream;

uvoice_api_code uvoice_sdk_init(void** handle);

uvoice_api_code uvoice_sdk_process(void* handle, uvoice_stream* audio, char* out, int sdk_model);

uvoice_api_code uvoice_sdk_deinit(void* handle);

uvoice_api_code uvoice_sdk_enable_vad(void* handle);

uvoice_api_code uvoice_sdk_disable_vad(void* handle);

uvoice_api_code uvoice_sdk_change_vad_eos(void* handle, int change_vad_eos);

uvoice_api_code uvoice_sdk_change_wakeup_value(void* handle, float value);

#ifdef __cplusplus
}
#endif

#endif
