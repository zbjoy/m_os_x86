#include "kernel/include/ipc/sem.h"
#include "kernel/include/core/task.h"
#include "kernel/include/cpu/irq.h"

void sem_init(sem_t* sem, int init_count) {
    sem->count = init_count;
    list_init(&sem->wait_list);
}

void sem_wait(sem_t* sem) {
    irq_state_t state = irq_enter_protection();
    if (sem->count > 0) {
        sem->count--;
    } else {
        task_t* curr = task_current();
        task_set_block(curr); // 从就绪队列中移除
        list_insert_last(&sem->wait_list, &curr->wait_node); // 将当前任务放到等待队列
        task_dispatch(); // 调度器切换到下一个任务
    }
    irq_leave_protection(state);
}

// semaphore 通知函数
void sem_notify(sem_t* sem) {
    irq_state_t state = irq_enter_protection();

    // 判断是否有任务在等待
    if (list_count(&sem->wait_list)) {
        list_node_t* node = list_remove_first(&sem->wait_list);
        task_t* task = list_node2parent(node, task_t, wait_node);
        task_set_ready(task); // 将任务从等待队列移到就绪队列
        task_dispatch(); // 调度器切换到下一个任务
    } else {
        sem->count++;
    }

    irq_leave_protection(state);
}

int sem_count(sem_t* sem) {
    irq_state_t state = irq_enter_protection();
    int count = sem->count;
    irq_leave_protection(state);
    return count;
}
