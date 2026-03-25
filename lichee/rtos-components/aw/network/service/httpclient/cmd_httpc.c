/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cmd_util.h"
#include "HTTPCUsr_api.h"
#include "mbedtls/mbedtls.h"
#include <console.h>

#if defined(CONFIG_ARCH_SUN20IW2) && defined(CONFIG_ARCH_RISCV_RV64)
#define HTTPC_THREAD_STACK_SIZE        (10 * 1024)
#else
#define HTTPC_THREAD_STACK_SIZE        (4 * 1024)
#endif

#define CMD_HTTPC_LOG printf

struct cmd_httpc_common {
	XR_OS_Thread_t thread;
	char arg_buf[1024];
	security_client user_param;
};

static struct cmd_httpc_common cmd_httpc;

extern const char   mbedtls_test_cas_pem[];
extern const size_t mbedtls_test_cas_pem_len;

void *get_certs(void)
{
	memset(&cmd_httpc.user_param, 0, sizeof(cmd_httpc.user_param));
	cmd_httpc.user_param.pCa = (char *)mbedtls_test_cas_pem;
	cmd_httpc.user_param.nCa = mbedtls_test_cas_pem_len;
	return &cmd_httpc.user_param;
}

static unsigned char data_checksum;
static int cmd_data_process_init(void)
{
	data_checksum = 0;
	return 0;
}

static int cmd_data_process(void *buffer, int length)
{
	unsigned char *cal = (unsigned char *)buffer;
	while (length != 0) {
		data_checksum += cal[--length];
	}
	return 0;
}

static int cmd_data_process_deinit(void)
{
	CMD_HTTPC_LOG("[httpc cmd test]:checksum = %#x\n", data_checksum);
	data_checksum = 0;
	return 0;
}

static int httpc_get(char *url)
{
	int ret = -1;
	int recvSize = 0;
	char *buf = NULL;
	unsigned int toReadLength = 4096;
	HTTPParameters *clientParams = NULL;

	uint32_t recvStartTime  = 0, recvEndTime = 0;
	double recvAllDataSize = 0, rateBytes = 0;

	clientParams = malloc(sizeof(HTTPParameters));
	if (clientParams == NULL) {
		goto exit0;
	}
	memset(clientParams, 0, sizeof(HTTPParameters));

	buf = malloc(toReadLength);
	if (buf == NULL) {
		goto exit0;
	}
	memset(buf, 0, toReadLength);

	strcpy(clientParams->Uri, url);
	//cmd_data_process_init();
	recvStartTime = XR_OS_GetTicks();
	do {
		ret = HTTPC_get(clientParams, buf, 4096, (INT32 *)&recvSize);
		if ((ret != HTTP_CLIENT_SUCCESS) && (ret != HTTP_CLIENT_EOS)) {
			CMD_ERR("Transfer err...ret:%d\n", ret);
			ret = -1;
			break;
		}
		//cmd_data_process(buf, recvSize);
		recvAllDataSize += recvSize;
		if (ret == HTTP_CLIENT_EOS) {
			CMD_DBG("The end..\n");
			ret = 0;
			break;
		}
	} while (1);
	recvEndTime = XR_OS_GetTicks();
	rateBytes = ((recvAllDataSize / (recvEndTime - recvStartTime)) * 1000) / (1024 * 1024);
	printf("=======> total bytes : %f Mbytes - rate : %f MBytes/s\n", recvAllDataSize / (1024 * 1024), rateBytes);
	//cmd_data_process_deinit();

exit0:
	//CMD_HTTPC_LOG("buf:%s\n", buf);
	free(buf);
	free(clientParams);
	return ret;
}

