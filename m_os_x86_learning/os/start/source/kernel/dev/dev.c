#include "kernel/include/dev/dev.h"
#include "kernel/include/cpu/irq.h"

#define DEV_TABLE_SIZE 128 // 设备表大小

extern dev_desc_t dev_tty_desc; // tty 设备描述符

// 设备描述符表 (用于设备的注册)
static dev_desc_t* dev_desc_tbl[] = {
    &dev_tty_desc, // 设备描述符表, 目前只有 tty 设备
};

// 用于存放特定的某个具体设备的信息
static device_t dev_tbl[DEV_TABLE_SIZE]; // 设备表

// 设备操作函数
int dev_open(int major, int minor, void* data) { // 返回设备 id (dev_id)
    irq_state_t state = irq_enter_protection(); // 保护中断

    

    irq_leave_protection(state); // 开放中断
    return -1;
}

int dev_read(int dev_id, int addr, char* buf, int size) {
    return size;
}

int dev_write(int dev_id, int addr, char* buf, int size) {
    return size;
}

int dev_control(int dev_id, int cmd, int arg0, int arg1) {
    return 0;
}

void dev_close(int dev_id) {
    return;
}
