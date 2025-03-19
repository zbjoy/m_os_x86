#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "kernel/include/tools/list.h"

#define TASK_NAME_SIZR 32

typedef struct _task_t {
    // uint32_t* stack;

    enum {
        TASK_CREATED, // 进程初始化完成后
        TASK_RUNNING, // 进程正在运行
        TASK_SLEEP, // 进程在延时(睡眠)状态
        TASK_READY, // 进程处于就绪状态
        TASK_WAITTING, // 进程在等待某个事件, eg: 等待IO完成, 等待磁盘空闲
    } state;

    char name[TASK_NAME_SIZR];

    list_node_t run_node;
    list_node_t all_node;
    tss_t tss;
    uint16_t tss_sel;
}task_t;

int task_init(task_t *task, const char* name, uint32_t entry, uint32_t esp);

void task_switch_from_to(task_t *from, task_t *to);

typedef struct _task_manager_t {
    task_t* curr_task;
    list_t ready_list;
    list_t task_list;

    task_t first_task;
} task_manager_t;

void task_manager_init(void);
void task_first_init(void);
task_t* task_first_task(void);

void task_set_ready(task_t* task);
void task_set_block(task_t* task);

task_t* task_next_run(void);
task_t* task_current(void);
int sys_sched_yield(void);
void task_dispatch(void);

#endif