static int httpc_get_fresh(char *url, char *user_name, char *password,
                           int use_ssl)
{
	int ret = -1;
	char *buf = NULL;
	unsigned int toReadLength = 4096;
	unsigned int Received = 0;
	HTTP_CLIENT httpClient;
	HTTPParameters *clientParams = NULL;

	clientParams = malloc(sizeof(HTTPParameters));
	if (clientParams == NULL) {
		goto exit0;
	}
	memset(clientParams, 0, sizeof(HTTPParameters));

	buf = malloc(toReadLength);
	if (buf == NULL) {
		goto exit0;
	}
	memset(buf, 0, toReadLength);

	memset(&httpClient, 0, sizeof(httpClient));
	clientParams->HttpVerb = VerbGet;
	strcpy(clientParams->Uri, url);
	if ((user_name) && (password)) {
		cmd_strlcpy(clientParams->UserName, user_name,
		            sizeof(clientParams->UserName));
		cmd_strlcpy(clientParams->Password, password,
		            sizeof(clientParams->Password));
		clientParams->AuthType = AuthSchemaDigest;
	}

	if (use_ssl) {
		HTTPC_Register_user_certs(get_certs);
		//HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_OPTIONAL);
	}

	ret = HTTPC_open(clientParams);
	if (ret != 0) {
		CMD_ERR("http open err..\n");
		goto exit0;
	}

	ret = HTTPC_request(clientParams, NULL);
	if (ret != 0) {
		CMD_ERR("http request err..\n");
		goto exit1;
	}

	ret = HTTPC_get_request_info(clientParams, &httpClient);
	if (ret != 0) {
		CMD_ERR("http get request info err..\n");
		goto exit1;
	}

	cmd_data_process_init();
	if (httpClient.TotalResponseBodyLength != 0) {
		do {
			ret = HTTPC_read(clientParams, buf, toReadLength,
			                 (void *)&Received);
			if ((ret != HTTP_CLIENT_SUCCESS) && (ret != HTTP_CLIENT_EOS)) {
				CMD_ERR("Transfer err...ret:%d\n", ret);
				ret = -1;
				break;
			}
			cmd_data_process(buf, Received);
			if (ret == HTTP_CLIENT_EOS) {
				CMD_DBG("The end..\n");
				ret = 0;
				break;
			}
		} while (1);
	}
	cmd_data_process_deinit();

exit1:
	HTTPC_close(clientParams);
exit0:
	free(buf);
	free(clientParams);
	return ret;
}

static int httpc_post(char *url, char *credentials, int use_ssl)
{
	int   ret = 0;
	char *buf = NULL;
	unsigned int toReadLength = 4096;
	unsigned int Received = 0;
	HTTP_CLIENT  httpClient;
	HTTPParameters *clientParams = NULL;

	clientParams = malloc(sizeof(HTTPParameters));
	if (clientParams == NULL) {
		goto exit0;
	}
	memset(clientParams, 0, sizeof(HTTPParameters));

	buf = malloc(toReadLength);
	if (buf == NULL) {
		goto exit0;
	}
	memset(buf, 0, toReadLength);

	memset(&httpClient, 0, sizeof(httpClient));
	clientParams->HttpVerb = VerbPost;
	clientParams->pData = buf;
	strcpy(clientParams->Uri, url);
	clientParams->pLength = strlen(credentials);
	memcpy(buf, credentials, strlen(credentials));

request:
	ret = HTTPC_open(clientParams);
	if (ret != 0) {
		CMD_ERR("http open err..\n");
		goto exit0;
	}

	ret = HTTPC_request(clientParams, NULL);
	if (ret != 0) {
		CMD_ERR("http request err..\n");
		goto exit1;
	}

	ret = HTTPC_get_request_info(clientParams, &httpClient);
	if (ret != 0) {
		CMD_ERR("http get request info err..\n");
		goto exit1;
	}

	if (httpClient.HTTPStatusCode != HTTP_STATUS_OK) {
		if ((httpClient.HTTPStatusCode == HTTP_STATUS_OBJECT_MOVED) ||
				(httpClient.HTTPStatusCode == HTTP_STATUS_OBJECT_MOVED_PERMANENTLY)) {
			CMD_DBG("Redirect url..\n");
			HTTPC_close(clientParams);
			memset(clientParams, 0, sizeof(*clientParams));
			clientParams->HttpVerb = VerbGet;
			if (httpClient.RedirectUrl->nLength < sizeof(clientParams->Uri)) {
				strncpy(clientParams->Uri, httpClient.RedirectUrl->pParam,
						httpClient.RedirectUrl->nLength);
			} else {
				goto exit0;
			}
			CMD_DBG("go to request.\n");
			goto request;

		} else {
			ret = -1;
			CMD_DBG("get result not correct, status: %d\n", httpClient.HTTPStatusCode);
			goto exit1;
		}
	}
	cmd_data_process_init();
	if (httpClient.TotalResponseBodyLength != 0
		|| (httpClient.HttpFlags & HTTP_CLIENT_FLAG_CHUNKED)) {
		do {
			ret = HTTPC_read(clientParams, buf, toReadLength,
			                 (void *)&Received);
			if ((ret != HTTP_CLIENT_SUCCESS) && (ret != HTTP_CLIENT_EOS)) {
				CMD_ERR("Transfer err...ret:%d\n", ret);
				ret = -1;
				break;
			}
			cmd_data_process(buf, Received);
			if (ret == HTTP_CLIENT_EOS) {
				CMD_DBG("The end..\n");
				ret = 0;
				break;
			}
		} while (1);
	}
	cmd_data_process_deinit();

exit1:
	HTTPC_close(clientParams);
exit0:
	free(buf);
	free(clientParams);
	return ret;
}

