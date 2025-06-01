#include "kernel/include/fs/fs.h"
#include "comm/types.h"
#include "kernel/include/tools/klib.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include <sys/stat.h>
#include "kernel/include/dev/console.h"
#include "kernel/include/fs/file.h"
#include "kernel/include/core/task.h"
#include "kernel/include/dev/dev.h"
#include "kernel/include/tools/log.h"

// #define TEMP_ADDR (8 * 1024 * 1024) // 8MB
static uint8_t TEMP_ADDR[100 * 1024]; // 100KB
static uint8_t* temp_pos;
#define TEMP_FILE_ID 100

static void read_disk(uint32_t sector, uint32_t sector_count, uint8_t *buf)
{
    outb(0x1F6, 0xE0);
    outb(0x1F2, (uint8_t)(sector_count >> 8));
    outb(0x1F3, (uint8_t)(sector_count >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    outb(0x1F2, (uint8_t)sector_count);
    outb(0x1F3, (uint8_t)(sector));
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));

    outb(0x1F7, 0x24);

    uint16_t* data_buf = (uint16_t*)buf;
    while (sector_count--)
    {
        while ((inb(0x1F7) & 0x88) != 0x8) {};
        for (int i = 0; i < SECTOR_SIZE / 2; i++)
        {
            *data_buf++ = inw(0x1F0);
        }
    }
}

static is_path_valid(const char* path) {
    if ((path == (const char*)0) || (path[0] == '\0')) {
        return 0; // 空路径不合法
    }
    return 1;
}

int sys_open(const char* name, int flags, ...) {
    if (kernel_strncmp(name, "tty", 3) == 0) {
        if (!is_path_valid(name)) {
            log_printf("sys_open: path is valid");
            return -1; // 路径不合法
        }

        int fd = -1;
        file_t* file = file_alloc(); // 分配文件结构
        if (file) {
            fd = task_alloc_fd(file); // 分配文件描述符
            if (fd < 0) {
                goto sys_open_failed; 
            }
        } else {
            goto sys_open_failed;
        }

        if (kernel_strlen(name) < 5) {
            goto sys_open_failed; // 路径长度不合法
        }
        int num = name[4] - '0';
        int dev_id = dev_open(DEV_TTY, num, 0); // 打开设备
        if (dev_id < 0) {
            goto sys_open_failed; // 打开设备失败
        }

        file->dev_id = dev_id;
        file->mode = 0; // TODO: 后续可能会调整成 flags 的值
        file->pos = 0; // 文件指针置零
        file->ref = 1; // 引用计数置为 1
        file->type = FILE_TTY; // 文件类型为普通文件
        kernel_strncpy(file->file_name, name, FILE_NAME_SIZE); // 设置文件名
        return fd;
sys_open_failed:
        if (file) {
            file_free(file);
        }

        if (fd >= 0) {
            task_remove_fd(fd);
        }
        return -1; // 打开失败
    } else {
        if (name[0] == '/') { // 说明是读取 shell.elf 文件
            read_disk(5000, 80, (uint8_t *)TEMP_ADDR);
            temp_pos = (uint8_t *)TEMP_ADDR; // 设置临时地址指针
            return TEMP_FILE_ID;             // 返回临时文件ID
        }
    }

    return -1; // 其他文件暂不支持
}

int sys_read(int file, char* ptr, int len) {
    if (file == TEMP_FILE_ID) { // 说明是读取临时文件
        kernel_memcpy(ptr, temp_pos, len); // 复制数据到用户空间
        temp_pos += len; // 更新临时地址指针
        return len; // 返回读取长度
    }
    return -1; // 其他文件暂不支持
}

int sys_write(int file, char* ptr, int len) {
    if (file == 1) {
        // 这里不再进行串口输出, 改为控制台输出
        // ptr[len] = '\0';       // 确保字符串以 null 结尾
        // log_printf("%s", ptr); // 打印到日志
        // console_write(0, ptr, len); // 写入控制台, 目前只有一个控制台, 所以第一个参数直接写0
        ptr[len] = '\0';       // 确保字符串以 null 结尾
        log_printf("%s", ptr); // 打印到日志
    }
    return -1; // 其他文件暂不支持
}

int sys_lseek(int file, int ptr, int dir) {
    if (file == TEMP_FILE_ID) {
        temp_pos = (uint8_t*)(TEMP_ADDR + ptr); // 设置临时地址指针
        return 0; // 返回成功
    }
    return -1; // 其他文件暂不支持
}

int sys_close(int file) {
    return 0; // 成功关闭文件
}

int sys_isatty(int file) {
    return -1;
}


int sys_fstat(int file, struct stat* st) {
    return -1;
}

void fs_init(void) {
    file_table_init(); // 初始化文件表
}
