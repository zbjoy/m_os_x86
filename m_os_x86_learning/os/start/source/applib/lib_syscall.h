#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "comm/types.h"
#include "kernel/include/core/syscall.h"
#include "kernel/include/os_cfg.h"

typedef struct _syscall_args_t {
    int id;
    int arg0;
    int arg1;
    int arg2;
    int arg3;
} syscall_args_t;

#define SYS_sleep 0

static inline int sys_call(syscall_args_t *args) {
    // TODO: 通过内联汇编实现系统调用
    uint32_t addr[] = {0, SELECTOR_SYSCALL | 0};

    __asm__ __volatile__(
        "push %[arg3]\n\t"
        "push %[arg2]\n\t"
        "push %[arg1]\n\t"
        "push %[arg0]\n\t"
        "push %[id]\n\t"
        "lcalll *(%[a])"::
        [arg3]"r"(args->arg3), [arg2]"r"(args->arg2), [arg1]"r"(args->arg1), [arg0]"r"(args->arg0), [id]"r"(args->id), [a]"r"(addr) // 这里的 addr 是一个指针，指向一个数组 
    );
}

static inline void ms_sleep(int ms) {
    if (ms <= 0) {
        return;
    }

    // 通过调用门实现系统调用
    syscall_args_t args;
    args.id = SYS_sleep;
    args.arg0 = ms;
    sys_call(&args); // 参数的数量
}


#endif
