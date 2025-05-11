#include "kernel/include/core/task.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "kernel/include/cpu/cpu.h"
#include "kernel/include/tools/log.h"
#include "comm/cpu_instr.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/core/memory.h"
#include "kernel/include/cpu/mmu.h"
#include "kernel/include/core/syscall.h"
#include "comm/elf.h"
#include "kernel/include/fs/fs.h"

static uint32_t idel_task_stack[IDLE_TASK_SIZE];
static task_manager_t task_manager;
static task_t task_table[TASK_NR]; // 任务表, 用于存放所有的任务
static mutex_t task_table_mutex; // 任务表的互斥锁

static int tss_init(task_t* task, int flag, uint32_t entry, uint32_t esp) {
    int tss_sel = gdt_alloc_desc();
    if (tss_sel < 0) {
        log_printf("alloc tss desc failed\n");
        return -1;
    }
    segment_desc_set(tss_sel, (uint32_t)&task->tss, sizeof(tss_t), SEG_P_PRESENT | SEG_DPL0 | SEG_TYPE_TSS);

    kernel_memset(&task->tss, 0, sizeof(tss_t));

    uint32_t kernel_stack = memory_alloc_page(); // 分配内核栈, 4KB
    if (kernel_stack == 0) {
        goto task_init_failed; // 分配失败, 释放 tss_sel
    }

    int code_sel, data_sel;
    // 判断 flag 是否是系统任务
    if (flag & TASK_FLAGS_SYSTEM) {
        code_sel = KERNEL_SELECTOR_CS; // 代码段选择子, 0级特权级 
        data_sel = KERNEL_SELECTOR_DS; // 数据段选择子, 0级特权级 
    } else {
        code_sel = task_manager.app_code_sel | SEG_CPL3; // 代码段选择子, 3级特权级
        data_sel = task_manager.app_data_sel | SEG_CPL3; // 数据段选择子, 3级特权级
    }

    task->tss.eip = entry;         // 任务入口地址
    task->tss.esp = esp;
    task->tss.esp0 = kernel_stack + MEM_PAGE_SIZE; // 当前任务的栈顶指针, 因为程序运行在特权级0，所以esp0和esp1指向同一位置
    task->tss.ss = data_sel; // 任务的堆栈段选择子
    task->tss.ss0 = KERNEL_SELECTOR_DS;          // 内核数据段选择子
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = data_sel;          // 任务的数据段选择子
    task->tss.cs = code_sel;           // 任务的代码段选择子
    task->tss.eflags = EFLAGS_IF | EFLAGS_DEFAULT;

    uint32_t page_dir = memory_create_uvm(); // 创建一个新的页目录表, 物理地址
    if (page_dir == 0) {
        goto task_init_failed; // 分配失败,TODO: 释放 tss_sel
        // return -1;
    }
    task->tss.cr3 = page_dir; // 设置任务的页目录表地址

    // task->tss.iomap = 0;

    task->tss_sel = tss_sel;

    return 0;
task_init_failed:
    gdt_free_sel(tss_sel); // 释放分配的选择子
    if (kernel_stack) { // 如果分配了内核栈, 释放掉
        memory_free_page(kernel_stack); // 释放分配的内核栈
    }
    return -1;
}



int task_init(task_t *task, const char* name, int flag, uint32_t entry, uint32_t esp) {
    ASSERT(task != (task_t*)0);

    tss_init(task, flag, entry, esp);
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

    task->parent = (task_t*)0; // 父进程, 这里没有父进程, 所以设置为 0
    task->heap_start = 0; // 堆空间起始地址
    task->heap_end = 0; // 堆空间结束地址

    list_node_init(&task->run_node);
    list_node_init(&task->wait_node);
    list_node_init(&task->all_node);

    // 属于临界区, 必须使用临界区保护
    irq_state_t state = irq_enter_protection();
    task->pid = (uint32_t)task; // 任务的 PID, 这里使用任务的地址作为 PID, 方便后续查找

    // task_set_ready(task); // 将任务设置到就绪队列 // 将 task_set_ready 的工作在 task_start 中完成

    list_insert_last(&task_manager.task_list, &task->all_node);

    // 退出临界区
    irq_leave_protection(state);
    
    return 0;
}

void task_start(task_t* task) {
    // 任务的状态设置为就绪状态
    irq_state_t state = irq_enter_protection(); // 进入临界区保护
    task_set_ready(task); // 将任务设置到就绪队列
    irq_leave_protection(state); // 退出临界区保护
}

