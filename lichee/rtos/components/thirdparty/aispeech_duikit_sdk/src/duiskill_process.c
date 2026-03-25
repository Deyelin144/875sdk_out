#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "cJSON.h"
#include "AudioSystem.h"

#ifdef CONFIG_DISP2_SUNXI
#include "dev_disp.h"
#else
#include "disp_display.h"
#endif

#define DUIKIT_LOG_TAG "DuiSkillProcess"
#define DUIKIT_LOG_LVL DUIKIT_LOG_LEVEL_DEBUG
#include "duikit_log.h"
#include "duikit_string.h"

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
#include "media_manager.h"
#endif

#ifdef CONFIG_ADS_COMPONENT_NETWORK_MANAGER
#include "network_manager.h"
#endif

#include "dds.h"

#include "duikit_sdk.h"
#include "duiskill_process.h"

#define CENTRAL_CONTROL_TASK    "中控"
#define PLAY_CONTROL_TASK       "播放控制"
#define Doubao_Chat_TASK       "外接豆包大模型"

#define CENTRAL_CONTROL_SET_VOLUME          "DUI.System.Display.SetVolume"      //设置音量
#define CENTRAL_CONTROL_SET_BRIGHTNESS      "DUI.System.Display.SetBrightness"  //设置亮度
#define CENTRAL_CONTROL_SOUNDS_OPEN_MODE    "DUI.System.Sounds.OpenMode"        //打开静音模式/打开勿扰模式
#define CENTRAL_CONTROL_SOUNDS_CLOSE_MODE   "DUI.System.Sounds.CloseMode"       //关闭静音模式/关闭勿扰模式
#define CENTRAL_CONTROL_GO_BACK             "DUI.System.GoBack"                 //返回
#define CENTRAL_CONTROL_GO_HOME             "DUI.System.GoHome"                 //返回主页
#define CENTRAL_CONTROL_OPEN_SETTINGS       "DUI.System.OpenSettings"           //打开界面(设置/网络设置)

#define MEDIA_CONTROLLER_PLAY               "DUI.MediaController.Play"          //播放
#define MEDIA_CONTROLLER_PAUSE              "DUI.MediaController.Pause"         //暂停播放
#define MEDIA_CONTROLLER_STOP               "DUI.MediaController.Stop"          //停止播放
#define MEDIA_CONTROLLER_REPLAY             "DUI.MediaController.Replay"        //重新播放
#define MEDIA_CONTROLLER_PREV               "DUI.MediaController.Prev"          //上一个
#define MEDIA_CONTROLLER_NEXT               "DUI.MediaController.Next"          //下一个
#define MEDIA_CONTROLLER_SWITCH             "DUI.MediaController.Switch"        //切换
#define MEDIA_CONTROLLER_SET_PLAY_MODE      "DUI.MediaController.SetPlayMode"   //设置播放模式
#define MEDIA_CONTROLLER_FORWARD            "DUI.MediaController.Forward"       //快进
#define MEDIA_CONTROLLER_BACKWARD           "DUI.MediaController.Backward"      //快退

#define CENTRAL_CONTROL_BRIGHTNESS_STEP     (25)
#define CENTRAL_CONTROL_BRIGHTNESS_MAX      (255)
#define CENTRAL_CONTROL_BRIGHTNESS_MIN      (1)

#define MEDIA_PLAY_MODE_ORDER_PLAY          "orderPlay"                         //顺序播放
#define MEDIA_PLAY_MODE_RANDOM_PLAY         "randomPlay"                        //随机播放
#define MEDIA_PLAY_MODE_SINGLE_LOOP         "singleLoop"                        //单曲循环
#define MEDIA_PLAY_MODE_LOOP_PLAY           "loopPlay"                          //列表循环
#define MEDIA_PLAY_MODE_CYCLE_PLAY          "cyclePlay"                         //循环播放

