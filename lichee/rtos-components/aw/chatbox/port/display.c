#include "display.h"
#include "log.h"

void display_init(void)
{

}

void display_deinit(void)
{

}

void display_set_status(const char *status)
{
    CHATBOX_INFO("status: %s\n", status);
}

void display_notification(const char *notification, int duration_ms)
{
    CHATBOX_INFO("notification: %s\n", notification);
}

void display_set_emotion(const char *emotion)
{
    CHATBOX_INFO("emotion: %s\n", emotion);
}

void display_set_chat_message(const char *role, const char *content)
{
    CHATBOX_INFO("role: %s, content: %s\n", role, content);
}

void display_set_icon(const char *icon)
{
    CHATBOX_INFO("icon: %s\n", icon);
}
