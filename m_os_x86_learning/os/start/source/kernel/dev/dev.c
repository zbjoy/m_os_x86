#include "kernel/include/dev/dev.h"

extern dev_desc_t dev_tty_desc; // tty 设备描述符

// 设备操作函数
int dev_open(int major, int minor, void* data) { // 返回设备 id (dev_id)
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
