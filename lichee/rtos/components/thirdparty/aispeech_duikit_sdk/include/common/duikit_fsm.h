#ifndef __DUIKIT_FSM_H__
#define __DUIKIT_FSM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 有限状态机实现
 */

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    //触发事件
    int event;
    //事件数据
    void *event_data;
} duikit_fsm_event_t;

/*
 * [当前状态]下根据[触发事件]执行[动作]并进入[下一个状态]
 * 注意当action返回值作为下一个状态为第一优先级，若为-1则按照静态表进行状态迁移
 */
typedef struct {
    //触发事件
    int event;
    //当前状态
    int state;
    //动作，返回值为执行完动作所要进入的下一个状态
    int (*action)(duikit_fsm_event_t *event, void *user_data);
} duikit_fsm_transfer_t;

/*
 * FSM状态机管理器
 */
typedef struct {
    //当前状态
    int state;
    //状态迁移表
    duikit_fsm_transfer_t *transfer_table;
    //状态迁移表大小
    int table_size;
} duikit_fsm_t;

//若当前状态下无法处理触发事件则返回fasle
static inline bool duikit_fsm_handle(duikit_fsm_t *self, duikit_fsm_event_t *event, void *user_data)
{
    int i;
    for (i = 0; i < self->table_size; i++) {
        if (event->event == self->transfer_table[i].event &&
                self->state == self->transfer_table[i].state) {
            self->state = self->transfer_table[i].action(event, user_data);
            return true;
        }
    }
    return false;
}

#ifdef __cplusplus
}
#endif

#endif //__DUIKIT_FSM_H__
