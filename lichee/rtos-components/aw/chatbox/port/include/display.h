#ifndef __DISPLAY_H__
#define __DISPLAY_H__

void display_init(void);

void display_set_status(const char *status);

void display_notification(const char *notification, int duration_ms);

void display_set_emotion(const char *emotion);

void display_set_chat_message(const char *role, const char *content);

void display_set_icon(const char *icon);

void display_deinit(void);

#endif
