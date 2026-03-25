#ifndef DEV_ROBOT_H__
#define DEV_ROBOT_H__

#include "../atcmd/mcu_at_device.h"
#include "../atcmd/at_command_handler.h"

typedef enum {
    ROBOT_RES_ERROR = -1,
    ROBOT_RES_SUCCESS = 0,
} RobotRet;
typedef enum {
    HOME_ROBOT_CMD = 0,             //归位      0
    WALK_FORWARD_ROBOT_CMD,         //前进      1
    WALK_BACKWARD_ROBOT_CMD,        //后退      2
    WALK_LEFT_ROBOT_CMD,            //左转      3
    WALK_RIGHT_ROBOT_CMD,           //右转      4
    JITTER_ROBOT_CMD,               //抖动      5
    SLIDE_ROBOT_CMD,                //搓脚      6
    STOMP_ROBOT_CMD,                //快速跺脚  7
    STOMP_ROBOT_CMD_SLOW,           //慢速跺脚  8
    STOMP_ROBOT_CMD_LEFT,           //跺左脚    9
    STOMP_ROBOT_CMD_RIGHT,          //跺右脚    10
    CRUSAITO_ROBOT_CMD,             //左右摇头  11
    CRUSAITO_ROBOT_CMD_LEFT,        //左摇头    12
    CRUSAITO_ROBOT_CMD_RIGHT,       //右摇头    13
    DANCE_ROBOT_CMD,                //跳舞      14
    POSTURE_ROBOT_CMD,              //立正      15
    POSTURE_ROBOT_CMD_EASE,         //稍息      16
    SWING_ROBOT_CMD_LEFT,           //单腿摇摆  17
    SWING_ROBOT_CMD_RIGHT,          //单腿摇摆  18
    TWIST_ROBOT_CMD,                //左右扭动  19
    FALL_ROBOT_CMD,                 //摔倒      20
    TEST_ROBOT_CMD,                 //测试      21
    SPLIT_ROBOT_CMD,                //劈叉      22
    MAX_ROBOT_CMD                   //MAX      23
} RobotCmd;

int dev_robot_control(int action, int steps, int time);
int dev_robot_cmdlist_send(const char *send_arg);
int dev_robot_action_over(void);

#endif /* DEV_ROBOT_H__ */