#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
extern void lv_demo_panel_wakeup_dialog_asr_result(void);
extern void lv_demo_panel_wakeup_dialog_dm_end(void);
extern void lv_demo_panel_music_newlist(void);
extern void lv_demo_panel_music_play_single_cycle(void);
extern void lv_demo_panel_music_play_order_cycle(void);
extern void lv_demo_panel_music_play_random(void);
extern void lv_demo_panel_music_play_forward(void);
extern void lv_demo_panel_music_play_backward(void);
extern void lv_demo_panel_music_play_stopped(void);
extern void lv_demo_panel_wakeup_dialog_back_home(void);
extern void lv_demo_panel_wakeup_dialog_settings(void);
extern void lv_demo_panel_statusbar_set_vol(int value);
extern void lv_demo_panel_statusbar_set_brightness(int value);
#endif

#ifdef CONFIG_LVGL8_86_BOXES
extern int lv_box_aimode_add_message(int position, const char *sender_name, const char *message_content);
#endif

struct dui_skill_handle {
    char *action;
    int (*handle)(cJSON *args);
};

static int g_original_vol = 0;  //记录静音前的音量

//音量设置处理
static void set_volume_process(cJSON *param)
{
    cJSON *temp = cJSON_GetObjectItem(param, "volume");
    if (temp) {
        int volume;
        if (0 == strncmp(temp->valuestring, "+", 1)) {
            volume = get_device_volume();
            if (strlen(temp->valuestring) == 1) {
                volume += DEFAULT_VOLUME_STEP; //+
            } else {
                volume += atoi(temp->valuestring + 1); //+5
            }
        } else if (0 == strncmp(temp->valuestring, "-", 1)) {
            volume = get_device_volume();
            if (strlen(temp->valuestring) == 1) {
                volume -= DEFAULT_VOLUME_STEP; //-
            } else {
                volume -= atoi(temp->valuestring + 1); //-5
            }
        } else if (0 == strcmp(temp->valuestring, "max")) {
            volume = DEFAULT_VOLUME_MAX;
        } else if (0 == strcmp(temp->valuestring, "min")) {
            volume = DEFAULT_VOLUME_MIN;
        } else {
            volume = atoi(temp->valuestring); //5 / 5%
        }
        if (volume < DEFAULT_VOLUME_MIN) {
            volume = DEFAULT_VOLUME_MIN;
        } else if (volume > DEFAULT_VOLUME_MAX) {
            volume = DEFAULT_VOLUME_MAX;
        }
        if (set_device_volume(volume) != 0) {
            DUIKIT_LOG_E("failed to set volume: %d", volume);
        }
    } else {
        DUIKIT_LOG_E("failed to get volume\n");
    }
}

//亮度设置处理
static void set_brightness_process(cJSON *param)
{
    cJSON *temp = cJSON_GetObjectItem(param, "brightness");
#ifdef CONFIG_DISP2_SUNXI
    struct disp_manager *mgr = NULL;
    mgr = g_disp_drv.mgr[0];
    int brightness = mgr->device->get_bright(mgr->device);
#else
    int brightness = CENTRAL_CONTROL_BRIGHTNESS_MAX - bsp_disp_lcd_get_bright(0);
#endif
    if (0 == strncmp(temp->valuestring, "+", 1)) {
        if (strlen(temp->valuestring) == 1) {
            brightness += CENTRAL_CONTROL_BRIGHTNESS_STEP; //+
        } else {
            brightness += atoi(temp->valuestring + 1); //+5
        }
    } else if (0 == strncmp(temp->valuestring, "-", 1)) {
        if (strlen(temp->valuestring) == 1) {
            brightness -= CENTRAL_CONTROL_BRIGHTNESS_STEP; //-
        } else {
            brightness -= atoi(temp->valuestring + 1); //-5
        }
    } else if (0 == strcmp(temp->valuestring, "max")) {
        brightness = CENTRAL_CONTROL_BRIGHTNESS_MAX;
    } else if (0 == strcmp(temp->valuestring, "min")) {
        brightness = CENTRAL_CONTROL_BRIGHTNESS_MIN;
    } else {
        brightness = atoi(temp->valuestring); //5 / 5%
    }
    if (brightness < CENTRAL_CONTROL_BRIGHTNESS_MIN) {
        brightness = CENTRAL_CONTROL_BRIGHTNESS_MIN;
    } else if (brightness > CENTRAL_CONTROL_BRIGHTNESS_MAX) {
        brightness = CENTRAL_CONTROL_BRIGHTNESS_MAX;
    }
#ifdef CONFIG_DISP2_SUNXI
    int ret = mgr->device->set_bright(mgr->device, brightness);
#else
    int ret = bsp_disp_lcd_set_bright(0, CENTRAL_CONTROL_BRIGHTNESS_MAX - brightness);
#endif
    if (ret == 0) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_statusbar_set_brightness(brightness);
#endif
    } else {
        DUIKIT_LOG_E("failed to set brightness: %d", brightness);
    }
}

