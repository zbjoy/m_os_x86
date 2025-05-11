#include "kernel/include/core/syscall.h"
#include "kernel/include/core/task.h"
#include "kernel/include/tools/log.h"
#include "kernel/include/core/memory.h"
#include "kernel/include/fs/fs.h"

typedef int (*sys_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

void sys_print_msg(char* fmt, int arg) {
    log_printf(fmt, arg);
}

static const sys_handler_t sys_table[] = {
    [SYS_sleep] = (sys_handler_t)sys_sleep,
    [SYS_getpid] = (sys_handler_t)sys_getpid,
    [SYS_fork] = (sys_handler_t)sys_fork,
    [SYS_printmsg] = (sys_handler_t)sys_print_msg,
    [SYS_execve] = (sys_handler_t)sys_execve,
    [SYS_yield] = (sys_handler_t)sys_sched_yield,
    [SYS_open] = (sys_handler_t)sys_open,
    [SYS_read] = (sys_handler_t)sys_read,
    [SYS_write] = (sys_handler_t)sys_write,
    [SYS_close] = (sys_handler_t)sys_close,
    [SYS_lseek] = (sys_handler_t)sys_lseek,
    [SYS_isatty] = (sys_handler_t)sys_isatty,
    [SYS_sbrk] = (sys_handler_t)sys_sbrk,
    [SYS_fstat] = (sys_handler_t)sys_fstat,
};
void do_handler_syscall(syscall_frame_t* frame) {
    if (frame->func_id < sizeof(sys_table) / sizeof(sys_table[0])) {
        sys_handler_t handler = sys_table[frame->func_id];
        if (handler) {
            int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
            frame->eax = ret; // 将返回值放到 eax 中
            return;
        } 
    }

    task_t* task = task_current();
    log_printf("task: %s, Unknown syscall: %d", task->name, frame->func_id);
    frame->eax = -1; // 返回错误
}
