#include "ota.h"
#include "mac.h"
#include "http.h"
#include "cJSON.h"
#include "log.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int ota_request_response(char *url, char *active_code)
{
	int ret = -1;

	char ota_data[256] = {0};

	char *uuid = get_uuid();

	if (uuid == NULL) {
		CHATBOX_ERROR("get uuid failed.\n");
		return -1;
	}
	CHATBOX_INFO("uuid: %s\n", uuid);

	snprintf(ota_data, sizeof(ota_data),\
			"{"
				"\"uuid\": \"%s\","
				"\"application\": {"
					"\"name\": \"xiaozhi_aw\","
					"\"version\": \"1.0.0\""
				"},"
				"\"ota\": {"
					"\"label\": \"ota_0\""
				"},"
				"\"board\": {"
					"\"type\": \"001\","
					"\"name\": \"00a\""
				"}"
			"}",\
			uuid);

	char mac[18] = {0};
	if (get_dev_mac(mac, 18)) {
		CHATBOX_ERROR("get mac failed.\n");
		return -1;
	}

	CHATBOX_INFO("dev mac: %s\n", mac);

	char ext_h[256] = {0};

	snprintf(ext_h, sizeof(ext_h), \
			"Content-Type: application/json\r\n"
			"device-id: %s\r\n"
			"User-Agent: aw\r\n"
			"Accept-Language: zh-CN\r\n",
			mac);

	char *recv_buf = NULL;

	http_ctx_t *ctx = http_open(url);
	if (ctx == NULL) {
		CHATBOX_ERROR("http open failed.\n");
		goto exit0;
	}

	{
		http_post(ctx, url, ext_h, ota_data, strlen(ota_data));

		if (http_recv_response(ctx, &recv_buf))
			goto exit1;

		if (recv_buf == NULL) {
			goto exit1;
		}
	}

	cJSON *json;
	{
		json = cJSON_Parse(recv_buf);
		if (json == NULL) {
			CHATBOX_ERROR("json parse failed.\n");
			goto exit1;
		}
		cJSON *activation = cJSON_GetObjectItem(json, "activation");
		if (activation == NULL) {
			CHATBOX_ERROR("not found activation.\n");
			cJSON *firmware = cJSON_GetObjectItem(json, "firmware");
			if (firmware) {
				CHATBOX_INFO("but find <firmware> string. it means dev is already actived.\n");
				ret = 0;
			} else
				ret = -1;
			goto exit2;
		}
		cJSON *code = cJSON_GetObjectItem(activation, "code");
		if (code == NULL) {
			CHATBOX_ERROR("not found code.\n");
			ret = -1;
			goto exit2;
		}
		CHATBOX_INFO("activation code: %s\n", code->valuestring);
		memcpy(active_code, code->valuestring, strlen(code->valuestring));
		ret = 0;
	}

exit2:
	cJSON_Delete(json);
exit1:
	http_close(ctx);
exit0:
	return ret;
}

int active_device(char **active_code_str)
{
	static char active_code[24] = {0};

	memset(active_code, 0, sizeof(active_code));

	int ret = ota_request_response(OTA_URL, active_code);

	*active_code_str = active_code;

	return ret;
}