static void set_mute_process(void)
{
    int value = get_device_volume();
    if (set_device_volume(0) == 0) {
        g_original_vol = value;
    } else {
        DUIKIT_LOG_E("failed to set mute");
    }
}

static void set_unmute_process(void)
{
    if (get_device_volume() == 0) {
        if (g_original_vol == 0)
            g_original_vol = 50;

        if (set_device_volume(g_original_vol) != 0) {
            DUIKIT_LOG_E("failed to set unmute");
        }
    } else {
        DUIKIT_LOG_W("mute mode is not open, do nothing!");
    }
}

static int central_control_skill(cJSON *dm)
{
    cJSON *command = cJSON_GetObjectItem(dm, "command");
    cJSON *api = cJSON_GetObjectItem(command, "api");

    if (api == NULL) {
        DUIKIT_LOG_E("failed to get api\n");
        return -1;
    }

    if (0 == strcmp(api->valuestring, CENTRAL_CONTROL_SET_VOLUME)) {
        set_volume_process(cJSON_GetObjectItem(dm, "param"));
    } else if (0 == strcmp(api->valuestring, CENTRAL_CONTROL_SET_BRIGHTNESS)) {
        set_brightness_process(cJSON_GetObjectItem(dm, "param"));
    } else if (0 == strcmp(api->valuestring, CENTRAL_CONTROL_SOUNDS_OPEN_MODE)) {
        cJSON *temp = cJSON_GetObjectItem(cJSON_GetObjectItem(dm, "param"), "mode");
        if (0 == strcmp(temp->valuestring, "静音模式")) {
            set_mute_process();
        } else {
            DUIKIT_LOG_E("unsupported mode: %s", temp->valuestring);
        }
    } else if (0 == strcmp(api->valuestring, CENTRAL_CONTROL_SOUNDS_CLOSE_MODE)) {
        cJSON *temp = cJSON_GetObjectItem(cJSON_GetObjectItem(dm, "param"), "mode");
        if (0 == strcmp(temp->valuestring, "静音模式")) {
            set_unmute_process();
        } else {
            DUIKIT_LOG_E("unsupported mode: %s", temp->valuestring);
        }
    } else if (0 == strcmp(api->valuestring, CENTRAL_CONTROL_GO_BACK)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_wakeup_dialog_back_home();
#endif
    } else if (0 == strcmp(api->valuestring, CENTRAL_CONTROL_GO_HOME)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_wakeup_dialog_back_home();
#endif
    } else if (0 == strcmp(api->valuestring, CENTRAL_CONTROL_OPEN_SETTINGS)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_wakeup_dialog_settings();
#endif
    } else {
        DUIKIT_LOG_E("unsupported api: %s", api->valuestring);
    }
    return 0;
}