static int httpc_muti_get(char *url0, char *url1)
{
	int ret = -1;
	char *buf = NULL;
	char *pUrl0 = url0;
	char *pUrl1 = url1;
	unsigned int toReadLength = 4096;
	unsigned int Received = 0;
	HTTP_CLIENT httpClient;
	HTTPParameters *clientParams = NULL;

	clientParams = malloc(sizeof(HTTPParameters));
	if (clientParams == NULL) {
		goto exit0;
	}
	memset(clientParams, 0, sizeof(HTTPParameters));

	buf = malloc(toReadLength);
	if (buf == NULL) {
		goto exit0;
	}
	memset(buf, 0, toReadLength);

	memset(&httpClient, 0, sizeof(httpClient));
	clientParams->HttpVerb = VerbGet;
	clientParams->Flags |= HTTP_CLIENT_FLAG_KEEP_ALIVE;
	cmd_strlcpy(clientParams->Uri, pUrl0, sizeof(clientParams->Uri));

	ret = HTTPC_open(clientParams);
	if (ret != 0) {
		CMD_ERR("http open err..\n");
		goto exit0;
	}
	pUrl0 = NULL;
	goto direct_request;
set_url1:
	HTTPC_reset_session(clientParams);
	cmd_strlcpy(clientParams->Uri, pUrl1, sizeof(clientParams->Uri));
	CMD_DBG("http test get url1..\n");
	pUrl1 = NULL;
direct_request:
	ret = HTTPC_request(clientParams, NULL);
	if (ret != 0) {
		CMD_ERR("http request err..\n");
		goto exit1;
	}

	ret = HTTPC_get_request_info(clientParams, &httpClient);
	if (ret != 0) {
		CMD_ERR("http get request info err..\n");
		goto exit1;
	}

	cmd_data_process_init();
	if (httpClient.TotalResponseBodyLength != 0) {
		do {
			ret = HTTPC_read(clientParams, buf, toReadLength,
			                 (void *)&Received);
			if ((ret != HTTP_CLIENT_SUCCESS) && (ret != HTTP_CLIENT_EOS)) {
				CMD_ERR("Transfer err...ret:%d\n", ret);
				ret = -1;
				break;
			}
			cmd_data_process(buf, Received);
			if (ret == HTTP_CLIENT_EOS) {
				CMD_DBG("The end..\n");
				ret = 0;
				break;
			}
		} while (1);
	}
	cmd_data_process_deinit();

	if ((ret == 0) && (pUrl1 != NULL)) {
		goto set_url1;
	}

exit1:
	HTTPC_close(clientParams);
exit0:
	free(buf);
	free(clientParams);
	return ret;
}

#define USE_KEEP_ALIVE 0 /* http keepalive example */
#if USE_KEEP_ALIVE
#define MAX_HOSTNAME_LEN 128

struct list_index {
	struct list_index *next, *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}
#define LIST_HEAD_DEF(name) \
struct list_index name = LIST_HEAD_INIT(name)

LIST_HEAD_DEF(g_http_session_list);
LIST_HEAD_DEF(g_https_session_list);

enum {
	IDLE,
	BUSY
};

typedef struct {
	HTTP_SESSION_HANDLE session;
	char hostname[MAX_HOSTNAME_LEN];
	char state;
	char idle_time;
	struct list_index list;
} session_item_t;

#define list_for_each(pos, list_head) \
	for (pos = (list_head)->next; pos != list_head; pos = pos->next)

#ifndef __DEQUALIFY
#define __DEQUALIFY(type, var) ((type)(uintptr_t)(const volatile void *)(var))
#endif

#ifndef offsetof
#define offsetof(type, field) \
	((size_t)(uintptr_t)((const volatile void *)&((type *)0)->field))
#endif

#ifndef __containerof
#define __containerof(ptr, type, field) \
	__DEQUALIFY(type	*, (const volatile char *)(ptr) - offsetof(type, field))
#endif

#ifndef container_of
#define container_of(ptr, type, field) __containerof(ptr, type, field)
#endif

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

static inline void __list_add(struct list_index *new, \
		struct list_index *prev, \
		struct list_index *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void __list_del(struct list_index * prev, struct list_index * next)
{
	next->prev = prev;
	prev->next = next;
}

static inline int list_empty(const struct list_index *head)
{
	return head->next == head;
}

static inline void list_add(struct list_index *new, struct list_index *head)
{
	__list_add(new, head, head->next);
}

static inline void list_del(struct list_index *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = entry;
	entry->prev = entry;
}

#define LOCK_LIST 1
#if LOCK_LIST
#define LOCK_TRACE(f) //do { if(f) printf("%s lock\n",__func__); \
	else printf("%s unlock\n",__func__);} while(0)
XR_OS_Mutex_t g_http_list_mutex;
XR_OS_Mutex_t g_https_list_mutex;

int htkp_list_mutex_create(void)
{
	XR_OS_Mutex_t *mutex = NULL;

	for (int num = 0; num < 2; num ++) {
		mutex = num? &g_https_list_mutex : &g_http_list_mutex;

		if (mutex == NULL)
			return -1;

		if(!XR_OS_MutexIsValid(mutex)) {
			if (XR_OS_RecursiveMutexCreate(mutex) == XR_OS_FAIL) {
				printf("[HTKP] list mutex creat failed!.\n");
				return -1;
			}
			printf("[HTKP] list mutex creat success!.\n");
		}
	}

	return 0;
}

