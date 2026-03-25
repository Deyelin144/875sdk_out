#include <string.h>
#include <hal_gpio.h>
#include <hal_time.h>
#include "compiler.h"
#include <console.h>
#include "../drv_log.h"
#include "dev_robot.h"
#include "../atcmd/mcu_at_device.h"
#include "../atcmd/at_command_handler.h"
#include "kernel/os/os.h"

#define ROBOT_CMD_QUEUE_SIZE 10
#define ROBOT_CMD_MAX_LEN 128

typedef struct robot_action_cmd {
    int action;
    int steps;
    int time;
}robot_action_cmd_t;

typedef struct robot_cmd_queue {
    char cmd[ROBOT_CMD_MAX_LEN];
    int need_delay;  // 是否需要延时
} robot_cmd_queue_t;

// 命令队列相关
static robot_cmd_queue_t s_robot_cmd_queue[ROBOT_CMD_QUEUE_SIZE];
static int s_robot_queue_head = 0;
static int s_robot_queue_tail = 0;
static int s_robot_queue_count = 0;
static XR_OS_Mutex_t s_robot_queue_mutex;
static int s_robot_queue_initialized = 0;

// 延时定时器
static XR_OS_Timer_t s_robot_delay_timer;
static int s_robot_processing = 0;
static int s_robot_last_was_home = 0;


// 处理队列中的下一条命令
static void robot_process_next_cmd(void);

// 延时定时器回调函数
static void robot_delay_timer_cb(void *arg)
{
    // 延时结束，处理下一条命令
    robot_process_next_cmd();
}

// 发送实际的机器人命令
static void robot_send_actual_cmd(const char *cmd)
{
    printf("\n[ACTION] robot cmdlist: %s\n", cmd);
    at_send_command(AT_REQ_CMD_ROBOT, cmd, NULL, 0);
}

// 处理队列中的下一条命令
static void robot_process_next_cmd(void)
{
    XR_OS_MutexLock(&s_robot_queue_mutex, XR_OS_WAIT_FOREVER);

    if(s_robot_queue_count == 0){
        s_robot_processing = 0;
        XR_OS_MutexUnlock(&s_robot_queue_mutex);
        return;
    }

    // 取出队列头部的命令
    robot_cmd_queue_t *cmd_item = &s_robot_cmd_queue[s_robot_queue_head];

    // 发送命令
    robot_send_actual_cmd(cmd_item->cmd);

    int need_delay = cmd_item->need_delay;

    // 记录当前命令是否是 "0,0" 开头的
    s_robot_last_was_home = need_delay;

    // 更新队列
    s_robot_queue_head = (s_robot_queue_head + 1) % ROBOT_CMD_QUEUE_SIZE;
    s_robot_queue_count--;

    XR_OS_MutexUnlock(&s_robot_queue_mutex);

    // 如果需要延时，启动定时器；否则继续处理下一条
    if(need_delay){
        XR_OS_TimerStart(&s_robot_delay_timer);
    } else {
        robot_process_next_cmd();
    }
}

int dev_robot_control(int action, int steps, int time)
{
    if(action < 0){
        printf("%s %s %d: robot action setting fail!\n", __FILE__, __func__, __LINE__);
        return ROBOT_RES_ERROR;
    }
    if(steps < 0){
        printf("%s %s %d: robot steps setting fail!\n", __FILE__, __func__, __LINE__);
        return ROBOT_RES_ERROR;
    }
    if(time < 0){
        printf("%s %s %d: robot time setting fail!\n", __FILE__, __func__, __LINE__);
        return ROBOT_RES_ERROR;
    }
    char cmd[64]={0};
    if(steps == 0 && time == 0){
        memset(cmd, 0, sizeof(cmd));
        switch(action) {
            case HOME_ROBOT_CMD:
                sprintf(cmd, "%d,0,150\r\n",HOME_ROBOT_CMD);
                break;
            case WALK_FORWARD_ROBOT_CMD:
                sprintf(cmd, "%d,2,1500\r\n",WALK_FORWARD_ROBOT_CMD);
                break;
            case WALK_BACKWARD_ROBOT_CMD:
                sprintf(cmd, "%d,2,1500\r\n",WALK_BACKWARD_ROBOT_CMD);
                break;
            case WALK_LEFT_ROBOT_CMD:
                sprintf(cmd, "%d,4,1500\r\n",WALK_LEFT_ROBOT_CMD);
                break;
            case WALK_RIGHT_ROBOT_CMD:
                sprintf(cmd, "%d,4,1500\r\n",WALK_RIGHT_ROBOT_CMD);
                break;
            case SLIDE_ROBOT_CMD:
                sprintf(cmd, "%d,8,120\r\n",SLIDE_ROBOT_CMD);
                break;
            case CRUSAITO_ROBOT_CMD:
                sprintf(cmd, "%d,4,1200\r\n",CRUSAITO_ROBOT_CMD);
                break;
            case JITTER_ROBOT_CMD:
                sprintf(cmd, "%d,8,90\r\n",JITTER_ROBOT_CMD);
                break;
            case STOMP_ROBOT_CMD:
                sprintf(cmd, "%d,8,90\r\n",STOMP_ROBOT_CMD);
                break;
            default:
                break;
        }
    }else{
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "%d,%d,%d\r\n",action, steps, time);
    }
    at_send_command(AT_REQ_CMD_ROBOT, cmd, NULL,0);
    return ROBOT_RES_SUCCESS;
}

