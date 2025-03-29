#include "kernel/include/ipc/mutex.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/core/task.h"

void mutex_init(mutex_t* mutex) {
    mutex->owner = 0;
    mutex->locked_count = 0;
    list_init(&mutex->wait_list);
}

void mutex_lock(mutex_t* mutex) {
    irq_state_t state = irq_enter_protection();

    task_t* curr = task_current();
    if (mutex->locked_count == 0) {
        mutex->locked_count++;
        mutex->owner = curr;
    } else if (mutex->owner == curr) {
        mutex->locked_count++;
    } else { // 已经上锁, 并且不是当前任务的锁
        // 将当前任务放到等待队列
        task_set_block(curr); // 从就绪队列中移除
        list_insert_last(&mutex->wait_list, &curr->wait_node); // 将当前任务放到等待队列
        task_dispatch(); // 调度器切换到下一个任务
    }

    irq_leave_protection(state);
}

void mutex_unlock(mutex_t* mutex) {
    irq_state_t state = irq_enter_protection();

    task_t* curr = task_current();
    if (mutex->owner == curr) {
        if (--mutex->locked_count == 0) {
            mutex->owner = (task_t*)0;

            if (list_count(&mutex->wait_list)) {
                list_node_t* node = list_remove_first(&mutex->wait_list);
                task_t* task = list_node2parent(node, task_t, wait_node);
                task_set_ready(task); // 将任务从等待队列移到就绪队列

                mutex->locked_count = 1;
                mutex->owner = task; // 设置锁的拥有者

                task_dispatch(); // 调度器切换到下一个任务
            }
        }
    }

    irq_leave_protection(state);
}
