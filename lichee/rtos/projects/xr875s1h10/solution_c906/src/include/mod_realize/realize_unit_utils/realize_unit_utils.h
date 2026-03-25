#ifndef _REALIZE_UNIT_UTILS_H_
#define _REALIZE_UNIT_UTILS_H_
#include <stdint.h>
#include "wrapper/socket_wrapper/socket_wrapper.h"
/**********************start***************************/
/*
	全局使用的，push pointer 的数据结构，
	各个模块解析需要按照这个结构进行
*/
typedef enum {
	DATA_BINARY,
	DATA_STRING
} unit_utils_data_type_t;

typedef struct {
	char* content;		   			//发送内容
	unsigned int size;				//发送内容大小
	unit_utils_data_type_t type;	//发送内容类型
} unit_utils_send_data_t;
/**************************end*************************/

int realize_unit_utils_create_random(void);
void realize_unit_utils_nv12torgb565(unsigned int width, unsigned int height , unsigned char *yuyv , unsigned char *rgb);
void realize_unit_utils_nv12torgb565_be(unsigned int width, unsigned int height , unsigned char *yuyv , unsigned char *rgb);
unsigned char realize_unit_utils_is_utf8(const char* str, int max_length);
unsigned char realize_unit_utils_is_cesu8(const unsigned char *data, int len);
void realize_unit_utils_utf8_to_cesu8(const char *utf8_str, unsigned char *cesu8_buf, int *cesu8_len);
int realize_unit_utils_resolve_hostname(const char *host_name, struct addrinfo *hints, struct in_addr *addr);

// 检查 UTF-8 字符串是否被截断
int realize_unit_utils_utf8_check_tail_truncation(const char *buf, size_t len, int *out_trim_len);
#endif