static int get_forward_backward_time(cJSON *param)
{
    cJSON *time = cJSON_GetObjectItem(param, "relativetime");
    //00:00:10转化为秒
    if (time) {
        char *p = time->valuestring;
        int hour = 0, min = 0, sec = 0;
        if (p[0] != '\0') {
            hour = atoi(p);
            p = strchr(p, ':');
            if (p) {
                min = atoi(p + 1);
                p = strchr(p + 1, ':');
                if (p) {
                    sec = atoi(p + 1);
                }
            }
        }
        return hour * 3600 + min * 60 + sec;
    } else {
        return 10;
    }
}

static int set_play_mode_process(cJSON *param)
{
    //设置播放模式
    cJSON *mode = cJSON_GetObjectItem(param, "mode");
    if (mode == NULL) {
        DUIKIT_LOG_E("failed to get media control mode\n");
        return -1;
    }
    if (!strcmp(mode->valuestring, MEDIA_PLAY_MODE_SINGLE_LOOP)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        //更新界面
        lv_demo_panel_wakeup_dialog_asr_result();
        lv_demo_panel_music_play_single_cycle();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //设置单曲循环
        media_music_set_play_mode(SINGLE_CYCLE);
#endif
    } else if (!strcmp(mode->valuestring, MEDIA_PLAY_MODE_ORDER_PLAY) ||
                !strcmp(mode->valuestring, MEDIA_PLAY_MODE_CYCLE_PLAY) ||
                !strcmp(mode->valuestring, MEDIA_PLAY_MODE_LOOP_PLAY)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        //更新界面
        lv_demo_panel_wakeup_dialog_asr_result();
        lv_demo_panel_music_play_order_cycle();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //设置顺序播放/列表循环/循环播放
        media_music_set_play_mode(ORDER_CYCLE);
#endif
    } else if (!strcmp(mode->valuestring, MEDIA_PLAY_MODE_RANDOM_PLAY)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        //更新界面
        lv_demo_panel_wakeup_dialog_asr_result();
        lv_demo_panel_music_play_random();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //设置随机播放
        media_music_set_play_mode(RANDOM_PLAY);
#endif
    } else {
        DUIKIT_LOG_W("unsupported mode: %s\n", mode->valuestring);
        return -1;
    }
    return 0;
}

static int play_control_skill(cJSON *dm)
{
    cJSON *command = cJSON_GetObjectItem(dm, "command");
    cJSON *api = cJSON_GetObjectItem(command, "api");

    if (api == NULL) {
        DUIKIT_LOG_E("failed to get media control api\n");
        return -1;
    }

    if (!strcmp(api->valuestring, MEDIA_CONTROLLER_SET_PLAY_MODE)) {
        return set_play_mode_process(cJSON_GetObjectItem(dm, "param"));
    } else if (!strcmp(api->valuestring, MEDIA_CONTROLLER_NEXT) ||
                !strcmp(api->valuestring, MEDIA_CONTROLLER_SWITCH)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_wakeup_dialog_dm_end();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //播放下一首
        media_music_next(true);
#endif
    } else if (!strcmp(api->valuestring, MEDIA_CONTROLLER_PREV)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_wakeup_dialog_dm_end();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //播放上一首
        media_music_previous(true);
#endif
    } else if (!strcmp(api->valuestring, MEDIA_CONTROLLER_PLAY)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_wakeup_dialog_dm_end();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //继续播放
        media_music_resume();
#endif
    }else if (!strcmp(api->valuestring, MEDIA_CONTROLLER_PAUSE)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_wakeup_dialog_dm_end();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //暂停播放
        media_music_pause();
#endif
    }else if (!strcmp(api->valuestring, MEDIA_CONTROLLER_STOP)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_music_play_stopped();
        lv_demo_panel_wakeup_dialog_dm_end();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //停止播放
        media_music_stop();
#endif
    } else if (!strcmp(api->valuestring, MEDIA_CONTROLLER_REPLAY)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_wakeup_dialog_dm_end();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //重新播放
        media_music_select_play(-1);
#endif
    } else if (!strcmp(api->valuestring, MEDIA_CONTROLLER_FORWARD)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_music_play_forward();
        lv_demo_panel_wakeup_dialog_dm_end();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        int time = get_forward_backward_time(cJSON_GetObjectItem(command, "param"));
        media_music_forward(time);
#endif
    } else if (!strcmp(api->valuestring, MEDIA_CONTROLLER_BACKWARD)) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_music_play_backward();
        lv_demo_panel_wakeup_dialog_dm_end();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        int time = get_forward_backward_time(cJSON_GetObjectItem(command, "param"));
        media_music_backward(time);
#endif
        return 0;
    }

    return -1;
}

