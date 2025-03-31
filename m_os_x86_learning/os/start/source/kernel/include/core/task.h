#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "kernel/include/tools/list.h"

#define TASK_NAME_SIZR 32
#define TASK_TIME_SLICE_DEFAULT 10
#define IDLE_TASK_SIZE 1024

typedef struct _task_t {
    // uint32_t* stack;

    enum {
        TASK_CREATED, // 进程初始化完成后
        TASK_RUNNING, // 进程正在运行
        TASK_SLEEP, // 进程在延时(睡眠)状态
        TASK_READY, // 进程处于就绪状态
        TASK_WAITTING, // 进程在等待某个事件, eg: 等待IO完成, 等待磁盘空闲
    } state;

    int time_ticks; // 进程运行的时间片
    int slice_ticks; // 进程的时间片长度
    int sleep_ticks; // 进程睡眠的时间

    char name[TASK_NAME_SIZR];

    list_node_t run_node;
    list_node_t wait_node; // 方便后续需要计时时将wait_node放到等待队列, 而将run_node放到延时队列
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
    list_t sleep_list;

    task_t first_task;
    task_t idle_task;
} task_manager_t;

void task_manager_init(void);
void task_first_init(void);
task_t* task_first_task(void);

void task_set_ready(task_t* task);
void task_set_block(task_t* task);

task_t* task_next_run(void);
task_t* task_current(void);
int sys_sched_yield(void); //   进程调度
void task_dispatch(void);

void task_time_tick(void);

void task_set_sleep(task_t* task, uint32_t ticks);
void task_set_wakeup(task_t* task);
void sys_sleep(uint32_t ms);

#endif
