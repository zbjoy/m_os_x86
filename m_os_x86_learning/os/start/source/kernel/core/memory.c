#include "kernel/include/core/memory.h"
#include "kernel/include/tools/log.h"

static void addr_alloc_init(addr_alloc_t* alloc, uint8_t* bits, uint32_t start, uint32_t size, uint32_t page_size) {
    mutex_init(&alloc->mutex);
    alloc->start = start;
    alloc->size = size;
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bits, alloc->size / page_size, 0);     // 初始化位图, 0-清空
}

static uint32_t addr_alloc_page(addr_alloc_t* alloc, int page_count) {
    // uint32_t addr = 0;

    mutex_lock(&alloc->mutex);
    uint32_t addr = 0;

    int page_index = bitmap_alloc_nbits(&alloc->bitmap, 0, page_count); // 在位图中找连续page_count个为0的位, 并将这些位取反, 为了用于分配和回收对应的页
    if (page_index >= 0) {
        addr = alloc->start + page_index * alloc->page_size; // 计算分配到的物理地址
    }


    mutex_unlock(&alloc->mutex);
    return addr; // TODO: 此时mutex已经被解锁, 万一这个时候别的进程上锁进入, 给addr赋值了呢? 这个时候addr就不对了, 需要加锁吗?
    // 因为返回的是 addr, 而不是 alloc 的值, 所以不需要加锁
}

static void addr_free_page(addr_alloc_t* alloc, uint32_t addr, int page_count) {
    mutex_lock(&alloc->mutex);

    uint32_t pg_index = (addr - alloc->start) / alloc->page_size; // 计算页索引
    bitmap_set_bit(&alloc->bitmap, pg_index, page_count, 0); // 将分配的页标记为未分配

    mutex_unlock(&alloc->mutex);
}

void show_mem_info(boot_info_t* boot_info) {
    log_printf("memory region:\n");
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        log_printf("ram region %d: start=0x%x, size=0x%x", i, boot_info->ram_region_cfg[i].start, boot_info->ram_region_cfg[i].size);
    }
    log_printf("\n");
}

static uint32_t total_mem_size(boot_info_t* boot_info) {
    uint32_t mem_size = 0;
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        mem_size += boot_info->ram_region_cfg[i].size;
    }
    return mem_size;
}

void memory_init(boot_info_t* boot_info) {

    log_printf("memory_init\n");

    show_mem_info(boot_info); // 显示内存信息

    uint32_t mem_up1MB_free = total_mem_size(boot_info) - MEM_EXT_START; // 计算从1MB开始的空闲内存大小
    mem_up1MB_free = down2(mem_up1MB_free, MEM_PAGE_SIZE); // 向下取整到页大小的倍数


    /**
     * 以下都是测试代码
     */
    // addr_alloc_t addr_alloc;
    // uint8_t bits[8];

    // addr_alloc_init(&addr_alloc, bits, 0x1000, 64*4096, 4096); // 初始化物理内存分配器, 64MB的内存
    // for (int i = 0; i < 32; i++) {
    //     uint32_t addr = addr_alloc_page(&addr_alloc, 2);
    //     log_printf("alloc page %d at 0x%x\n", i, addr);
    // }

    // uint32_t addr = 0x1000;
    // for (int i = 0; i < 32; i++) {
    //     addr_free_page(&addr_alloc, addr, 2);
    //     addr += 8192; // 8192 = 2*4096, 即分配的页大小
    //     log_printf("free page %d at 0x%x\n", i, addr);
    // }
}