static int doubao_chat_task(cJSON *dm)
{
    DUIKIT_LOG_D("%s\n", __func__);

    cJSON *languageClass = cJSON_GetObjectItem(dm, "languageClass");
    if (languageClass != NULL) {
        DUIKIT_LOG_D("languageClass: %s\n", languageClass->valuestring);
    }

    cJSON *context = cJSON_GetObjectItem(dm, "context");
    if (context != NULL) {
        cJSON *rec = cJSON_GetObjectItem(context, "rec");
        if (rec != NULL) {
            DUIKIT_LOG_D("rec: %s\n", rec->valuestring);
        }
    }

    cJSON *shouldEndSession = cJSON_GetObjectItem(dm, "shouldEndSession");
    if (shouldEndSession != NULL) {
        DUIKIT_LOG_D("shouldEndSession: %s\n", shouldEndSession->valuestring);
    }

    cJSON *nlg = cJSON_GetObjectItem(dm, "nlg");
    if (nlg != NULL) {
        DUIKIT_LOG_D("nlg: %s\n", nlg->valuestring);
    }

    cJSON *status = cJSON_GetObjectItem(dm, "status");
    if (status != NULL) {
        DUIKIT_LOG_D("status: %d\n", status->valueint);
    }

    cJSON *input = cJSON_GetObjectItem(dm, "input");
    if (input != NULL) {
        DUIKIT_LOG_D("input: %s\n", input->valuestring);
#ifdef CONFIG_LVGL8_86_BOXES
        lv_box_aimode_add_message(1, "USER2", input->valuestring); // aw add
#endif
    }

    cJSON *widget = cJSON_GetObjectItem(dm, "widget");
    if (widget != NULL) {
        cJSON *llmOutputStatus = cJSON_GetObjectItem(widget, "llmOutputStatus");
        if (llmOutputStatus != NULL) {
            DUIKIT_LOG_D("llmOutputStatus: %s\n", llmOutputStatus->valuestring);
        }

        cJSON *llmOutputTips = cJSON_GetObjectItem(widget, "llmOutputTips");
        if (llmOutputTips != NULL) {
            DUIKIT_LOG_D("llmOutputTips: %s\n", llmOutputTips->valuestring);
        }

        cJSON *type = cJSON_GetObjectItem(widget, "type");
        if (type != NULL) {
            DUIKIT_LOG_D("type: %s\n", type->valuestring);
        }

        cJSON *streamType = cJSON_GetObjectItem(widget, "streamType");
        if (streamType != NULL) {
            DUIKIT_LOG_D("streamType: %s\n", streamType->valuestring);
        }

        cJSON *displayText = cJSON_GetObjectItem(widget, "displayText");
        if (displayText != NULL) {
            DUIKIT_LOG_D("displayText: %s\n", displayText->valuestring);
#ifdef CONFIG_LVGL8_86_BOXES
            lv_box_aimode_add_message(0, "USER1", displayText->valuestring); // aw add
#endif
        }
    }

    return 0;  // 返回值可以根据你的需求而更改
}

static struct dui_skill_handle dui_skill_handle[] = {
    {CENTRAL_CONTROL_TASK,    central_control_skill},   // 系统控制(中控)技能处理, 仅处理了Command对话模式，Native未适配
    {PLAY_CONTROL_TASK,       play_control_skill},      // 播放控制技能处理, 仅处理了Command对话模式，Native未适配
    {Doubao_Chat_TASK,       doubao_chat_task}, 
    //自定义技能，请按照技能名称, 自行添加处理逻辑
};