int htkp_list_lock(char is_https)
{
	XR_OS_Mutex_t *mutex = NULL;
	XR_OS_Status ret;

	LOCK_TRACE(1);
	mutex = is_https? &g_https_list_mutex : &g_http_list_mutex;

	if (mutex == NULL)
		return -1;

	if(!XR_OS_MutexIsValid(mutex)) {
		printf("[HTKP] lock. list mutex is invalid.\n");
		return -1;
	}

	ret = XR_OS_RecursiveMutexLock(mutex, XR_OS_WAIT_FOREVER);

	if (ret != XR_OS_OK) {
		printf("[HTKP] list lock failed. ret : %d\n", ret);
		return -1;
	}

	return 0;
}

int htkp_list_unlock(char is_https)
{
	XR_OS_Mutex_t *mutex = NULL;
	XR_OS_Status ret;

	LOCK_TRACE(0);
	mutex = is_https? &g_https_list_mutex : &g_http_list_mutex;

	if (mutex == NULL)
		return -1;

	if(!XR_OS_MutexIsValid(mutex)) {
		printf("[HTKP] unlock. list mutex is invalid.\n");
		return -1;
	}

	ret  = XR_OS_RecursiveMutexUnlock(mutex);

	if (ret != XR_OS_OK) {
		printf("[HTKP] list unlock failed. ret : %d\n", ret);
		return -1;
	}

	return 0;
}

int htkp_list_mutex_delete(void)
{
	XR_OS_Mutex_t *mutex = NULL;

	for (int num = 0; num < 2; num ++) {
		mutex = num? &g_https_list_mutex : &g_http_list_mutex;

		if(XR_OS_MutexIsValid(mutex)) {
			XR_OS_RecursiveMutexDelete(mutex);
			XR_OS_MutexSetInvalid(mutex);
			printf("[HTKP] list mutex deleted!.\n");
		}
	}

	return 0;
}

#else
int htkp_list_mutex_create(void)	{return 0;};
int htkp_list_lock(char is_https)	{return 0;};
int htkp_list_unlock(char is_https) {return 0;};
int htkp_list_mutex_delete(void)	{return 0;};
#endif

#define API_TRACE(fmt) //printf("[HTKP] [%s %d]======= "fmt"\n", __func__, __LINE__);
char *htkp_get_hostname_by_url(char *url, char *is_https)
{
	char *pUrl = url;
	int url_hostname_len = 0;
	char *url_hostname_start = NULL;
	char *pa, *pb, *pc;
	static char hostname[MAX_HOSTNAME_LEN] = {0};

	API_TRACE("ENTER");
	if (!strncmp(pUrl, "http:", 5)) {
		*is_https = 0;
	} else if (!strncmp(pUrl, "https", 5)) {
		*is_https = 1;
	} else {
		printf("[HTKP] URL IS INVALID\n");
		API_TRACE("LEAVE");
		return NULL;
	}

	pa = strchr(pUrl, ':');//http://xxxxxxxxx/
	pb = pa + 3;//xxxxxxxx/
	pc = strchr(pb, '/');

	url_hostname_len = pc - pb;
	url_hostname_start = pb;

	if (url_hostname_start == NULL || url_hostname_len == 0) {
		printf("[HTKP] GET HOSTNME FAILED!\n");
		API_TRACE("LEAVE");
		return NULL;
	}

	memset(hostname, 0, MAX_HOSTNAME_LEN);
	memcpy(hostname, url_hostname_start, url_hostname_len);
	printf("[HTKP] GET HOSTNME: %s\n", hostname);
	API_TRACE("LEAVE");

	return hostname;
}

struct list_index *htkp_select_list(char is_https)
{
	struct list_index *session_list;

	if (is_https == 0) {
		session_list = &g_http_session_list;
		//printf("[HTKP] [HTTP]\n");
	} else if (is_https == 1) {
		session_list = &g_https_session_list;
		//printf("[HTKP] [HTTPS]\n");
	} else {
		printf("[HTKP] url is not http or https\n");
		return NULL;
	}

	return session_list;
}

int htkp_show_list_info(char is_https)
{
	struct list_index *pos;
	struct list_index *session_list;
	session_item_t *item = NULL;

	API_TRACE("ENTER");

	session_list = htkp_select_list(is_https);

	if (list_empty(session_list)) {
		printf("[HTKP] SESSION LIST IS EMPTY!!\n");
		API_TRACE("LEAVE");
		return -1;
	}

	printf("[HTKP] SESSION LIST INFO:\n");
	htkp_list_lock(is_https);
	list_for_each(pos, session_list) {
		item = list_entry(pos, session_item_t, list);
		printf("[HTKP] SESSION: %p <-> HOSTNME: %s\n", \
				item->session, item->hostname);
	}
	htkp_list_unlock(is_https);

	API_TRACE("LEAVE");

	return 0;
}

