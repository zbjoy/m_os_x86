#include <stdarg.h>
#include "kernel/include/tools/log.h"
#include "comm/cpu_instr.h"
#include "kernel/include/tools/klib.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/ipc/mutex.h"
#include "kernel/include/dev/console.h"
#include "kernel/include/dev/dev.h"

static mutex_t mutex;

#define LOG_USE_COM 0
#define COM1_PORT 0x3F8

static int log_dev_id;

void log_init(void) {
    mutex_init(&mutex);

    log_dev_id = dev_open(DEV_TTY, 0, (void*)0); // 打开 tty 设备

#if LOG_USE_COM
    outb(COM1_PORT + 1, 0x00); // 关掉串行接口内部的中断
    outb(COM1_PORT + 3, 0x80); // 设置 DLAB 为 1(与速度有关)
    outb(COM1_PORT + 0, 0x03); // 设置波特率为 38400
    outb(COM1_PORT + 1, 0x00); // 关闭 DLAB
    outb(COM1_PORT + 3, 0x03); // 开启 FIFO，设置 8 位数据位，1 位停止位
    outb(COM1_PORT + 2, 0xC7); // 设置输出格式为 8 位数据位，无校验位，1 位停止位
    outb(COM1_PORT + 4, 0x0F); // 开启 IR 接收器，允许发送数据
#endif

}

void log_printf(const char *fmt,...) {
    char str_buf[128];

    va_list args;

    kernel_memset(str_buf, '\0', sizeof(str_buf));
    va_start(args, fmt);
    kernel_vsprintf(str_buf, fmt, args);
    va_end(args); // 释放资源

    // 进入临界区, 禁止中断
    // irq_state_t state = irq_enter_protection();
    mutex_lock(&mutex);

#if LOG_USE_COM
    const char* p = str_buf;
    while (*p != '\0') {
        while ((inb(COM1_PORT + 5) & (1 << 6)) == 0); // 等待空闲 (检查串行接口是否在忙)
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r'); // 输出回车符
    outb(COM1_PORT, '\n'); // 输出换行符
#else 
    // console_write(0, str_buf, kernel_strlen(str_buf)); // 直接输出到控制台
    dev_write(log_dev_id, 0, str_buf, kernel_strlen(str_buf)); // 写入日志设备
    char ch = '\n';
    // console_write(0, &ch, 1); // 输出换行符
    dev_write(log_dev_id, 0, &ch, 1); // 写入日志设备
#endif

    // 退出临界区, 恢复中断
    // irq_leave_protection(state);
    mutex_unlock(&mutex);
}   



