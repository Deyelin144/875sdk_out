#include<string.h>
#include "cJSON.h"

#define DUIKIT_LOG_TAG "TemplateProject"
#define DUIKIT_LOG_LVL DUIKIT_LOG_LEVEL_DEBUG
#include "duikit_log.h"
#include "duikit_time.h"

#ifdef CONFIG_ADS_COMPONENT_COMMON_MEMERY_MONITOR
#include "as_mem_monitor.h"
#endif

#include "dsp_adapter.h"
#include "dsp_manager.h"

#ifdef CONFIG_ADS_COMPONENT_NETWORK_MANAGER
#include "wifi_adapter.h"
#endif

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
#include "music_player.h"
#include "tts_player.h"
#endif

#include "duikit_sdk.h"
#include "duiskill_process.h"
#include "duikit_task_config.h"

//默认配置文件路径
#ifndef DEFAULT_DUI_CONFIG_PATH
#define DEFAULT_DUI_CONFIG_PATH ("/data/dui.conf.json")
#endif

//默认授权文件保存路径
#ifndef DEFAULT_SAVED_PROFILE_PATH
#define DEFAULT_SAVED_PROFILE_PATH ("/data/savedProfile")
#endif

//默认唤醒提示音文件路径
#ifndef DEFAULT_WAKEUP_PROMPT_PATH
#define DEFAULT_WAKEUP_PROMPT_PATH ("/data/prompt/wakeup1.mp3")
#endif

//默认VAD Timeout提示音文件路径
#ifndef DEFAULT_VAD_TIMEOUT_PROMPT_PATH
#define DEFAULT_VAD_TIMEOUT_PROMPT_PATH ("/data/prompt/vad_timeout.mp3")
#endif

//默认进入多轮提示音文件路径
#ifndef DEFAULT_MULTIPLE_DIALOG_PROMPT_PATH
#define DEFAULT_MULTIPLE_DIALOG_PROMPT_PATH ("/data/prompt/dialog_multiwheel_ack.mp3")
#endif

#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
extern void lv_demo_panel_wakeup_dialog_wakeup(void);
extern void lv_demo_panel_wakeup_dialog_asr_result(void);
extern void lv_demo_panel_wakeup_dialog_asr_exec(char *text);
extern void lv_demo_panel_wakeup_dialog_dm_end(void);
extern void lv_demo_panel_music_play_paused(void);
extern void lv_demo_panel_music_play_started(void);
extern void lv_demo_panel_music_play_stopped(void);
extern void lv_demo_panel_music_playlist_changed(void);
extern void lv_demo_panel_music_play_error(void);
#endif

/**
 * 保存授权Profile
 */
static int save_auth_profile(char *profile)
{
    FILE *fd = NULL;

    fd = fopen(DEFAULT_SAVED_PROFILE_PATH, "w+");
    if (fd == NULL) {
        DUIKIT_LOG_E("failed to open %s", DEFAULT_SAVED_PROFILE_PATH);
        return -1;
    }

    fwrite(profile, sizeof(char), strlen(profile), fd);

    fclose(fd);

    return 0;
}

/**
 * 文件读取函数
 */
static int read_file_to_buf(const char *name, char **content)
{
    int ret = -1, filelen = 0;
    FILE *fd = NULL;

    fd = fopen(name, "r");
    if (fd == NULL) {
        ret = -1;
        goto FAILED;
    }

    ret = fseek(fd, 0, SEEK_END);
    if (ret) {
        ret = -4;
        goto FAILED;
    }

    filelen = ftell(fd) + 1;
    *content = (char *)duikit_calloc(1, filelen);
    if (*content == NULL) {
        ret = -3;
        goto FAILED;
    }

    fseek(fd, 0, SEEK_SET);
    ret = fread(*content, 1, filelen, fd);
    if (ret != filelen - 1) {
        ret = -2;
        goto FAILED;
    }

    fclose(fd);

    return 0;

FAILED:
    if (*content) {
        duikit_free(*content);
        *content = NULL;
    }
    if (fd)
        fclose(fd);
    return ret;
}

