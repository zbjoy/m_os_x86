#include "kernel/include/dev/tty.h"
#include "kernel/include/dev/dev.h"
#include "kernel/include/tools/log.h"
#include "kernel/include/dev/console.h"
#include "kernel/include/dev/kbd.h"

static tty_t tty_devs[TTY_NR]; // 终端设备数组

static tty_t* get_tty(device_t* dev) {
    int tty = dev->minor;
    if ((tty < 0) || (tty >= TTY_NR) || (!dev->open_count)) {
        log_printf("tty open failed. incorect tty num = %d", tty);
        return (tty_t*)0; // 错误的设备索引
    }

    return tty_devs + tty; // 返回 tty 设备
}

void tty_fifo_init(tty_fifo_t* fifo, char* buf, int size) {
    fifo->buf = buf;
    fifo->count = 0;
    fifo->size = size;
    fifo->read = fifo->write = 0;  // 读写指针初始化
}

int tty_fifo_put(tty_fifo_t* fifo, char c) {
    if (fifo->count >= fifo->size) {
        return -1; // 缓冲区已满
    }

    fifo->buf[fifo->write++] = c; // 写入字符
    if (fifo->write >= fifo->size) {
        fifo->write = 0;
    }
    fifo->count++; // 增加字符计数
    return 0; // 成功
}

int tty_fifo_get(tty_fifo_t* fifo, char* c) {
    if (fifo->count <= 0) {
        return -1; // 缓冲区为空
    }

    *c = fifo->buf[fifo->read++]; // 读取字符
    if (fifo->read >= fifo->size) {
        fifo->read = 0; // 循环读取
    }
    fifo->count--; // 减少字符计数
    return 0; // 成功
}

// tty 设备操作函数 (显示屏 与 键盘设备)
int tty_open(device_t* dev) {
    int idx = dev->minor;
    if (idx < 0 || idx >= TTY_NR) {
        log_printf("open tty failed. incorect tty num = %d", idx);
        return -1; // 错误的设备索引
    }

    tty_t* tty = tty_devs + idx; // 获取 tty 设备
    tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE); // 初始化输出缓冲区
    sem_init(&tty->osem, TTY_OBUF_SIZE); // 初始化输出信号量
    tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE); // 初始化输入缓冲区
    tty->oflags = TTY_OCRLF; // 设置输出标志, 开启回车换行
    tty->console_idx = idx; // 记录当前控制台设备索引

    kbd_init(); // 初始化键盘
    console_init(idx); // 初始化控制台
    return 0;
}

int tty_read(device_t* dev, int addr, char* buf, int size) {
    return size;
}

int tty_write(device_t* dev, int addr, char* buf, int size) {
    if (size < 0) {
        return -1;
    }
    tty_t* tty = get_tty(dev);
    if (!tty) {
        return -1;
    }

    int len = 0;
    while (size) {
        char c = *buf++;

        if ((c == '\n') && (tty->oflags & TTY_OCRLF)) { // 如果是换行符, 且开启了输出回车换行标志
            sem_wait(&tty->osem); // 等待输出信号量, 确保输出缓冲区有空间
            int err = tty_fifo_put(&tty->ofifo, '\r'); // 输出回车符
            if (err < 0) {
                break;
            }
        }

        sem_wait(&tty->osem); // 等待输出信号量, 确保输出缓冲区有空间

        int err = tty_fifo_put(&tty->ofifo, c);
        if (err < 0) {
            break;
        }
        len++;
        size--;
    
        // if () { // 判断显示器是否在忙
        //     continue;
        // } else { // 启动硬件发送显示
        // 
        // }

        console_write(tty);
    }
    // return size;
    return len;
}

int tty_control(device_t* dev, int cmd, int arg0, int arg1) {
    return 0;
}

void tty_close(device_t* dev) {

}

dev_desc_t dev_tty_desc = {
    .name = "tty",
    .major = DEV_TTY,
    .open = tty_open,
    .read = tty_read,
    .write = tty_write,
    .control = tty_control,
    .close = tty_close,
};
