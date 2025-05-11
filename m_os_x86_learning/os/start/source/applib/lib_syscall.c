#include "lib_syscall.h"


static inline int sys_call(syscall_args_t *args) {
    // TODO: 通过内联汇编实现系统调用
    uint32_t addr[] = {0, SELECTOR_SYSCALL | 0};
    int ret;

    // // 通过调用门实现系统调用 (使用 int $0x80 指令)
    // __asm__ __volatile__(
    //     "int $0x80"
    //     :"=a"(ret)
    //     :"S"(args->arg3), "d"(args->arg2), "c"(args->arg1),
    //     "b"(args->arg0), "a"(args->id)
    // );

    __asm__ __volatile__(
        "push %[arg3]\n\t"
        "push %[arg2]\n\t"
        "push %[arg1]\n\t"
        "push %[arg0]\n\t"
        "push %[id]\n\t"
        "lcalll *(%[a])"
        :"=a"(ret):
        [arg3]"r"(args->arg3), [arg2]"r"(args->arg2), [arg1]"r"(args->arg1), [arg0]"r"(args->arg0), [id]"r"(args->id), [a]"r"(addr) // 这里的 addr 是一个指针，指向一个数组 
    );

    return ret;
}

void ms_sleep(int ms) {
    if (ms <= 0) {
        return;
    }

    // 通过调用门实现系统调用
    syscall_args_t args;
    args.id = SYS_sleep;
    args.arg0 = ms;
    sys_call(&args); // 参数的数量
}

int getpid(void) {
    syscall_args_t args;
    args.id = SYS_getpid;

    return sys_call(&args); // 参数的数量
}

void print_msg(const char* fmt, int arg) {
    syscall_args_t args;
    args.id = SYS_printmsg;
    args.arg0 = (int)fmt; // 将参数转换为整数类型
    args.arg1 = arg; // 将参数转换为整数类型

    sys_call(&args); // 参数的数量
}

int fork() {
    syscall_args_t args;
    args.id = SYS_fork; // fork 的系统调用 ID
    return sys_call(&args); // 参数的数量
}

int execve(const char* name, char* const* argv, char* const* env) {
    syscall_args_t args;
    args.id = SYS_execve;
    args.arg0 = (int)name; // 将参数转换为整数类型
    args.arg1 = (int)argv; // 将参数转换为整数类型
    args.arg2 = (int)env; // 将参数转换为整数类型

    return sys_call(&args); // 参数的数量
}

int yield(void) {
    syscall_args_t args;
    args.id = SYS_yield; // fork 的系统调用 ID

    return sys_call(&args); // 参数的数量
}


int open(const char* name, int flags, ...) {
    syscall_args_t args;
    args.id = SYS_open;
    args.arg0 = (int)name;
    args.arg1 = (int)flags;

    return sys_call(&args); // 参数的数量
}

int read(int file, char* ptr, int len){
    syscall_args_t args;
    args.id = SYS_read;
    args.arg0 = file;
    args.arg1 = (int)ptr;
    args.arg2 = len;

    return sys_call(&args); // 参数的数量
}

int write(int file, char* ptr, int len) {
    syscall_args_t args;
    args.id = SYS_write;
    args.arg0 = file;
    args.arg1 = (int)ptr;
    args.arg2 = len;

    return sys_call(&args); // 参数的数量
}

int close(int file){
    syscall_args_t args;
    args.id = SYS_close;
    args.arg0 = file;

    return sys_call(&args); // 参数的数量
}

int lseek(int file, int ptr, int dir){
    syscall_args_t args;
    args.id = SYS_lseek;
    args.arg0 = file;
    args.arg1 = ptr;
    args.arg2 = dir;

    return sys_call(&args); // 参数的数量
}


int isatty(int file){
    syscall_args_t args;
    args.id = SYS_isatty;
    args.arg0 = file;

    return sys_call(&args); // 参数的数量
}

int fstat(int file, struct stat* st){
    syscall_args_t args;
    args.id = SYS_fstat;
    args.arg0 = file;
    args.arg1 = (int)st;

    return sys_call(&args); // 参数的数量
}

void* sbrk(ptrdiff_t incr){
    syscall_args_t args;
    args.id = SYS_sbrk;
    args.arg0 = incr;

    return (void*)sys_call(&args); // 参数的数量
}

