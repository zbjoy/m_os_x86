#ifndef TTY_H
#define TTY_H

#include "kernel/include/ipc/sem.h"

#define TTY_NR 8 // 终端数量

#define TTY_OBUF_SIZE 512 // 输出缓冲区大小
#define TTY_IBUF_SIZE 512 // 输入缓冲区大小

typedef struct _tty_fifo_t {
    char* buf;
    int size; // 缓冲区大小
    int read, write; // 读写指针
    int count; // 当前缓冲区中的字符数
} tty_fifo_t;

#define TTY_OCRLF (1 << 0) // 输出回车换行标志

#define TTY_INCLR (1 << 0) // 输入清除标志
#define TTY_IECHO (1 << 1) // 输入回显标志

typedef struct _tty_t {
    char obuf[TTY_OBUF_SIZE]; // 输出缓冲区
    tty_fifo_t ofifo; // 输出 FIFO 缓存
    sem_t osem; // 输出信号量, 用于控制输出操作
    char ibuf[TTY_IBUF_SIZE]; // 输入缓冲区
    tty_fifo_t ififo;  // 输入 FIFO 缓存
    sem_t isem; // 输入信号量, 用于控制输入操作

    int iflags; // 输入标志, 用于控制输入行为 (如是否开启回车换行)
    int oflags; // 输出标志, 用于控制输出行为 (如是否开启回车换行)
    int console_idx; // 关联的控制台索引
} tty_t;

int tty_fifo_put(tty_fifo_t* fifo, char c); // 向 FIFO 缓存中写入字符
int tty_fifo_get(tty_fifo_t* fifo, char* c); // 从 FIFO 缓存中读取字符
void tty_in(int idx, char c); // 向指定 tty 输入字符

#endif