static void meet_exiting_command_handle(void)
{
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_music_play_stopped();
#endif
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        media_music_stop();
#endif
}

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
int media_content_list_handle(cJSON *dm)
{
    cJSON *widget = NULL, *type = NULL;
    cJSON *content = NULL, *item = NULL, *temp = NULL;
    int i = 0, size = 0, len = 0;
    music_info_t *music_list = NULL;

    widget = cJSON_GetObjectItem(dm, "widget");
    type = cJSON_GetObjectItem(widget, "type");

    // 判断是否为media类型技能
    if (type == NULL || 0 != strcmp(type->valuestring, "media")) {
        return -1;
    }

    //获取媒体列表
    content = cJSON_GetObjectItem(widget, "content");
    if (!content) {
        DUIKIT_LOG_E("failed to get media conent list");
        return -1;
    }

    //获取列表长度
    size = cJSON_GetArraySize(content);
    if (size <= 0) {
        DUIKIT_LOG_E("meida conent list is empty");
        return -1;
    }

    music_list = duikit_calloc(size, sizeof(music_info_t));
    for (i = 0; i < size; i++) {
        item = cJSON_GetArrayItem(content, i);

        if (NULL == item)
            continue;

        //解析歌曲名称
        temp = cJSON_GetObjectItem(item, "title");
        if (temp && temp->type == cJSON_String) {
            music_list[len].title = duikit_strdup(temp->valuestring);
        }

        //解析歌手名称
        temp = cJSON_GetObjectItem(item, "subTitle");
        if (temp && temp->type == cJSON_String) {
            music_list[len].artist = duikit_strdup(temp->valuestring);
        }

        //解析播放链接
        temp = cJSON_GetObjectItem(item, "linkUrl");
        if (temp && temp->type == cJSON_String) {
            music_list[len].url = duikit_strdup(temp->valuestring);
        }

        len ++;
    }

    //设置播放列表
    media_music_set_playlist(music_list, len, true);

    //释放列表资源
    for (i = 0; i < size; i++) {
        if (music_list[i].title) duikit_free(music_list[i].title);
        if (music_list[i].artist) duikit_free(music_list[i].artist);
        if (music_list[i].url) duikit_free(music_list[i].url);
    }
    duikit_free(music_list);

    return 0;
}
#endif

