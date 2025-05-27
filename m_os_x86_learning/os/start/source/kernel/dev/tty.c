#include "kernel/include/dev/tty.h"
#include "kernel/include/dev/dev.h"
#include "kernel/include/tools/log.h"
#include "kernel/include/dev/console.h"
#include "kernel/include/dev/kbd.h"

static tty_t tty_devs[TTY_NR]; // 终端设备数组

void tty_fifo_init(tty_fifo_t* fifo, char* buf, int size) {
    fifo->buf = buf;
    fifo->count = 0;
    fifo->size = size;
    fifo->read = fifo->write = 0;  // 读写指针初始化
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
    tty_fifo_init(&tty->ifofo, tty->ibuf, TTY_IBUF_SIZE); // 初始化输入缓冲区
    tty->console_idx = idx; // 记录当前控制台设备索引

    kbd_init(); // 初始化键盘
    console_init(idx); // 初始化控制台
    return 0;
}

int tty_read(device_t* dev, int addr, char* buf, int size) {
    return size;
}

int tty_write(device_t* dev, int addr, char* buf, int size) {
    return size;
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