void task_uninit(task_t* task) {
    if (task->tss_sel) {
        gdt_free_sel(task->tss_sel); // 释放分配的选择子
    }

    if (task->tss.esp0) {
        memory_free_page(task->tss.esp0 - MEM_PAGE_SIZE); // 释放分配的内核栈
    }

    if (task->tss.cr3) {
        memory_destroy_uvm(task->tss.cr3); // 释放分配的页目录表
    }

    kernel_memset(task, 0, sizeof(task_t)); // 清空任务结构体
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
    kernel_memset(task_table, 0, sizeof(task_table)); // 清空任务表
    mutex_init(&task_table_mutex); // 初始化任务表的互斥锁

    int sel = gdt_alloc_desc();
    segment_desc_set(sel, 0x00000000, 0xFFFFFFFF, 
        SEG_P_PRESENT | SEG_DPL3 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW | SEG_D // 代码段选择子
    );
    task_manager.app_data_sel = sel;

    sel = gdt_alloc_desc();
    segment_desc_set(sel, 0x00000000, 0xFFFFFFFF, 
        SEG_P_PRESENT | SEG_DPL3 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW | SEG_D // 数据段选择子
    );
    task_manager.app_code_sel = sel;

    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    task_manager.curr_task = (task_t*)0;

    task_init(&task_manager.idle_task,
             "idle_task",
              TASK_FLAGS_SYSTEM,
              (uint32_t)idle_task_entry,
              (uint32_t)idel_task_stack + IDLE_TASK_SIZE
    );
    task_start(&task_manager.idle_task); // 启动 idle_task
}