/**
 * 配置初始化
 */
static int auth_config_init(char **auth_config, bool *use_local_vad)
{
    char *config = NULL;
    char *encryptProfile = NULL;
    cJSON *root = NULL, *temp = NULL;
    cJSON *auth = NULL, *auth_info = NULL;
    cJSON *device_info = NULL, *device_name = NULL;
    cJSON *local_vad = NULL;
    int ret, auth_select = -1, size = -1;

    read_file_to_buf(DEFAULT_DUI_CONFIG_PATH, &config);

    root = cJSON_Parse(config);
    if (NULL == root) {
        DUIKIT_LOG_E("cannot parse config, json format error");
        return -1;
    }

    do {
        temp = cJSON_GetObjectItem(root, "description");
        if (temp)
            DUIKIT_LOG_I("config description %s", temp->valuestring);

        //授权相关配置解析
        auth = cJSON_GetObjectItem(root, "auth");
        if (auth) {
            temp = cJSON_GetObjectItem(auth, "authSelect");
            auth_select = temp && temp->type == cJSON_Number ? temp->valueint : 0;
            auth_info = cJSON_GetObjectItem(auth, "authInfo");
            if (auth_info && auth_info->type == cJSON_Array) {
                size = cJSON_GetArraySize(auth_info);
                if (size > auth_select) {
                    temp = cJSON_GetArrayItem(auth_info, auth_select);
                    if (temp) {
                        //本地保存已有授权文件时直接使用
                        read_file_to_buf(DEFAULT_SAVED_PROFILE_PATH, &encryptProfile);
                        if (encryptProfile) {
                            cJSON_AddStringToObject(temp, "encryptProfile", encryptProfile);
                            duikit_free(encryptProfile);
                        }

                        //更新DeviceName，用于DDS授权的唯一标识
                        device_info = cJSON_GetObjectItem(temp, "devinfo");
                        if (device_info) {
                            //R128平台DSP固件固定使用ChipID校验授权，因此必须使用ChipID作为设备ID
                            char *device_id = dsp_manager_device_id_get();
                            if (device_id) {
                                device_name = cJSON_CreateString(device_id);
                                cJSON_ReplaceItemInObject(device_info, "deviceName", device_name);
                                free(device_id);
                            }
                        }

                        *auth_config = cJSON_PrintUnformatted(temp);
                        DUIKIT_LOG_I("auth config %s", *auth_config);
                    }
                } else {
                    DUIKIT_LOG_E("invalid auth_select: %d", auth_select);
                    ret = -1;
                    break;
                }
            } else {
                DUIKIT_LOG_E("invalid autho info");
                ret = -1;
                break;
            }
        } else {
            DUIKIT_LOG_E("invalid auth config");
            ret = -1;
            break;
        }

        //根据配置决定是否使用本地VAD
        local_vad = cJSON_GetObjectItem(root, "use_local_vad");
        if (local_vad && (local_vad->type == cJSON_True)) {
            *use_local_vad = true;
        } else {
            *use_local_vad = false;
        }
        DUIKIT_LOG_I("default use %s vad", *use_local_vad ? "local" : "cloud");

        ret = 0;
    } while (0);

    cJSON_Delete(root);

    if (config)
        duikit_free(config);

    return ret;
}

/**
 * 对话处理事件回调
 */
