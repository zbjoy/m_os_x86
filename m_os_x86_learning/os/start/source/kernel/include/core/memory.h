#ifndef MEMORY_H
#define MEMORY_H

#include "comm/types.h"
#include "kernel/include/tools/bitmap.h"
#include "kernel/include/ipc/mutex.h"
#include "comm/boot_info.h"

#define MEM_EBDA_START (0x00080000) // EBDA 起始地址
#define MEM_EXT_START (1024 * 1024) // 1MB
#define MEM_EXT_END (127 * 1024 * 1024) // 127MB
#define MEM_PAGE_SIZE (4096) // 4KB
#define MEMORY_TASK_BASE (0x80000000) // 2GB (进程虚拟地址的起始位置)

typedef struct _addr_alloc_t {
    mutex_t mutex;
    bitmap_t bitmap;
    
    uint32_t start;
    uint32_t size;
    uint32_t page_size;
} addr_alloc_t;

// 内存映射 (为 .data .bss .text 等分配内存)
typedef struct _memory_map_t {
    void* vstart; // 线性地址的起始地址
    void* vend; // 线性地址的结束地址
    void* pstart; // 物理地址的起始地址
    uint32_t perm; // 权限
} memory_map_t;

void memory_init(boot_info_t* boot_info);

uint32_t memory_create_uvm(void); // 创建一个页表, 返回页目录表的物理地址

#endif
