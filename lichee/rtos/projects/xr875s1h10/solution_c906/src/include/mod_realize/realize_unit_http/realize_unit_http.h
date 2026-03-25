#ifndef __REALIZE_UNIT_HTTP_H__
#define __REALIZE_UNIT_HTTP_H__
#include "../../platform/gulitesf_config.h"

#define HTTP_PKG_PUSH_MAX 6

#define HTTP_DEFAULT_PORT 80
#define HTTP_BODY_FRAG_SIZE 2800
#ifndef CONFIG_PLATFORM_RTOS
#define HTTP_DOWNLOAD_BUF_SIZE	(20 * 1024)
#define HTTP_DOWNLOAD_PUT_BUF_SIZE (128 * 2 * 1024)
#else
#define HTTP_DOWNLOAD_BUF_SIZE	2048
#define HTTP_DOWNLOAD_PUT_BUF_SIZE (128 * 1024)
#endif
#define MAX_RECV_ZERO_LEN_CNT 2

#define BUFFER_SIZE 1024
#define SOCKECT_WRITE_RETRY_CNT 2

typedef enum {
    SOCKECT_WRITE_FAIL_EXIT = -1,
    SOCKECT_WRITE_FAIL_UNDO,
    SOCKECT_WRITE_FAIL_RETRY,
} write_fail_proc_t;

typedef enum {
	HTTP_DOWNLOAD_START = 0,	//准备下载
	HTTP_DOWNLOAD_PROCESS,		//正在下载
	HTTP_DOWNLOAD_OVER,			//结束下载
} t_download_state;

typedef enum {
	HTTP_NORMAL_TYPE 			= 0,	//普通方式
	HTTP_FORM_DATA_TYPE,				//form-data方式
}http_upload_type;

typedef enum {
	DOWNLOAD_ERR_FORM_NET 			= -1,	//网络原因错误
	DOWNLOAD_ERR_FORM_FILE			= -2,   //文件操作错误
	DOWNLOAD_ERR_FORM_OTHER			= -3, 	//其他未知错误
} http_download_err_type;

typedef struct {
	int (*download_start_cb)(void *info, int total_size, int cur_size, int pos);
	int (*download_progress_cb)(void *info, int total_size, int cur_size, int pos);
	int (*download_end_cb)(void *info, int total_size, int cur_size, int pos);
	int (*download_error_cb)(void *info, http_download_err_type err, int total_size, int cur_size, int pos);
    int (*download_delete_cb)(void *info);
} http_download_cb_t;


typedef struct {
	char *url;
	char *path;
	int offset;
	int timeout;
	void *user_data;
	int is_abort;
	http_download_cb_t cb;
} http_download_info_t;


typedef struct  {//保持相应头信息
    int status_code;        // HTTP/1.1 '200' OK
    // char content_type[128];  Content-Type: application/gzip
    int content_length;    // Content-Length: 11683079
	unsigned char chunked;
	int cur_load;
	char *info;
}resp_header_t;


typedef enum {
	HTTP_CONNECT_OK,
	HTTP_PARSE_URL_FAIL,
	HTTP_CREATE_SOCKET_FAIL,
	HTTP_MALLOC_FAIL,
	HTTP_SEND_MSG_FAIL,
	HTTP_GET_HEADER_FAIL,
	HTTP_GET_WRONG_STATUE_CODE,
	HTTP_RECV_DATA_FAIL,
	HTTP_CONNECT_TIMES_MAX,
	HTTP_OVER_MAX_REQ,
} t_err_type;

typedef struct {
	char *headerContent;
	char *body;
	char *data;
	int data_len;
	int timeout;
} http_request_header_info_t;

typedef struct {
	char *data;
	int size;
} unit_http_data_info_t;

typedef struct _unit_http_request{
	void *user_data;
	int timeout;
	void *socket;
	int is_https;
	char *redirect_url;
	int destroy_sign;
	http_upload_type type;
	unsigned short pkg_interval[HTTP_PKG_PUSH_MAX];
	void (*http_connect_cb)(struct _unit_http_request *, resp_header_t *);
	void (*http_data_cb)(struct _unit_http_request *, char *, int);
	void (*http_error_cb)(struct _unit_http_request *, int );
	void (*http_end_cb)(struct _unit_http_request *, resp_header_t *, char *, int);
	void (*http_delete_cb)(struct _unit_http_request *);
} unit_http_request_t;

typedef struct _unit_sync_resp {
    int status_code;
    char *headers;
    unit_http_data_info_t body;
} unit_sync_resp_t;

typedef struct _unit_http_sync_req {
    unit_http_request_t unit_req;
    unit_sync_resp_t resp_info;
} unit_http_sync_request_t;

int realize_unit_http_download(http_download_info_t *download_info, char *header);

//兼容原来的接口 原生使用
int realize_unit_http_request(const char *url, const char *method, const char *header, const char *body, int timeout, char **data, int *size);

int realize_unit_http_request_header(unit_http_request_t *http_req, char *url, char *header, char *method);
int realize_unit_http_request_body(unit_http_request_t *http_req, char *body, int body_len);
int realize_unit_http_request_end(unit_http_request_t *http_req);
void realize_unit_http_obj_delete(unit_http_request_t *http_req);
int realize_unit_http_send(int socket, char *buff, int size);
int realize_unit_http_read(int socketfd, unsigned char *buffer, int len, int timeout_ms);
int realize_unit_http_htoi(char s[]);
resp_header_t *realize_unit_http_read_resp_header(unit_http_request_t *http_req);
int realize_unit_http_send_end_header(unit_http_request_t *http_req);
void realize_unit_http_close_socket(unit_http_request_t *http_req);
int realize_unit_http_get_chunked_resp_once(unit_http_request_t *http_req, char *response);

int realize_unit_http_sync_request_end(unit_http_sync_request_t *http_req);

//工具类接口
int realize_unit_http_parse_url(const char *url, char **host, char **file, int *port);
void *realize_unit_http_create(const char *host, int port);
write_fail_proc_t socket_write_retry(unsigned int retry_cnt, long wait_time);
#endif
