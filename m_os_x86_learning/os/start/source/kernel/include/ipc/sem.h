#ifndef SEM_H
#define SEM_H

#include "kernel/include/tools/list.h"

typedef struct _sem_t_ {
    int count; // 信号量的值
    list_t wait_list; // 等待队列
} sem_t;

void sem_init(sem_t* sem, int init_count);
void sem_wait(sem_t* sem);
void sem_notify(sem_t* sem);
int sem_count(sem_t* sem);

#endif
