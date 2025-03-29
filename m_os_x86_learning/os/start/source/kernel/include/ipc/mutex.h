#ifndef MUTEX_H
#define MUTEX_H

#include "kernel/include/tools/list.h"

typedef struct _mutex_t {
    task_t* owner; // 锁的拥有者
    int locked_count; // 上锁的次数
    list_t wait_list;
} mutex_t;

void mutex_init(mutex_t* mutex);
void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);

#endif