HTTP_SESSION_HANDLE htkp_get_session_by_hostname (char *hostname, char is_https)
{
	struct list_index *pos, *session_list;
	session_item_t *item = NULL;

	API_TRACE("ENTER");

	session_list = htkp_select_list(is_https);

	if (hostname == NULL || list_empty(session_list)) {
		printf("[HTKP] HOSTNME IS ERROR OR LIST IS EMPTY\n");
		API_TRACE("LEAVE");
		return 0;
	}

	htkp_list_lock(is_https);
	list_for_each(pos, session_list) {
		item = list_entry(pos, session_item_t, list);
		if (item->session != 0) {
			if (memcmp(item->hostname, hostname, strlen(hostname)) == 0) {
				printf("[HTKP] GET SESSION: %p <-> HOSTNME: %s\n", \
					item->session, item->hostname);
				API_TRACE("LEAVE");
				htkp_list_unlock(is_https);
				return (HTTP_SESSION_HANDLE)(item->session);
			}
		}
	}
	htkp_list_unlock(is_https);

	API_TRACE("LEAVE");

	//not found
	return 0;
}

int htkp_save_session_host_to_list (HTTP_SESSION_HANDLE session, char *hostname, char is_https)
{
	struct list_index *session_list;
	session_item_t *item = NULL;

	API_TRACE("ENTER");

	session_list = htkp_select_list(is_https);

	item = malloc(sizeof(session_item_t));
	if (item == NULL) {
		printf("[HTKP] MALLOC ITEM FAILED\n");
		API_TRACE("LEAVE");
		return -1;
	}
	memset(item, 0, sizeof(session_item_t));
	item->session = session;
	memcpy(item->hostname, hostname, strlen(hostname));
	item->state = BUSY;
	item->idle_time = 0;

	htkp_list_lock(is_https);
	list_add(&item->list, session_list);
	htkp_list_unlock(is_https);

	API_TRACE("LEAVE");

	return 0;
}

int htkp_delete_session_host_of_list (char *hostname, char is_https)
{
	struct list_index *pos;
	struct list_index *session_list;
	session_item_t *item = NULL;

	API_TRACE("ENTER");

	session_list = htkp_select_list(is_https);

	if (hostname == NULL || list_empty(session_list)) {
		printf("[HTKP] HOSTNME IS ERROR OR LIST IS EMPTY\n");
		API_TRACE("LEAVE");
		return -1;
	}

	htkp_list_lock(is_https);
	list_for_each(pos, session_list) {
		item = list_entry(pos, session_item_t, list);
		if (item->session != 0) {
			if (memcmp(item->hostname, hostname, strlen(hostname)) == 0) {
				printf("[HTKP] DELETE SESSION: %p <-> HOSTNME: %s\n", \
					item->session, item->hostname);
				list_del(&item->list);
				free(item);
				htkp_list_unlock(is_https);
				API_TRACE("LEAVE");
				return 0;
			}
		}
	}
	htkp_list_unlock(is_https);

	API_TRACE("LEAVE");

	return -1;
}

int htkp_set_session_state (char *hostname, char state, char is_https)
{
	struct list_index *pos;
	struct list_index *session_list;
	session_item_t *item = NULL;

	API_TRACE("ENTER");

	session_list = htkp_select_list(is_https);

	if (hostname == NULL || list_empty(session_list)) {
		printf("[HTKP] HOSTNME IS ERROR OR LIST IS EMPTY\n");
		API_TRACE("LEAVE");
		return -1;
	}

	htkp_list_lock(is_https);
	list_for_each(pos, session_list) {
		item = list_entry(pos, session_item_t, list);
		if (item->session != 0) {
			if (memcmp(item->hostname, hostname, strlen(hostname)) == 0) {
				item->state = state;
				item->idle_time = 0;
				//API_TRACE("LEAVE");
				htkp_list_unlock(is_https);
				return 0;
			}
		}
	}
	htkp_list_unlock(is_https);

	API_TRACE("LEAVE");

	return -1;
}

static XR_OS_Timer_t g_checkKeepAliveIdle_timer;
int htkp_stop_checkKeepAliveIdle_timer(void)
{
	if (XR_OS_TimerIsValid(&g_checkKeepAliveIdle_timer)) {
		XR_OS_TimerStop(&g_checkKeepAliveIdle_timer);
		XR_OS_TimerDelete(&g_checkKeepAliveIdle_timer);
		XR_OS_TimerSetInvalid(&g_checkKeepAliveIdle_timer);
		printf("[HTKP] STOP TIMER\n");
	}

	return 0;
}