static bool _online_event_callback(duikit_dp_event_data_t *ev_data)
{
    switch (ev_data->event) {
        case DUIKIT_DP_EV_OUT_AUTH_SUCCESS: {
            //授权成功，保存encryptProfile
            char *value = NULL;
            if (!dds_msg_get_string(ev_data->msg, "encryptProfile", &value)) {
                save_auth_profile(value);
            }
            break;
        }
        case DUIKIT_DP_EV_OUT_PLAY_WAKEUP_PROMPT:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
            //显示唤醒对话框
            lv_demo_panel_wakeup_dialog_wakeup();
#endif

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
            //播放唤醒提示音
            media_tts_play(DEFAULT_WAKEUP_PROMPT_PATH, false);
#else
            //如果不需要播放唤醒提示音，可以直接通知SDK提示音播报结束，让SDK进入对话监听
            {
                duikit_dp_event_t evt_data = {0};
                evt_data.type = DUIKIT_DP_EV_IN_WAKEUP_PROMPT_PLAY_FIN;
                duikit_dialog_process_send(&evt_data);
            }
#endif
            break;
        case DUIKIT_DP_EV_OUT_ONESHOT_WAKEUP:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
            //显示唤醒对话框
            lv_demo_panel_wakeup_dialog_wakeup();
#endif

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
            //暂停音乐播放
            media_music_pause2();
#endif
            break;

        case DUIKIT_DP_EV_OUT_PLAY_TTS: {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
                lv_demo_panel_wakeup_dialog_asr_result();
#endif
            break;
        }

        case DUIKIT_DP_EV_OUT_PLAY_MULTIPLE_DIALOG_PROMPT:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
            //对话框继续显示为唤醒监听状态
            lv_demo_panel_wakeup_dialog_wakeup();
#endif

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
            //播放多轮对话提示音
            media_tts_play(DEFAULT_MULTIPLE_DIALOG_PROMPT_PATH, false);
#else
            //如果不需要播放多轮提示音，可以直接通知SDK提示音播报结束，让SDK进入对话监听
            {
                duikit_dp_event_t evt_data = {0};
                evt_data.type = DUIKIT_DP_EV_IN_MULTIPLE_DIALOG_PROMPT_PLAY_FIN;
                duikit_dialog_process_send(&evt_data);
            }
#endif
            break;

        case DUIKIT_DP_EV_OUT_DUI_RESPONSE:
            //DUI技能处理
            duiskill_process(ev_data->msg);
            break;

        case DUIKIT_DP_EV_OUT_ERROR: {
            int error_id = -1;
            char *value = NULL;
            if (!dds_msg_get_integer(ev_data->msg, "errorId", &error_id)) {
                DUIKIT_LOG_E("errorId: %d\n", error_id);
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
                if ((error_id == DDS_ERROR_TIMEOUT) ||
                        (error_id == DDS_ERROR_NETWORK) ||
                        (error_id == DDS_ERROR_SERVER)) {
                    lv_demo_panel_wakeup_dialog_dm_end();
                }
#endif
            }
            break;
        }

        case DUIKIT_DP_EV_OUT_VAD_BEGAIN_TIMEOUT:
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
            //唤醒后未监听到用户说话，VAD检测，播放VAD超时提示音
            media_tts_play(DEFAULT_VAD_TIMEOUT_PROMPT_PATH, true);
#endif
            break;

        case DUIKIT_DP_EV_OUT_DIALOG_TIMEOUT:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
            lv_demo_panel_wakeup_dialog_dm_end();
#endif
            break;

        default:
            break;
    }

    return false;
}

static void _offline_event_process(duikit_op_event_data_t *ev_data)
{
    switch (ev_data->event) {
        case DUIKIT_OP_EV_OUT_PLAY_WAKEUP_PROMPT:
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
            media_tts_play(DEFAULT_WAKEUP_PROMPT_PATH, false);
#endif
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
            lv_demo_panel_wakeup_dialog_wakeup();
#endif
            break;

        case DUIKIT_OP_EV_OUT_ASR_RESULT:
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
            media_tts_play(DEFAULT_ACK_PROMPT_PATH, false);
#endif
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
            lv_demo_panel_wakeup_dialog_asr_exec((char *) ev_data->data);
#endif
            break;

        case DUIKIT_OP_EV_OUT_ASR_TIMEOUT:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
            lv_demo_panel_wakeup_dialog_dm_end();
#endif
            break;

        default:
            break;
    }
}

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
/**
 * 媒体播放器事件回调
 */