void task_first_init(void) {
    void first_task_entry(void);

    extern uint8_t s_first_task[], e_first_task[]; // 任务的入口地址 和 结束地址
    
    uint32_t copy_size = (uint32_t)(e_first_task - s_first_task); // 要拷贝的区域的大小
    uint32_t alloc_size = 10 * MEM_PAGE_SIZE; // 分配的大小, 10 个物理页
    ASSERT(copy_size < alloc_size); // 拷贝的大小不能超过分配的大小

    uint32_t first_start = (uint32_t)first_task_entry;

    // task_init(&task_manager.first_task, "first task", 0, 0); // 传入当前的地址
    task_init(&task_manager.first_task, "first task", 0, first_start, first_start + alloc_size); // 改成传入 first_task_entry 地址
    task_manager.first_task.heap_start = (uint32_t)e_first_task;
    task_manager.first_task.heap_end = (uint32_t)e_first_task;

    write_tr(task_manager.first_task.tss_sel);
    task_manager.curr_task = &task_manager.first_task;

    // 将页表切换到 first_task 的页表
    mmu_set_page_dir(task_manager.first_task.tss.cr3); // 设置页目录表

    memory_alloc_page_for(first_start, alloc_size, PTE_P | PTE_W | PTE_U);
    kernel_memcpy((void*)first_start, (void*)s_first_task, copy_size); // 拷贝到分配的内存中去

    task_start(&task_manager.first_task); // 启动 first_task
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

static task_t* alloc_task(void) {
    task_t* task = (task_t*)0;

    mutex_lock(&task_table_mutex); // 进入临界区
    for (int i = 0; i < TASK_NR; i++) {
        task_t* curr = task_table + i;
        if (curr->name[0] == '\0') {
            task = curr;
            break;
        }
    }
    mutex_unlock(&task_table_mutex); // 退出临界区

    return task;
}

static void free_task(task_t* task) {
    mutex_lock(&task_table_mutex); // 进入临界区
    task->name[0] = '\0'; // 清空任务名称, 释放任务
    mutex_unlock(&task_table_mutex); // 退出临界区
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

int sys_getpid(void) {
    task_t* task = task_current();
    return task->pid;
}

int sys_fork(void) {
    task_t* parent_task = task_current();
    task_t* child_task = alloc_task(); // 分配一个新的任务
    if (child_task == (task_t*)0) {
        goto fork_failed;
    }

    syscall_frame_t* frame = (syscall_frame_t*)(parent_task->tss.esp0 - sizeof(syscall_frame_t)); // 获取当前任务的栈指针

    int err = task_init(child_task, parent_task->name, 0, frame->eip, frame->esp + sizeof(uint32_t) * SYSCALL_PARAM_COUNT); // 初始化子任务
    if (err < 0) {
        goto fork_failed;
    }

    tss_t* tss = &child_task->tss;
    tss->eax = 0; 
    tss->ebx = frame->ebx;
    tss->ecx = frame->ecx;
    tss->edx = frame->edx;
    tss->esi = frame->esi;
    tss->edi = frame->edi;
    tss->ebp = frame->ebp;

    tss->cs = frame->cs; // 设置代码段选择子
    tss->ds = frame->ds; // 设置数据段选择子
    tss->es = frame->es; // 设置数据段选择子
    tss->fs = frame->fs; // 设置数据段选择子
    tss->gs = frame->gs; // 设置数据段选择子
    tss->eflags = frame->eflags; // 设置标志寄存器

    child_task->parent = parent_task; // 设置父进程

    if ((tss->cr3 = memory_copy_uvm(parent_task->tss.cr3)) < 0) { // 复制父进程的页目录表
        goto fork_failed; // 复制失败, 释放子任务
    }

    // 直接复制 父进程的tss.cr3页表, 会导致后面父进程因为和子进程栈是同一个导致互相破坏, 并且在后面父进程与子进程执行代码不一样时会导致修改同一个页表, 造成错误
    // tss->cr3 = parent_task->tss.cr3; // 设置页目录表地址, 这里是父进程的页目录表地址

    task_start(child_task); // 启动子任务
    return child_task->pid; // 返回子进程的 PID
    
fork_failed:
    if (child_task) {
        task_uninit(child_task);
        free_task(child_task);
    }
    return -1; // 创建子进程失败
}

static int load_phdr(int file, Elf32_Phdr* phdr, uint32_t page_dir) {
    // 分配内存
    int err = memory_alloc_for_page_dir(page_dir, phdr->p_vaddr, phdr->p_memsz, PTE_P | PTE_U | PTE_W);
    if (err < 0) {
        log_printf("no memory, alloc memory for phdr failed.\n");
        return -1;
    }

    if (sys_lseek(file, phdr->p_offset, 0) < 0) {
        log_printf("read file failed, lseek failed.\n");
        return -1;
    }

    uint32_t vaddr = phdr->p_vaddr;
    uint32_t size = phdr->p_filesz;
    while (size > 0) {
        int curr_size = (size > MEM_PAGE_SIZE) ? MEM_PAGE_SIZE : size;
        uint32_t paddr = memory_get_paddr(page_dir, vaddr);

        if (sys_read(file, (char*)paddr, curr_size) < curr_size) {
            log_printf("read file failed.");
            return -1;
        }

        size -= curr_size;
        vaddr += curr_size;
    }

    return 0;
}

static uint32_t load_elf_file(task_t* task, char* name, uint32_t page_dir) {
    // 这里需要实现加载 ELF 文件的功能, 目前先返回 0
    Elf32_Ehdr elf_hdr; // ELF 头部
    Elf32_Phdr elf_phdr; // ELF 程序头部

    int file = sys_open(name, 0); // 打开 ELF 文件
    if (file < 0) {
        log_printf("open file  %s failed.\n", name);
        goto load_failed;
    }
    
    int cnt = sys_read(file, (char*)&elf_hdr, sizeof(elf_hdr));
    if (cnt < sizeof(Elf32_Ehdr)) {
        log_printf("elf hdr too small. size = %d\n", cnt);
        goto load_failed;
    }

    if ((elf_hdr.e_ident[0] != 0x7F) || (elf_hdr.e_ident[1] != 'E') || (elf_hdr.e_ident[2] != 'L') || (elf_hdr.e_ident[3] != 'F')) {
        log_printf("not a valid elf file.\n");
        goto load_failed;
    }

    // TODO: 校验 ELF 文件的正确性

    uint32_t e_phoff = elf_hdr.e_phoff; // 程序头表偏移
    for (int i = 0; i < elf_hdr.e_phnum; i++, e_phoff += elf_hdr.e_phentsize) {
        if (sys_lseek(file, e_phoff, 0) < 0) {
            log_printf("lseek failed.\n");
            goto load_failed;
        }

        // 读取 Program Header 0 (或者 Program Header 1...n)
        cnt = sys_read(file, (char*)&elf_phdr, sizeof(elf_phdr)); // 读取程序头部
        if (cnt < sizeof(Elf32_Phdr)) {
            log_printf("elf phdr too small. size = %d\n", cnt);
            goto load_failed;
        }

        if ((elf_phdr.p_type != PT_LOAD) || (elf_phdr.p_vaddr < MEMORY_TASK_BASE)) {
            continue; // 跳过非加载段
        }

        int err = load_phdr(file, &elf_phdr, page_dir); // 加载程序头部
        if (err < 0) {
            log_printf("load phdr programe %d failed.\n", i);
            goto load_failed;
        }

        task->heap_start = elf_phdr.p_vaddr + elf_phdr.p_memsz; // 更新堆的起始地址
        task->heap_end = task->heap_start; // 更新堆的结束地址
    }

    sys_close(file); // 关闭文件
    return elf_hdr.e_entry; // 返回入口地址

load_failed:
    if (file) {
        sys_close(file); // 关闭文件
    }


    return 0; // 加载失败, 返回 0
}

static int copy_args(char* to, uint32_t page_dir, int argc, char** argv) {
    task_args_t task_args; // 任务参数结构体
    task_args.argc = argc;
    task_args.argv = (char**)(to + sizeof(task_args_t));

    char* dest_arg = to + sizeof(task_args_t) + sizeof(char*) * argc; // 参数的起始地址
    char** dest_arg_tb = (char**)memory_get_paddr(page_dir, (uint32_t)(to + sizeof(task_args_t))); // 获取参数表的物理地址
    for (int i = 0; i < argc; i++) {
        char* from = argv[i];
        int len = kernel_strlen(from) + 1; // 字符串长度 + 1
        int err = memory_copy_uvm_data((uint32_t)dest_arg, page_dir, (uint32_t)from, len); // 拷贝参数到新的栈中去
        ASSERT(err >= 0);
    
        dest_arg_tb[i] = dest_arg; // 设置参数表
        dest_arg += len; // 指向下一个参数的起始地址
    }

    return memory_copy_uvm_data((uint32_t)to, page_dir, (uint32_t)&task_args, sizeof(task_args)); // 拷贝参数结构体到新的栈中去
}

int sys_execve(char* name, char** argv, char** env) { // 执行进程
    task_t* task = task_current();

    kernel_strncpy(task->name, get_file_name(name), TASK_NAME_SIZR); // 设置任务名称

    uint32_t old_page_dir = task->tss.cr3; // 保存旧的页目录表地址
    uint32_t new_page_dir = memory_create_uvm(); // 创建一个新的页目录表, 物理地址
    if (!new_page_dir) {
        goto execve_failed; // 创建失败, 释放页目录表
    }

    uint32_t entry = load_elf_file(task, name, new_page_dir); // 加载 ELF 文件, 返回入口地址
    if (entry == 0) {
        goto execve_failed; // 加载失败, 释放页目录表
    }

    uint32_t stack_top = MEM_TASK_STACK_TOP - MEM_TASK_ARG_SIZE; // 栈顶地址, 这里是栈的起始地址
    int err = memory_alloc_for_page_dir(
        new_page_dir, MEM_TASK_STACK_TOP - MEM_TASK_STACK_SIZE,
        MEM_TASK_STACK_SIZE, PTE_P | PTE_U | PTE_W
    );
    if (err < 0) {
        goto execve_failed; // 栈分配失败, 释放页目录表
    }

    int argc = strings_count(argv); // 获取参数个数
    err = copy_args((char*)stack_top, new_page_dir, argc, argv); // 拷贝参数到新的栈中去
    if (err < 0) {
        goto execve_failed; // 拷贝参数失败, 释放页目录表
    }

    syscall_frame_t* frame = (syscall_frame_t*)(task->tss.esp0 - sizeof(syscall_frame_t)); // 获取当前任务的栈指针
    frame->eip = entry; // 设置新任务的入口地址
    frame->eax = frame->ebx = frame->ecx = frame->edx = frame->esi = frame->edi = 0; // 清空寄存器
    frame->esi = frame->edi = frame->ebp = 0; // 设置 ebp 寄存器
    frame->eflags = EFLAGS_IF | EFLAGS_DEFAULT; // 设置标志寄存器 
    frame->esp = stack_top - sizeof(uint32_t) * SYSCALL_PARAM_COUNT; // 设置栈指针, 栈顶地址

    task->tss.cr3 = new_page_dir; // 设置页目录表地址
    mmu_set_page_dir(new_page_dir); // 设置页目录表

    memory_destroy_uvm(old_page_dir); // 释放旧的页目录表

    return 0;

execve_failed:
    if (new_page_dir) {
        task->tss.cr3 = old_page_dir; // 恢复旧的页目录表地址
        mmu_set_page_dir(old_page_dir); // 设置页目录表

        memory_destroy_uvm(new_page_dir); // 释放新的页目录表
    }
    return -1;
}

