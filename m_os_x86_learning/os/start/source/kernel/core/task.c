#include "kernel/include/core/task.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "kernel/include/cpu/cpu.h"
#include "kernel/include/tools/log.h"

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



int task_init(task_t *task, uint32_t entry, uint32_t esp) {
    ASSERT(task != (task_t*)0);

    // tss_init(task, entry, esp);
    uint32_t* pesp = (uint32_t*)esp;
    if (pesp) {
        // 设置的寄存器依据 source/kernel/init/start.S 中的设置
        /**
         *  pop %edi
         *  pop %esi
         *  pop %ebx
         *  pop %ebp
         */
        *(--pesp) = entry; // EIP 设置成任务入口地址, 使在任务切换的时候可以返回到下一个要跳转的函数中去
        *(--pesp) = 0; // EDI
        *(--pesp) = 0; // ESI
        *(--pesp) = 0; // EBX
        *(--pesp) = 0; // EBP
        task->stack = pesp; // 任务栈指针
    }
    return 0;
}

// 函数声明, 实现在 source/kernel/init/start.S 中
void simple_switch(uint32_t** from, uint32_t* to);

void task_switch_from_to(task_t* from, task_t* to) {
    // switch_to_tss(to->tss_sel);
    simple_switch(&from->stack, to->stack);
}

