#ifndef SEM_H
#define SEM_H

#include "kernel/include/tools/list.h"

typedef struct _sem_t_ {
    int count;
    list_t wait_list;
} sem_t;

void sem_init(sem_t* sem, int init_count);

#endif
