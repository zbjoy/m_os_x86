__asm__(".code16gcc");

#include "boot.h"

// loader的起始地址
#define LOADER_START_ADDR 0x8000

/**
 * Loader的C入口函数
 * 只完成一项功能，即从磁盘找到loader文件然后加载到内容中，并跳转过去
 */
void boot_entry(void) {
    // 转化为函数并调用
    ((void (*)(void))LOADER_START_ADDR)();
} 

