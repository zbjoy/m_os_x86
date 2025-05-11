#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "comm/types.h"
#include "kernel/include/core/syscall.h"
#include "kernel/include/os_cfg.h"
#include <sys/stat.h>

typedef struct _syscall_args_t {
    int id;
    int arg0;
    int arg1;
    int arg2;
    int arg3;
} syscall_args_t;

// #define SYS_sleep 0 // 放在了 syscall.h 中


void ms_sleep(int ms);
int getpid(void);
void print_msg(const char* fmt, int arg);
int fork();
int execve(const char* name, char* const* argv, char* const* env);
int yield(void);


int open(const char* name, int flags, ...);
int read(int file, char* ptr, int len);
int write(int file, char* ptr, int len);
int close(int file);
int lseek(int file, int ptr, int dir);

int isatty(int file);
int fstat(int file, struct stat* st);
void* sbrk(ptrdiff_t incr);

#endif
