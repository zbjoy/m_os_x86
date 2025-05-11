#include "kernel/include/fs/fs.h"
#include "comm/types.h"
#include "kernel/include/tools/klib.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include <sys/stat.h>

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

int sys_open(const char* name, int flags, ...) {
    if (name[0] == '/') { // 说明是读取 shell.elf 文件
        read_disk(5000, 80, (uint8_t*)TEMP_ADDR);
        temp_pos = (uint8_t*)TEMP_ADDR; // 设置临时地址指针
        return TEMP_FILE_ID; // 返回临时文件ID
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
