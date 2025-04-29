#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

typedef struct _syscall_args_t {
    int id;
    int arg0;
    int arg1;
    int arg2;
    int arg3;
} syscall_args_t;

#define SYS_sleep 0

static inline int sys_call(syscall_args_t *args) {
    
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
