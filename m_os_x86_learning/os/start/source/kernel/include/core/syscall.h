#ifndef SYSCALL_H
#define SYSCALL_H

#include "comm/types.h"

#define SYS_sleep 0 // 系统调用的 ID
#define SYS_getpid 1 // 系统调用的 ID

#define SYSCALL_PARAM_COUNT 5

typedef struct _syscall_frame_t {
    int eflags; // 标志寄存器
    int gs, fs, es, ds; // 数据段寄存器
    uint32_t edi, esi, ebp, dummy, ebx, edx, ecx, eax; // 通用寄存器
    int eip, cs;
    int func_id, arg0, arg1, arg2, arg3; // 系统调用参数
    int esp, ss; // 栈指针和栈段寄存器
} syscall_frame_t;

void exception_handler_syscall(void);

#endif
