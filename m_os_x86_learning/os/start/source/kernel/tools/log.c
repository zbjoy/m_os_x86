#include "kernel/include/tools/log.h"
#include "comm/cpu_instr.h"

#define COM1_PORT 0x3F8

void log_init(void) {
    outb(COM1_PORT + 1, 0x00); // 关掉串行接口内部的中断
    outb(COM1_PORT + 3, 0x80); // 设置 DLAB 为 1(与速度有关)
    outb(COM1_PORT + 0, 0x03); // 设置波特率为 38400
    outb(COM1_PORT + 1, 0x00); // 关闭 DLAB
    outb(COM1_PORT + 3, 0x03); // 开启 FIFO，设置 8 位数据位，1 位停止位
    outb(COM1_PORT + 2, 0xC7); // 设置输出格式为 8 位数据位，无校验位，1 位停止位
    outb(COM1_PORT + 4, 0x0F); // 开启 IR 接收器，允许发送数据

}

void log_printf(const char *fmt,...) {
    const char* p = fmt;
    while (*p != '\0') {
        while ((inb(COM1_PORT + 5) & (1 << 6)) == 0); // 等待空闲 (检查串行接口是否在忙)
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r'); // 输出回车符
    outb(COM1_PORT, '\n'); // 输出换行符
}   