void htkp_checkKeepAliveIdle_tmr(void *arg)
{
	struct list_index *pos;
	struct list_index *session_list;
	session_item_t *item = NULL;
	char idle_list_num = 0;

	API_TRACE("ENTER");

	(void) arg;
	for (int num = 0; num < 2; num ++) {
		session_list = htkp_select_list(num); //0:http 1:https

		if (list_empty(session_list)) {
			//printf("[HTKP] SESSION LIST %d IS EMPTY!!\n", num);
			idle_list_num ++;
			continue;
		}

		htkp_list_lock(num);
		list_for_each(pos, session_list) {
			item = list_entry(pos, session_item_t, list);
			if (item->session > 0) {
				if (item->idle_time == 10) {
					//delete idle timeout session
					list_del(&item->list);
				    printf("[HTKP] DELETE SESSION: %p <-> HOSTNME: %s\n", \
							item->session, item->hostname);
					HTTPClientCloseRequest(&item->session);
					item->idle_time = 0;
					item->session = 0;
					//list_del(&item->list);
					free(item);
					htkp_list_unlock(num);
					return ;
				} else if (item->state == BUSY) {
					item->idle_time = 0;
				} else if (item->state == IDLE) {
					item->idle_time ++;
				}
				//printf("[HTKP] SESSION: %p <-> HOSTNME: %s <-> STATE: %d <-> IDLE: %d\n", \
						item->session, item->hostname, item->state, item->idle_time);
			}
		}
		htkp_list_unlock(num);
	}
	if (idle_list_num >= 2) {
		htkp_stop_checkKeepAliveIdle_timer();
	}
	API_TRACE("LEAVE");
}

int htkp_start_checkKeepAliveIdle_timer(unsigned int periodMS)
{
	if (!XR_OS_TimerIsValid(&g_checkKeepAliveIdle_timer)) {
		if (XR_OS_TimerCreate(&g_checkKeepAliveIdle_timer, \
					XR_OS_TIMER_PERIODIC, \
					htkp_checkKeepAliveIdle_tmr, \
					NULL, periodMS) != XR_OS_OK) {
			printf("[HTKP] check keepalive idle timer creat failed!\n");
			return -1;
		}
		XR_OS_TimerStart(&g_checkKeepAliveIdle_timer);
		printf("[HTKP] START TIMER\n");
	}

	return 0;
}

int htkp_delete_all_session(void)
{
	struct list_index *pos;
	struct list_index *session_list;
	session_item_t *item = NULL;

	API_TRACE("ENTER");

	for (int num = 0; num < 2; num ++) { //http_list \ https_list
		session_list = htkp_select_list(num);

		if (list_empty(session_list)) {
			printf("[HTKP] SESSION LIST IS EMPTY!!\n");
			continue;
		}

		printf("[HTKP] DELETE ALL SESSION:\n");
delete_next:
		list_for_each(pos, session_list) {
			item = list_entry(pos, session_item_t, list);
			list_del(&item->list);
			printf("[HTKP] DELETE SESSION: %p <-> HOSTNME: %s\n", \
						item->session, item->hostname);
			if (item->session > 0)
				HTTPClientCloseRequest(&item->session);
			free(item);
			goto delete_next;
		}
	}

	API_TRACE("LEAVE");

	return 0;
}

int htkp_init(void)
{
	htkp_list_mutex_create();

	return 0;
}

int htkp_deinit(void)
{
	htkp_stop_checkKeepAliveIdle_timer();
	htkp_delete_all_session();
	htkp_list_mutex_delete();

	return 0;
}

