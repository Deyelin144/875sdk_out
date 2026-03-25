#ifndef __LOG_H__
#define __LOG_H__

#if __cplusplus
extern "C" {
#endif
#include <stdio.h>
#define CHATBOX_LOG_TAG "CH"

#define CHATBOX_INFO(fmt, ...)  printf("[INFO] %s,%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CHATBOX_ERROR(fmt, ...) printf("[ERROR] %s,%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CHATBOX_DEBUG(fmt, ...) printf("[DEBUG] %s,%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CHATBOX_WARNG(fmt, ...) printf("[WARNG] %s,%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CHATBOX_DUMP(fmt, ...)

#if __cplusplus
}; // extern "C"
#endif

#endif
