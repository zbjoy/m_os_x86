#include "kernel/include/core/task.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "kernel/include/cpu/cpu.h"
#include "kernel/include/tools/log.h"
#include "comm/cpu_instr.h"
#include "kernel/include/cpu/irq.h"

static uint32_t idel_task_stack[IDLE_TASK_SIZE];
static task_manager_t task_manager;

static int tss_init(task_t* task, uint32_t entry, uint32_t esp) {
    int tss_sel = gdt_alloc_desc();
    if (tss_sel < 0) {
        log_printf("alloc tss desc failed\n");
        return -1;
    }
    segment_desc_set(tss_sel, (uint32_t)&task->tss, sizeof(tss_t), SEG_P_PRESENT | SEG_DPL0 | SEG_TYPE_TSS);

    kernel_memset(&task->tss, 0, sizeof(tss_t));
    task->tss.eip = entry;         // 任务入口地址
    task->tss.esp = task->tss.esp0 = esp; // 当前任务的栈顶指针, 因为程序运行在特权级0，所以esp0和esp1指向同一位置
    task->tss.ss = task->tss.ss0 = KERNEL_SELECTOR_DS;          // 任务的堆栈段选择子
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = KERNEL_SELECTOR_DS;          // 任务的数据段选择子
    task->tss.cs = KERNEL_SELECTOR_CS;           // 任务的代码段选择子
    task->tss.eflags = EFLAGS_IF | EFLAGS_DEFAULT;
    task->tss.iomap = 0;

    task->tss_sel = tss_sel;

    return 0;
}



int task_init(task_t *task, const char* name, uint32_t entry, uint32_t esp) {
    ASSERT(task != (task_t*)0);

    tss_init(task, entry, esp);
    kernel_strncpy(task->name, name, TASK_NAME_SIZR);
    task->state = TASK_CREATED;
    // uint32_t* pesp = (uint32_t*)esp;
    // if (pesp) {
    //     // 设置的寄存器依据 source/kernel/init/start.S 中的设置
    //     /**
    //      *  pop %edi
    //      *  pop %esi
    //      *  pop %ebx
    //      *  pop %ebp
    //      */
    //     *(--pesp) = entry; // EIP 设置成任务入口地址, 使在任务切换的时候可以返回到下一个要跳转的函数中去
    //     *(--pesp) = 0; // EDI
    //     *(--pesp) = 0; // ESI
    //     *(--pesp) = 0; // EBX
    //     *(--pesp) = 0; // EBP
    //     task->stack = pesp; // 任务栈指针
    // }

    task->time_ticks = TASK_TIME_SLICE_DEFAULT;
    task->slice_ticks = task->time_ticks;
    task->sleep_ticks = 0;

    list_node_init(&task->run_node);
    list_node_init(&task->all_node);

    // 属于临界区, 必须使用临界区保护
    irq_state_t state = irq_enter_protection();

    task_set_ready(task); // 将任务设置到就绪队列

    list_insert_last(&task_manager.task_list, &task->all_node);

    // 退出临界区
    irq_leave_protection(state);
    
    return 0;
}

// 函数声明, 实现在 source/kernel/init/start.S 中
void simple_switch(uint32_t** from, uint32_t* to);

void task_switch_from_to(task_t* from, task_t* to) {
    switch_to_tss(to->tss_sel);
    // simple_switch(&from->stack, to->stack);
}

static void idle_task_entry(void) {
    for (;;) {
        hlt(); // 执行低功耗指令, 让 CPU 进入空闲状态
    }
}

void task_manager_init(void) {
    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    task_manager.curr_task = (task_t*)0;

    task_init(&task_manager.idle_task,
             "idle_task",
              (uint32_t)idle_task_entry,
              (uint32_t)idel_task_stack + IDLE_TASK_SIZE
    );
}

void task_first_init(void) {
    task_init(&task_manager.first_task, "first task", 0, 0);
    write_tr(task_manager.first_task.tss_sel);
    task_manager.curr_task = &task_manager.first_task;
}

task_t* task_first_task(void) {
    return &task_manager.first_task;
}

void task_set_ready(task_t* task) {
    if (task == &task_manager.idle_task) {
        return;
    }
    list_insert_last(&task_manager.ready_list, &task->run_node);
    task->state = TASK_READY;
}

void task_set_block(task_t* task) {
    if (task == &task_manager.idle_task) {
        return;
    }
    list_remove(&task_manager.ready_list, &task->run_node);
}

task_t* task_next_run(void) {
    if (list_count(&task_manager.ready_list) == 0) {
        return &task_manager.idle_task;
    }
    list_node_t* task_node = list_first(&task_manager.ready_list);
    return list_node2parent(task_node, task_t, run_node);
}

task_t* task_current(void) {
    return task_manager.curr_task;
}

int sys_sched_yield(void) {
    // 临界区保护
    irq_state_t state = irq_enter_protection();

    if (list_count(&task_manager.ready_list) > 1) { // 至少有两个就绪任务 才切换
        task_t* curr_task = task_current();

        task_set_block(curr_task); // 将当前任务从头部取出来
        task_set_ready(curr_task); // 将当前任务放到尾部

        task_dispatch(); // 切换任务到下一个就绪任务
    }

    // 退出临界区
    irq_leave_protection(state);

    return 0;
}

void task_dispatch(void) {
    // 临界区保护
    irq_state_t state = irq_enter_protection();

    task_t* to = task_next_run();
    if (to != task_current()) {
        task_t* from = task_current();
        task_manager.curr_task = to;
        to->state = TASK_RUNNING;
        // from->state = TASK_READY;
        task_switch_from_to(from, to);
    }

    // 退出临界区
    irq_leave_protection(state);
}

void task_time_tick(void) {
    task_t* curr_task = task_current();

    if (--curr_task->slice_ticks == 0) { // 说明任务运行时间太长, 时间片用完了
        curr_task->slice_ticks = curr_task->time_ticks;

        task_set_block(curr_task);
        task_set_ready(curr_task);

        task_dispatch();
    }

    list_node_t* curr = list_first(&task_manager.sleep_list);
    while (curr) {
        list_node_t* next = list_node_next(curr);
        task_t* task = list_node2parent(curr, task_t, run_node);
        // 要注意一旦 sleep_ticks 是 负数, 会导致死循环
        if (--task->sleep_ticks == 0) {
            task_set_wakeup(task);
            task_set_ready(task);
        }

        curr = next;
    }
    task_dispatch();
}

void task_set_sleep(task_t* task, uint32_t ticks) {
    if (ticks == 0) {
        return;
    }

    task->sleep_ticks = ticks;
    task->state = TASK_SLEEP;
    list_insert_last(&task_manager.sleep_list, &task->run_node);
}

void task_set_wakeup(task_t* task) {
    list_remove(&task_manager.sleep_list, &task->run_node);
}
void sys_sleep(uint32_t ms) {
    irq_state_t state = irq_enter_protection();

    // 将 ms 改成对时钟节拍的计数
    task_set_block(task_current());
    
    // 一个时钟节拍是 10ms, 所以需要将 ms 除以 10 得到时钟节拍数
    task_set_sleep(task_current(), (ms + (OS_TICKS_MS - 1)) / OS_TICKS_MS);
    task_dispatch();

    irq_leave_protection(state);
}

