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

#define MEM_TASK_STACK_TOP 0xE0000000 // 栈顶地址
#define MEM_TASK_STACK_SIZE (MEM_PAGE_SIZE * 500) // 栈大小
#define MEM_TASK_ARG_SIZE (MEM_PAGE_SIZE * 4) // 参数大小, 4KB

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
int memory_alloc_for_page_dir(uint32_t page_dir, uint32_t vaddr, uint32_t size, uint32_t perm); // 为指定页目录表分配内存
int memory_alloc_page_for(uint32_t addr, uint32_t size, uint32_t perm); // 为指定进程分配内存

uint32_t memory_alloc_page(void); // 分配一个页物理内存, 返回物理地址
void memory_free_page(uint32_t addr); // 释放一个页物理内存

uint32_t memory_copy_uvm(uint32_t page_dir);
void memory_destroy_uvm(uint32_t page_dir);
uint32_t memory_get_paddr(uint32_t page_dir, uint32_t vaddr); // 返回 page_dir 页目录表中 vaddr 虚拟地址对应的物理地址
int memory_copy_uvm_data(uint32_t to, uint32_t page_dir, uint32_t from, uint32_t size); // 拷贝数据到新的页物理内存中去
char* sys_sbrk(int incr); // 分配内存

#endif