int duiskill_process(void *arg)
{
    struct dds_msg *msg = arg;
    int ret = 0, type = 0;
    char *value = NULL;

    ret = dds_msg_get_type(msg, &type);
    if (ret != 0 || type != DDS_EV_OUT_DUI_RESPONSE) {
        return -1;
    }

    ret = dds_msg_get_string(msg, "response", &value);
    if (ret != 0) {
        return -1;
    }

#if 0
    //云端下发的数据可能会很多很频繁，建议此处打印只在开发Debug阶段开启
    printf("response:[%s]\n", value);
#endif

    cJSON *response = cJSON_Parse(value);
    if (NULL == response)
        return -1;

    //全局退出处理
    cJSON *error = cJSON_GetObjectItem(response, "error");
    cJSON *errId = cJSON_GetObjectItem(error, "errId");
    if(errId && !strcmp(errId->valuestring, "010403")) {
        meet_exiting_command_handle();
        goto done;
    }

    cJSON *dm = cJSON_GetObjectItem(response, "dm");
    cJSON *skill = cJSON_GetObjectItem(response, "skill");
    cJSON *inspire = cJSON_GetObjectItem(dm, "inspire");
    cJSON *task = cJSON_GetObjectItem(dm, "task");

    if (inspire && inspire->type == cJSON_Array) {
        //一语多意图结果处理
        int count = cJSON_GetArraySize(inspire);
        for (int j = 0; j < count; ++j) {
            cJSON *item = cJSON_GetArrayItem(inspire, j);
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
            //媒体播放列表处理
            if (media_content_list_handle(item) == 0) {
                //界面更新
                #ifdef CONFIG_LVGL8_USE_DEMO_PANEL
                    lv_demo_panel_music_newlist();
                #endif

                //没有tts时，立即开始播放音乐，有tts时，会在tts播放结束后检查播放列表并播放
                cJSON *speak = cJSON_GetObjectItem(item, "speak");
                if (speak) {
                    if (cJSON_GetObjectItem(speak, "speakUrl") == NULL) {
                        media_music_start();
                    }
                }

                continue;
            }
#endif
            //其他控制列技能/自定义技能，按照任务名称匹配处理
            if (task) {
                for (int i = 0; i < (sizeof(dui_skill_handle) / sizeof(dui_skill_handle[0])); i++) {
                    if (strcmp(task->valuestring, dui_skill_handle[i].action) == 0) {
                        dui_skill_handle[i].handle(item);
                        break;
                    }
                }
            }
        }
    } else {
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        //媒体播放列表处理
        if (media_content_list_handle(dm) == 0) {
            #ifdef CONFIG_LVGL8_USE_DEMO_PANEL
                //界面更新
                lv_demo_panel_music_newlist();
            #endif

            //没有tts时，立即开始播放音乐，有tts时，会在tts播放结束后检查播放列表并播放
            if (cJSON_GetObjectItem(response, "speakUrl") == NULL) {
                media_music_start();
            }

            goto done;
        }
#endif
        //其他控制列技能/自定义技能，按照任务名称匹配处理
        if (task) {
            for (int i = 0; i < (sizeof(dui_skill_handle) / sizeof(dui_skill_handle[0])); i++) {
                if (strcmp(task->valuestring, dui_skill_handle[i].action) == 0) {
                    dui_skill_handle[i].handle(dm);
                    goto done;
                }
            }
        }
        if (skill) {
            for (int i = 0; i < (sizeof(dui_skill_handle) / sizeof(dui_skill_handle[0])); i++) {
                if (strcmp(skill->valuestring, dui_skill_handle[i].action) == 0) {
                    dui_skill_handle[i].handle(dm);
                    goto done;
                }
            }
        }
    }

done:
    cJSON_Delete(response);

    return ret;
}

//设置设备音量
int set_device_volume(int vol)
{
    uint32_t volume_value = 0;

    if (vol < DEFAULT_VOLUME_MIN) {
        vol = DEFAULT_VOLUME_MIN;
    } else if (vol > DEFAULT_VOLUME_MAX) {
        vol = DEFAULT_VOLUME_MAX;
    }

    DUIKIT_LOG_I("set vol %d", vol);
    int ret = softvol_control_with_streamtype(AUDIO_STREAM_MUSIC, &volume_value, 2);
    if (ret != 0) {
        DUIKIT_LOG_E("get softvol range failed:%d", ret);
    }
    int volume_max = (uint16_t)((volume_value >> 16) & 0xffff);
    int volume_min = (uint16_t)(volume_value & 0xffff);
    if ((vol < volume_min) || (vol > volume_max)) {
        DUIKIT_LOG_E("warning:out range : %d ~ %d",volume_min,volume_max);
        return -1;
    }
    volume_value = ((vol << 16) | vol);
    ret = softvol_control_with_streamtype(AUDIO_STREAM_MUSIC, &volume_value, 1);
    if (ret != 0) {
        DUIKIT_LOG_E("error:set softvol failed!");
    }

#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
    lv_demo_panel_statusbar_set_vol(vol);
#endif

    return 0;
}

//获取设备音量
int get_device_volume(void)
{
    int ret = -1;
    int volume_mode = 0;
    uint32_t volume_value = 0;

    ret = softvol_control_with_streamtype(AUDIO_STREAM_MUSIC, &volume_value, volume_mode);
    if (ret != 0) {
        DUIKIT_LOG_E("get softvol failed:%d", ret);
        return -1;
    }
    return ((volume_value & 0xffff) + ((volume_value >> 16) & 0xffff)) / 2;
}
