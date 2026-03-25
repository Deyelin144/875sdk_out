#ifndef __REALIZE_UNIT_RECORD_H__
#define __REALIZE_UNIT_RECORD_H__

#define  CONFIG_RECORD_INTERNAL    0
#define  RECORD_TIMEOUT         10000       //10s
#define  RECORD_LIST_LEN        600

typedef struct {
	unsigned int rate;				// 采样率
	unsigned int channels;			// 通道数
	unsigned short format;			// pcm格式
	unsigned short period_size;		// 周期
	unsigned short period_count;	// 周期数
} record_pcm_config_t;

typedef enum {
    RECORD_NONE,
    RECORD_STRAT,
    RECORD_PROGRESS,
    RECORD_FINISH
} record_state_t;

typedef enum {
    RECORD_AMR_TYPE,
    RECORD_SPEEX_TYPE,
    RECORD_OPUS_TYPE,
    RECORD_PCM_TYPE,
} record_auido_type_t;

typedef struct {
	void *codec;
    unsigned char need_header;
	int (*encode_start)(void *, void *, void *);
	int (*encode_write)(void *, void *, int, void *, int *);
	void (*encode_stop)(void *);
	int (*encode_get_header)(void *, const char **, int *);
	int (*encode_get_param)(void *, const char **); 
	int (*encode_set_param_using_json)(void *, char *);
} record_encode_context_t;

typedef struct {
	record_pcm_config_t pcm_config;
	int (*set_pcm_param_using_json)(void *context, char *json_str);
} record_pcm_context_t;

typedef struct {
    void *data;
    int len;
	int frame_num;
} record_data_t;

typedef struct {
	void *user_data;
    char *path;
	record_state_t record_state;
    record_auido_type_t type;
	record_data_t record_data;
    record_pcm_context_t pcm_context;
	void *json_param;
    record_encode_context_t *encode_context;
} record_info_t;

typedef struct rec_context {
	record_info_t record_info;
	int (*record_start)(struct rec_context **);
	void (*record_stop)(struct rec_context **);
	int (*record_start_cb)(struct rec_context *);
	int (*record_get_data_cb)(struct rec_context *);
	int (*record_stop_cb)(struct rec_context **);//异步停止回调
	int (*record_err_cb)(struct rec_context **, int);
} record_context_t;

typedef void(*cb_free)(void **);

record_context_t *realize_unit_record_create(void);
void realize_unit_record_distory(record_context_t **record_context, cb_free user_free);
#endif