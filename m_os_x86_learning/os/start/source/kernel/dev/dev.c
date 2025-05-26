#include "kernel/include/dev/dev.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/tools/klib.h"

#define DEV_TABLE_SIZE 128 // 设备表大小

extern dev_desc_t dev_tty_desc; // tty 设备描述符

// 设备描述符表 (用于设备的注册)
static dev_desc_t* dev_desc_tbl[] = {
    &dev_tty_desc, // 设备描述符表, 目前只有 tty 设备
};

// 用于存放特定的某个具体设备的信息
static device_t dev_tbl[DEV_TABLE_SIZE]; // 设备表

static int is_devid_bad(int dev_id) {
    if ((dev_id < 0) || (dev_id >= DEV_TABLE_SIZE)) {
        return 1; // 设备 id 越界
    }

    if (dev_tbl[dev_id].desc == (dev_desc_t*)0) {
        return 1; // 设备描述符为空
    }
}

// 设备操作函数
int dev_open(int major, int minor, void* data) { // 返回设备 id (dev_id)
    irq_state_t state = irq_enter_protection(); // 保护中断

    device_t* free_dev = (device_t*)0; // 空闲设备
    for (int i = 0; i < sizeof(dev_tbl) / sizeof(dev_tbl[0]); i++) { // 遍历设备表
        device_t* dev = dev_tbl + i; // 指向当前设备
        if (dev->open_count == 0) {
            free_dev = dev; // 找到空闲设备
        } else if ((dev->desc->major == major) && (dev->minor == minor)) {
            dev->open_count++; // 设备已打开, 增加打开计数
            irq_leave_protection(state); // 开放中断
            return i; // 返回设备 id
        }
    }

    dev_desc_t* desc = (dev_desc_t*)0; // 设备描述符
    for (int i = 0; i < sizeof(dev_desc_tbl) / sizeof(dev_desc_tbl[0]); i++) { // 遍历设备描述符表
        dev_desc_t* d = dev_desc_tbl[i]; // 指向当前设备描述符
        if (d->major == major) {
            desc = d; // 找到设备描述符
            break;
        }
    }

    if (desc && free_dev) {
        free_dev->minor = minor; // 绑定设备次设备号
        free_dev->data = data; // 绑定设备数据
        free_dev->desc = desc; // 绑定设备描述符

        int err = desc->open(free_dev); // 打开设备
        if (err == 0) {
            free_dev->open_count = 1; // 设备已打开, 打开计数为 1
            irq_leave_protection(state); // 开放中断
            return free_dev - dev_tbl; // 返回设备 id
        }
    }

    irq_leave_protection(state); // 开放中断
    return -1;
}

int dev_read(int dev_id, int addr, char* buf, int size) {
    if (is_devid_bad(dev_id)) {
        return -1;
    }

    device_t* dev = dev_tbl + dev_id; // 指向当前设备
    return dev->desc->read(dev, addr, buf, size); // 调用设备描述符的 read 函数
}

int dev_write(int dev_id, int addr, char* buf, int size) {
    if (is_devid_bad(dev_id)) {
        return -1;
    }

    device_t* dev = dev_tbl + dev_id; // 指向当前设备
    return dev->desc->write(dev, addr, buf, size); // 调用设备描述符的 write 函数
}

int dev_control(int dev_id, int cmd, int arg0, int arg1) {
    if (is_devid_bad(dev_id)) {
        return -1;
    }

    device_t* dev = dev_tbl + dev_id; // 指向当前设备
    return dev->desc->control(dev, cmd, arg0, arg1); // 调用设备描述符的 control 函数
}

void dev_close(int dev_id) {
    if (is_devid_bad(dev_id)) {
        return;
    }

    device_t* dev = dev_tbl + dev_id; // 指向当前设备
    irq_state_t state = irq_enter_protection(); // 保护中断
    if (--dev->open_count == 0) {
        dev->desc->close(dev); // 调用设备描述符的 close 函数
        kernel_memset(dev, 0, sizeof(device_t)); // 清空设备表项
    }
    irq_leave_protection(state); // 开放中断
}
