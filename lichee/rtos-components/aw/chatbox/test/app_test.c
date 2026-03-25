#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "console.h"
#include "stdio.h"
#include "ota.h"
#include "mac.h"

void chatbox_app_start(int argc, char *argv[])
{
	char *active_code;

	if (active_device(&active_code)) {
		printf("active failed,please check network!!!\n");
		return ;
	}

	if (strlen(active_code) != 0) {
		printf("Please log in to https://xiaozhi.me/ to bind your device, active_code: %s\n", active_code);
		printf("Restart after successful binding\n");
		return;
	}

	app_start();

	if (argc == 2)
		app_set_chatmode(atoi(argv[1]));
	else
		app_set_chatmode(CHAT_MODE_REALTIME);

	app_start_listening();
}

void chatbox_app_stop(int argc, char *argv[])
{
	app_stop();
}

void chatbox_storge_enable(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: storge_enable <enable>\n");
		return;
	}

	app_storage_enable(atoi(argv[1]));
}

void chatbox_dev_active_test(int argc, char *argv[])
{
	char *active_code;

	if (active_device(&active_code)) {
		printf("active failed.\n");
	} else {
		if (strlen(active_code) == 0) {
			printf("dev is already actived!\n");
		} else
			printf("active success, active_code: %s\n", active_code);
	}
}

void chatbox_get_dev_mac(int argc, char *argv[])
{
	char mac[18] = {0};
	if (get_dev_mac(mac, 18)) {
		printf("get mac failed.\n");
		return ;
	}

	printf("dev mac: %s\n", mac);
}

void chatbox_get_uuid(int argc, char *argv[])
{
	char *uuid = get_uuid();

	if (uuid == NULL) {
		printf("get uuid failed.\n");
		return ;
	}
	printf("uuid: %s\n", uuid);
}

FINSH_FUNCTION_EXPORT_CMD(chatbox_app_start, chatbox_start, chatbox start);
FINSH_FUNCTION_EXPORT_CMD(chatbox_app_stop, chatbox_stop, chatbox stop);
FINSH_FUNCTION_EXPORT_CMD(chatbox_storge_enable, chatbox_storage, chatbox storage [0/1]);
FINSH_FUNCTION_EXPORT_CMD(chatbox_dev_active_test, chatbox_dev_active_test, chatbox dev active test);
FINSH_FUNCTION_EXPORT_CMD(chatbox_get_dev_mac, chatbox_get_dev_mac, chatbox get dev mac);
FINSH_FUNCTION_EXPORT_CMD(chatbox_get_uuid, chatbox_get_uuid, chatbox get uuid);