static void _media_event_process(media_event_data_t *event_data)
{
    DUIKIT_LOG_D("event: %d, url:%s", event_data->event, event_data->url);

    switch (event_data->event) {
    case MEDIA_EVENT_ERROR:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        if (event_data->type == MEDIA_MUSIC) {
            lv_demo_panel_music_play_error();
        } else {
            lv_demo_panel_wakeup_dialog_dm_end();
        }
#endif
        break;
    case MEDIA_EVENT_STOPPED:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        if (event_data->type == MEDIA_MUSIC) {
            lv_demo_panel_music_play_stopped();
        }
#endif
        break;

    case MEDIA_EVENT_PAUSED:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        if (event_data->type == MEDIA_MUSIC) {
            lv_demo_panel_music_play_paused();
        }
#endif
        break;

    case MEDIA_EVENT_STARTED:
        if (event_data->type == MEDIA_MUSIC) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
            lv_demo_panel_music_play_started();
#endif
        } else {
            if (duikit_sdk_is_online()) {
                if (strcmp(event_data->url, DEFAULT_WAKEUP_PROMPT_PATH) != 0 &&
                    strcmp(event_data->url, DEFAULT_MULTIPLE_DIALOG_PROMPT_PATH) != 0) {
                    duikit_dp_event_t evt_data = {0};
                    evt_data.type = DUIKIT_DP_EV_IN_TTS_PLAY_START;
                    duikit_dialog_process_send(&evt_data);
                }
            }
        }
        break;
    case MEDIA_EVENT_RESUMED:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        if (event_data->type == MEDIA_MUSIC) {
            lv_demo_panel_music_play_started();
        }
#endif
        break;

    case MEDIA_EVENT_COMPLETED:
        if (event_data->type == MEDIA_TTS) {
#ifdef CONFIG_ADS_COMPONENT_NETWORK_MANAGER
            if (!duikit_sdk_is_online()) {
                //离线状态TTS播报结束
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
                if (strcmp(event_data->url, DEFAULT_ACK_PROMPT_PATH) == 0) {
                    lv_demo_panel_wakeup_dialog_wakeup();
                } else if (strcmp(event_data->url, DEFAULT_WAKEUP_PROMPT_PATH) != 0) {
                    lv_demo_panel_wakeup_dialog_dm_end();
                }
#endif
            } else
#endif
            {
                //在线状态TTS播报结束
                if (strcmp(event_data->url, DEFAULT_WAKEUP_PROMPT_PATH) == 0) {
                    duikit_dp_event_t evt_data = {0};
                    evt_data.type = DUIKIT_DP_EV_IN_WAKEUP_PROMPT_PLAY_FIN;
                    duikit_dialog_process_send(&evt_data);
                } else if (strcmp(event_data->url, DEFAULT_MULTIPLE_DIALOG_PROMPT_PATH) == 0) {
                    duikit_dp_event_t evt_data = {0};
                    evt_data.type = DUIKIT_DP_EV_IN_MULTIPLE_DIALOG_PROMPT_PLAY_FIN;
                    duikit_dialog_process_send(&evt_data);
                } else if (strcmp(event_data->url, DEFAULT_ACK_PROMPT_PATH) == 0) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
                        lv_demo_panel_wakeup_dialog_dm_end();
#endif
                } else {
                    if (duikit_dialog_should_end_session()) {
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
                        lv_demo_panel_wakeup_dialog_dm_end();
#endif
                    } else {
                        duikit_dp_event_t evt_data = {0};
                        evt_data.type = DUIKIT_DP_EV_IN_TTS_PLAY_FIN;
                        duikit_dialog_process_send(&evt_data);
                    }
                }
            }
        }
        break;
    case MEDIA_EVENT_PLAYLIST_CHANGED:
#ifdef CONFIG_LVGL8_USE_DEMO_PANEL
        lv_demo_panel_music_playlist_changed();
#endif
        break;
    }
}
#endif

#ifdef CONFIG_ADS_COMPONENT_NETWORK_MANAGER
/**
 * 网络事件回调
 */
static void _network_event_process(network_event_data_t *data)
{
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
    switch (data->event) {
        case NETWORK_EVENT_CONNECTED:
            //media_tts_play("/data/prompt/network_connected.mp3", true);
            break;
        case NETWORK_EVENT_DISCONNECTED:
            //media_tts_play("/data/prompt/network_disconnected.mp3", true);
            break;
    }
#endif
}
#endif

/**
 * DSP事件回调
 */
static void _dsp_event_process(duikit_dsp_event_data_t *ev_data)
{
    DUIKIT_LOG_D("DSP event: %s", ev_data->data);
    if (ev_data->event == DUIKIT_DSP_EVENT_WAKEUP_COMMAND) {
        //快捷唤醒词处理
#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
        if (NULL != strstr(ev_data->data, "zan ting bo fang")) {
            //暂停播放
            media_music_pause();
        } else if (NULL != strstr(ev_data->data, "ji xv bo fang")) {
            //继续播放
            media_music_continue(); 
        } else
#endif
        if (NULL != strstr(ev_data->data, "zeng da yin liang")) {
            //增大音量
            int volume = get_device_volume();
            volume += DEFAULT_VOLUME_STEP;
            set_device_volume(volume);
        } else if (NULL != strstr(ev_data->data, "jiang di yin liang")) {
            //降低音量
            int volume = get_device_volume();
            volume -= DEFAULT_VOLUME_STEP;
            set_device_volume(volume);
        }
    }
}

static void _duikit_event_callback(int event, void *data)
{
    switch (event) {
    case DUIKIT_SDK_EVENT_DIALOG_ONLINE:
        _online_event_callback((duikit_dp_event_data_t *) data);
        break;

    case DUIKIT_SDK_EVENT_DIALOG_OFFLINE:
        _offline_event_process((duikit_op_event_data_t *) data);
        break;

#ifdef CONFIG_ADS_COMPONENT_NETWORK_MANAGER
    case DUIKIT_SDK_EVENT_NETWORK:
        _network_event_process((network_event_data_t *) data);
        break;
#endif

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
    case DUIKIT_SDK_EVENT_MEDIA:
        _media_event_process((media_event_data_t *) data);
        break;
#endif

    case DUIKIT_SDK_EVENT_DSP:
        _dsp_event_process((duikit_dsp_event_data_t *) data);
        break;

    default:
        break;
    }
}

#ifdef CONFIG_ADS_COMPONENT_COMMON_MEMERY_MONITOR
void *duikit_cjson_malloc(size_t size)
{
    return duikit_malloc(size);
}

void duikit_cjson_free(void *ptr)
{
    duikit_free(ptr);
}
#endif