HTTP_SESSION_HANDLE     pHTTP;
int httpc_kp_get(char *url)
{
    int                 nRetCode;
	unsigned int        nSize,nTotal = 0;
	char                    Buffer[HTTP_CLIENT_BUFFER_SIZE];

#if USE_KEEP_ALIVE
	//find host and session.
	//not found mening first http get this server.
	char *hostname = NULL;
	char is_https = -1; //unkown
	htkp_init();
	hostname = htkp_get_hostname_by_url(url, &is_https);
	htkp_set_session_state (hostname, BUSY, is_https);
	pHTTP = 0;
	pHTTP = htkp_get_session_by_hostname(hostname, is_https);
	if (!pHTTP) {
		printf("[HTKP] =====> OPEN HTTP SESSION\n");
		pHTTP = HTTPClientOpenRequest(HTTP_CLIENT_FLAG_KEEP_ALIVE);
		if (pHTTP) {
			htkp_save_session_host_to_list(pHTTP, hostname, is_https);
			htkp_start_checkKeepAliveIdle_timer(1000); //1000ms
		}
	}
    htkp_show_list_info(is_https);
#else
	pHTTP = HTTPClientOpenRequest(0);
#endif /* USE_KEEP_ALIVE */

	HTTPClientReset(pHTTP);
	CMD_DBG("http get url: %s..\n", url);
    // Send a request for the home page
    if((nRetCode = HTTPClientSendRequest(pHTTP,url,NULL,0,FALSE,0,0)) != HTTP_CLIENT_SUCCESS)
    {
		printf("HTTPClientSendRequest: %d\n", nRetCode);
        goto exit1;
    }

    // Retrieve the the headers and analyze them
    if((nRetCode = HTTPClientRecvResponse(pHTTP,6)) != HTTP_CLIENT_SUCCESS)
    {
		printf("HTTPClientRecvResponse: %d\n", nRetCode);
        goto exit1;
    }

    // Get the data until we get an error or end of stream code
    // printf("Each dot represents %d bytes:\n",HTTP_BUFFER_SIZE );
    while(nRetCode == HTTP_CLIENT_SUCCESS || nRetCode != HTTP_CLIENT_EOS)
    {
        // Set the size of our buffer
        nSize = HTTP_CLIENT_BUFFER_SIZE;

        // Get the data
        nRetCode = HTTPClientReadData(pHTTP,Buffer,nSize,0,&nSize);
		if ((nRetCode != HTTP_CLIENT_SUCCESS) && (nRetCode != HTTP_CLIENT_EOS)) {
			CMD_ERR("Transfer err...ret:%d\n", nRetCode);
			goto exit1;
		}
        nTotal += nSize;
    }

#if USE_KEEP_ALIVE
	HTTP_CLIENT HTTPClient;
	HTTPClientGetInfo (pHTTP, &HTTPClient);
	if (HTTPClient.Connection == TRUE) { //the server support keep-alive?
		//do not close http session.
		//keep-alive session be close by idle timeout.
		htkp_set_session_state (hostname, IDLE, is_https);
	}
	else
#endif /* USE_KEEP_ALIVE */
exit1:
	{
		HTTPClientCloseRequest(&pHTTP);
#if USE_KEEP_ALIVE
		htkp_delete_session_host_of_list(hostname, is_https);
		htkp_show_list_info(is_https);
#endif /* USE_KEEP_ALIVE */
	}

	return 0;
}

static enum cmd_status cmd_httpc_kp_get(char *cmd)
{
	int ret;

	for (int i = 0; i < 1; i ++) {
		CMD_LOG("KP GET START=========== %d\n", i);
		ret = httpc_kp_get(cmd);
		CMD_LOG("KP GET OVER============ %d ret : %d\n", i, ret);
	}
	if (ret != 0) {
		return CMD_STATUS_FAIL;
	} else {
		return CMD_STATUS_OK;
	}
}
#endif /* USE_KEEP_ALIVE */

static enum cmd_status cmd_httpc_get(char *cmd)
{
	int ret;

	ret = httpc_get(cmd);
	if (ret != 0) {
		return CMD_STATUS_FAIL;
	} else {
		return CMD_STATUS_OK;
	}
}

static enum cmd_status cmd_httpc_post(char *cmd)
{
	int ret;
	int argc = 0;
	char *argv[2];

	argc = cmd_parse_argv(cmd, argv, 2);
	if (argc < 2) {
		CMD_ERR("invalid httpc cmd(post), argc %d\n", argc);
		return CMD_STATUS_FAIL;
	}

	ret = httpc_post(argv[0], argv[1], 0);
	if (ret != 0) {
		return CMD_STATUS_FAIL;
	} else {
		return CMD_STATUS_OK;
	}
}

static enum cmd_status cmd_httpc_get_fresh(char *cmd)
{
	int ret;

	ret = httpc_get_fresh(cmd, NULL, NULL, 0);
	if (ret != 0) {
		return CMD_STATUS_FAIL;
	} else {
		return CMD_STATUS_OK;
	}
}

static enum cmd_status cmd_httpc_head(char *cmd)
{
	return CMD_STATUS_OK;
}

static enum cmd_status cmd_httpc_auth_get(char *cmd)
{
	int ret;
	int argc = 0;
	char *argv[3];

	argc = cmd_parse_argv(cmd, argv, 3);
	if (argc < 3) {
		CMD_ERR("invalid httpc cmd(auth-get), argc %d\n", argc);
		return CMD_STATUS_FAIL;
	}

	ret = httpc_get_fresh(argv[0], argv[1], argv[2], 0);
	if (ret != 0) {
		return CMD_STATUS_FAIL;
	} else {
		return CMD_STATUS_OK;
	}
}

static enum cmd_status cmd_httpc_ssl_get(char *cmd)
{
	int ret;

	ret = httpc_get_fresh(cmd, NULL, NULL, 1);
	if (ret != 0) {
		return CMD_STATUS_FAIL;
	} else {
		return CMD_STATUS_OK;
	}
}

static enum cmd_status cmd_httpc_ssl_post(char *cmd)
{
	int ret;
	int argc = 0;
	char *argv[2];

