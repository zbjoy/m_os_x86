#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

static inline void ms_sleep(int ms) {
    if (ms <= 0) {
        return;
    }

    // 通过调用门实现系统调用
}


#endif