void duikit_sdk_demo(void)
{
#ifndef CONFIG_LVGL8_USE_DEMO_PANEL
    //不带UI的版本延时1s启动，防止dsp创建rpdata太慢导致无法和dsp_manager连接
    //TODO: 应该优化R128平台dsp_manager及dsp创建rpdata逻辑，保证无论哪边先启动都可以正常连接通讯
    duikit_delayms(1000);
#endif

#ifdef CONFIG_ADS_COMPONENT_COMMON_MEMERY_MONITOR
    //初始化内存管理模块
    as_mem_monitor_init(NULL);

    //开启内存管理模块时，CJSON必须使用duikit_malloc/duikit_free
    //因为malloc申请的内存无法使用duikit_free释放
    struct cJSON_Hooks js_hook = {duikit_cjson_malloc, duikit_cjson_free};
    cJSON_InitHooks(&js_hook);
#endif

    //初始化日志
    duikit_log_config_t log_cfg = {0};
    log_cfg.priority = DUIKIT_LOG_TASK_PRIORITY;
    log_cfg.stack_size = DUIKIT_LOG_TASK_STACKSIZE;
    log_cfg.core_id = DUIKIT_LOG_TASK_COREID;
    log_cfg.ext_stack = DUIKIT_LOG_TASK_EXT_STACK;
    duikit_log_init(&log_cfg);
    // duikit_log_set_output(false);

    //初始化配置
    duikit_sdk_config_t config = {0};

    char *auth_config = NULL;
    if (0 != auth_config_init(&auth_config, &config.use_local_vad)) {
        DUIKIT_LOG_E("dui config invalid!!!");
        return;
    }

    config.auth_config = auth_config;
    config.callback = _duikit_event_callback;

    config.dsp_adapter = &dsp_adapter;

#ifdef CONFIG_ADS_COMPONENT_NETWORK_MANAGER
    config.wifi_adapter = &wifi_adapter;
#endif

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
    config.music_player = &music_player;
    config.tts_player = &tts_player;
#endif

    //初始化duikit sdk
    duikit_sdk_init(&config);

    DUIKIT_LOG_D("========dui sdk version: %s========", DUI_SDK_VERSION);

    if (auth_config) {
        duikit_free(auth_config);
    }

    //TODO: 其他业务逻辑初始化
}

//===============================================================

#ifndef CONFIG_KERNEL_FREERTOS
#define CONFIG_KERNEL_FREERTOS
#endif
#include <console.h>
#include <getopt.h>

static void duikit_sdk_usage(void)
{
    DUIKIT_LOG_I("Usage: duikit_sdk [option]");
    DUIKIT_LOG_I("-h,    duikit sdk help");
    DUIKIT_LOG_I("-I,    duikit sdk init");
    DUIKIT_LOG_I("-D,    duikit sdk deinit");
    DUIKIT_LOG_I("-m,    duikit sdk set mic");
    DUIKIT_LOG_I("-v,    duikit sdk set vad, 1: use local vad, other: use cloud vad");
    DUIKIT_LOG_I("-o,    duikit sdk set oneshot, 1: enable oneshot, other: disable oneshot");
    DUIKIT_LOG_I("-s,    duikit sdk set online status");
#ifdef CONFIG_ADS_COMPONENT_COMMON_MEMERY_MONITOR
    DUIKIT_LOG_I("-p,    duikit sdk mem status");
    DUIKIT_LOG_I("-c,    duikit sdk mem check");
#endif
}

static int cmd_duikit_sdk(int argc, char **argv)
{
    int c = 0, port;
    optind = 0;
    while ((c = getopt(argc, argv, "hIDm:v:o:s:p:c")) != -1) {
        switch (c) {
            case 'I':
                duikit_sdk_demo();
                break;
            case 'D':
                duikit_sdk_deinit();
                break;
            case 'm':
                duikit_sdk_wakeup_set(atoi(optarg));
                break;
            case 'v':
                duikit_sdk_vad_set(atoi(optarg) == 1);
                break;
            case 'o':
                duikit_sdk_oneshot_set(atoi(optarg) == 1);
                break;
            case 's':
                duikit_sdk_online_set(atoi(optarg) == 1);
                break;
#ifdef CONFIG_ADS_COMPONENT_COMMON_MEMERY_MONITOR
            case 'p':
                if (optarg != NULL) {
                    as_mem_module_status(atoi(optarg));
                } else {
                    printf("\n*************************** ALL MODULE INFO ***************************\n");
                    for (int i = 0; i < AS_MM_MODULE_NUM; i++) {
                        as_mem_module_status(i);
                    }
                    printf("\n************************* ALL MODULE INFO END *************************\n");
                }
                break;
            case 'c':
                as_mem_crossed_check();
                break;
#endif
            case 'h':
            default:
                duikit_sdk_usage();
                return -1;
        }
    }

    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_duikit_sdk, duikit_sdk, duikit sdk test);
