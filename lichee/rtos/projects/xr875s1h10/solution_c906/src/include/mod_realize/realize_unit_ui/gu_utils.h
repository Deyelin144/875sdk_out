/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.14
  *Description: 工具
  *History:  
**********************************************************************************/
#include "../../components/gu_json/gu_json.h"
#include "../realize_unit_log/realize_unit_log.h"
#include "../realize_unit_kv/realize_unit_kv.h"
#include "gu_lvgl.h"
#include "lvgl.h"

#ifndef GU_UTILS_H
#define GU_UTILS_H
#define LOG_LV 0
#if LOG_LV
#define gu_log_lv(...) {printf(__VA_ARGS__);if(get_lock_status() == false) printf("\n%s %d.........................lv not in lock................\n", __func__, __LINE__);}
#else
#define gu_log_lv(...) 
#endif
#define printDuration(last_time, max_dur, format, ...) { \
    long dur = wrapper_get_handle()->os.get_time_ms() - last_time; \
    if(dur >= max_dur){ \
        realize_unit_log_error(format" dur=%ld", ##__VA_ARGS__, dur); \
    } \
};

typedef enum {
    EVA_EXP_TYPE_NUMBER,
    EVA_EXP_TYPE_STRING
} ValueType;

typedef struct {
    ValueType type;
    union {
        int number;
        char *string;
    } data;
} Value;

typedef struct {
    Value **data;
    int top;
    int capacity;
} Stack;

Value* evaluateExpression(char* tokens);
void freeValue(Value *value);
int get_error_cost_time();
char *get_expression_value(const char *expression, guJSON *data);
char * kv_get(const char *key);
int kv_set(const char *key, const char *value);
int set_kv_option(const char *option);
int saveToFile(char *path, char *data, int size);
int get_str_md5(const char *url, char *md5out);
bool check_compose_file_is_exist(const char *path);
lv_font_t *realize_unit_ui_loadfont(const char *path);
char *getLocaleValue(char *path, void *curApp);
guJSON *get_json_item_by_path(guJSON *data,const char *data_path);
void set_json_item_by_path(guJSON *data, const char *data_path, const guJSON *jsondata);
guJSON * get_js_json_item_by_path(const char *data_path, bool is_get_type_only);
char *get_js_json_string_data_by_path(const char *data_path);
char *get_json_string_data_by_path(guJSON *data, const char *data_path);
char *get_json_to_string(guJSON *cur_node);
void updJsonStringValue(guJSON *updItem, const char *value);
void updJsonStringByPathValue(guJSON *root, const char *path, const char *value);
int getStrInt(char *str, int ret[], int count);
char *ContentOfFile(const char *file_name, int *out_size);
char *clear_right(char *p);
const char *clear_left(const char *p);
char *cutstr(char *start, char *end); 
char *cut_string( const char *start,  char *end);
char* gu_base64_decode(const char *source, int *size);
int enc_unicode_to_utf8_one(unsigned long unic, char *pOutput,
        int outSize);
int xz_unzip(const char *in_buff, int in_size, char **out_buff, unsigned int max_buff_size, unsigned int *size);
int read_unzip_file(const char *path, unsigned char **buf, unsigned int *len);
void replaceSubstring(char *str, const char *oldSub, const char *newSub);
int isInt(char *str);
int unzip_buffer(const char *data, unsigned int size, char **buf,  unsigned int *len);
int realize_unit_ui_unzip_file(const char *source_filename, const char *dest_filename);
void set_app_static_mode(bool mode);
bool get_app_static_mode(void);
#endif