	argc = cmd_parse_argv(cmd, argv, 2);
	if (argc < 2) {
		CMD_ERR("invalid httpc cmd(post), argc %d\n", argc);
		return CMD_STATUS_FAIL;
	}

	ret = httpc_post(argv[0], argv[1], 1);
	if (ret != 0) {
		return CMD_STATUS_FAIL;
	} else {
		return CMD_STATUS_OK;
	}
}

static enum cmd_status cmd_httpc_multi_get(char *cmd)
{
	int ret;
	int argc = 0;
	char *argv[2];

	argc = cmd_parse_argv(cmd, argv, 2);
	if (argc < 2) {
		CMD_ERR("invalid httpc cmd(multi-get), argc %d\n", argc);
		return CMD_STATUS_FAIL;
	}

	ret = httpc_muti_get(argv[0], argv[1]);
	if (ret != 0) {
		return CMD_STATUS_FAIL;
	} else {
		return CMD_STATUS_OK;
	}
}

static enum cmd_status cmd_httpc_multi_post(char *cmd)
{
	return CMD_STATUS_OK;
}

static const struct cmd_data g_httpc_cmds[] = {
	{ "get",        cmd_httpc_get },
	{ "post",       cmd_httpc_post },
	{ "-get",       cmd_httpc_get_fresh },
	{ "head",       cmd_httpc_head },
	{ "auth-get",   cmd_httpc_auth_get },
	{ "ssl-get",    cmd_httpc_ssl_get },
	{ "ssl-post",   cmd_httpc_ssl_post },
	{ "multi-get",  cmd_httpc_multi_get },
	{ "multi-post", cmd_httpc_multi_post },
#if USE_KEEP_ALIVE
	{ "kp_get",     cmd_httpc_kp_get },
#endif /* USE_KEEP_ALIVE */
};

void httpc_cmd_task(void *arg)
{
	enum cmd_status status;
	char *cmd = (char *)arg;

	CMD_LOG(1, "<net> <httpc> <request> <cmd : %s>\n", cmd);
	status = cmd_exec(cmd, g_httpc_cmds, cmd_nitems(g_httpc_cmds));
	if (status != CMD_STATUS_OK)
		CMD_LOG(1, "<net> <httpc> <response : fail> <%s>\n", cmd);
	else {
		CMD_LOG(1, "<net> <httpc> <response : success> <%s>\n", cmd);
	}
	XR_OS_ThreadDelete(&cmd_httpc.thread);
}

#if CMD_DESCRIBE
#define httpc_help_info \
"[*] net httpc <cmd> <url> file=<file_test> <user> <password>\n"\
"[*] cmd: {get | -get | post | auth-get | ssl-get | ssl-post}\n"\
"[*]    get: get method\n"\
"[*]    -get: same as get method, but use new interfaces\n"\
"[*]    post: post method\n"\
"[*]    auth-get: get method with login, at this time <user> and <password> are valid, otherwise, this parameter need not be written\n"\
"[*]    ssl-get: get method with ssl\n"\
"[*]    ssl-post: post method with ssl\n"\
"[*] url: server url\n"\
"[*] file_test: only used in post method and represents the file sent to the server\n"\
"[*] user: username, used only in auth-get method\n"\
"[*] password: password, used only in auth-get method"
#endif /* CMD_DESCRIBE */

static enum cmd_status cmd_httpc_help_exec(char *cmd)
{
#if CMD_DESCRIBE
	CMD_LOG(1, "%s\n", httpc_help_info);
#endif
	return CMD_STATUS_ACKED;
}

enum cmd_status cmd_httpc_exec(char *cmd)
{
	if (cmd_strcmp(cmd, "help") == 0) {
		cmd_write_respond(CMD_STATUS_OK, "OK");
		return cmd_httpc_help_exec(cmd);
	}

	if (XR_OS_ThreadIsValid(&cmd_httpc.thread)) {
		CMD_ERR("httpc task is running\n");
		return CMD_STATUS_FAIL;
	}

	memset(cmd_httpc.arg_buf, 0, sizeof(cmd_httpc.arg_buf));
	cmd_strlcpy(cmd_httpc.arg_buf, cmd, sizeof(cmd_httpc.arg_buf));

	if (XR_OS_ThreadCreate(&cmd_httpc.thread,
	                    "httpc",
	                    httpc_cmd_task,
	                    (void *)cmd_httpc.arg_buf,
	                    XR_OS_THREAD_PRIO_APP,
	                    HTTPC_THREAD_STACK_SIZE) != XR_OS_OK) {
		CMD_ERR("httpc task create failed\n");
		return CMD_STATUS_FAIL;
	}
	return CMD_STATUS_OK;
}

static void httpc_exec(int argc, char *argv[])
{
	cmd2_main_exec(argc, argv, cmd_httpc_exec);
}

FINSH_FUNCTION_EXPORT_CMD(httpc_exec, httpc, httpc testcmd);
