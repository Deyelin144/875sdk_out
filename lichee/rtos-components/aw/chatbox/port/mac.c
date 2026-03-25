#include "mac.h"
#include "sysinfo/sysinfo.h"
#include "log.h"
#include <time.h>

static char *g_uuid;

static void fill_random_bytes(int8_t *buf, size_t len)
{
	srand((unsigned int)time(NULL));

	for (size_t i = 0; i < len; ++i) {
		buf[i] = (unsigned char)rand();
	}
}

static char *generate_uuid(void)
{
#if 0
	int8_t uuid[16];

	fill_random_bytes(uuid, sizeof(uuid));

	uuid[6] = (uuid[6] & 0x0F) | 0x40;    // 版本 4
	uuid[8] = (uuid[8] & 0x3F) | 0x80;    // 变体 1

	static char uuid_str[37] = {0};
	snprintf(uuid_str, sizeof(uuid_str),
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			uuid[0], uuid[1], uuid[2], uuid[3],
			uuid[4], uuid[5], uuid[6], uuid[7],
			uuid[8], uuid[9], uuid[10], uuid[11],
			uuid[12], uuid[13], uuid[14], uuid[15]);

	CHATBOX_INFO("generate_uuid: %s\n", uuid_str);

	return uuid_str;
#endif
	return UUID;
}

char *get_uuid(void)
{
	if (g_uuid == NULL) {
		g_uuid = generate_uuid();
	}

	return g_uuid;
}

int get_dev_mac(char *buf, int buf_len)
{
	struct sysinfo *sysinfo;

	sysinfo = sysinfo_get();

	sysinfo = NULL;

	if (sysinfo) {
		uint8_t mac_hex[6];
		memcpy(mac_hex, sysinfo->mac_addr, 6);

		snprintf(buf, buf_len, "%02x:%02x:%02x:%02x:%02x:%02x",
				mac_hex[0], mac_hex[1], mac_hex[2],
				mac_hex[3], mac_hex[4], mac_hex[5]);

		CHATBOX_INFO("get mac_addr: %s\n", buf);
	} else {
		memcpy(buf, MAC, strlen(MAC));
		CHATBOX_INFO("get mac_addr from sysinfo failed. use default mac: %s\n", MAC);
	}

	return 0;
}