int dev_robot_cmdlist_send(const char *send_arg)
{
    if(send_arg == NULL){
        return ROBOT_RES_ERROR;
    }

    // 初始化队列（仅第一次）
    if(!s_robot_queue_initialized){
        XR_OS_MutexCreate(&s_robot_queue_mutex);
        XR_OS_TimerCreate(&s_robot_delay_timer, XR_OS_TIMER_ONCE,
                         robot_delay_timer_cb, NULL, 500);
        s_robot_queue_initialized = 1;
    }

    // 判断当前命令是否是 "0,0" 开头
    int is_home_cmd = (send_arg[0] == '0' && send_arg[1] == ',' && send_arg[2] == '0');

    XR_OS_MutexLock(&s_robot_queue_mutex, XR_OS_WAIT_FOREVER);

    // 过滤逻辑：如果上一条命令是 "0,0" 开头，且当前命令也是 "0,0" 开头，则过滤掉
    if(s_robot_last_was_home && is_home_cmd){
        XR_OS_MutexUnlock(&s_robot_queue_mutex);
        printf("\n[ACTION] Filtered duplicate: %s\n", send_arg);
        return ROBOT_RES_SUCCESS;
    }

    // 检查队列是否已满
    if(s_robot_queue_count >= ROBOT_CMD_QUEUE_SIZE){
        XR_OS_MutexUnlock(&s_robot_queue_mutex);
        return ROBOT_RES_ERROR;
    }

    // 将命令添加到队列
    robot_cmd_queue_t *cmd_item = &s_robot_cmd_queue[s_robot_queue_tail];
    strncpy(cmd_item->cmd, send_arg, ROBOT_CMD_MAX_LEN - 1);
    cmd_item->cmd[ROBOT_CMD_MAX_LEN - 1] = '\0';
    cmd_item->need_delay = is_home_cmd;

    s_robot_queue_tail = (s_robot_queue_tail + 1) % ROBOT_CMD_QUEUE_SIZE;
    s_robot_queue_count++;

    // 如果当前没有在处理命令，立即开始处理
    int should_start = !s_robot_processing;
    if(should_start){
        s_robot_processing = 1;
    }

    XR_OS_MutexUnlock(&s_robot_queue_mutex);

    // 开始处理队列
    if(should_start){
        robot_process_next_cmd();
    }

    return ROBOT_RES_SUCCESS;
}

int dev_robot_action_over(void){
    // 动作执行完毕，表示已经回到 home 位置
    // 设置标记，下一个 home 命令会被过滤掉
    if(s_robot_queue_initialized){
        XR_OS_MutexLock(&s_robot_queue_mutex, XR_OS_WAIT_FOREVER);
        s_robot_last_was_home = 1;  // 设置为 1 表示已在 home 位置
        XR_OS_MutexUnlock(&s_robot_queue_mutex);
        printf("\n[ACTION] Action over.\n");
    }
    return ROBOT_RES_SUCCESS;
}

int dev_robot_test(int argc, const char **argv)
{
    if(0 == memcmp(argv[1], "action", strlen("action"))){
        if (3 != argc) {
            printf("param is invalid, please input again.\n");
        }
        char cmd[64]={0};
        sprintf(cmd, "%s\r\n",argv[2]);
        printf("robot cmd: %s\n",cmd);
        // at_send_command(AT_REQ_CMD_ROBOT, cmd, NULL,0);
        dev_robot_cmdlist_send(cmd);
    }
    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(dev_robot_test, robot, robot action options);