#include "kernel/include/core/memory.h"
#include "kernel/include/tools/log.h"
#include "kernel/include/tools/klib.h"
#include "kernel/include/cpu/mmu.h"

static addr_alloc_t paddr_alloc;
static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE))); // 页目录表, 4KB, 1024个页目录项, 每个页目录项4字节, 4KB

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

pte_t* find_pte(pde_t* page_dir, uint32_t vaddr, int alloc) { // 找到虚拟地址对应的页表项, alloc 表示是否要分配表项
    pte_t* page_table;
    // 高 10 位 作为页目录索引, 低 12 位 作为页表索引
    pde_t* pde = page_dir + pde_index(vaddr); // 找到页目录项

    if (pde->present) {
        page_table = (pte_t*)pde_paddr(pde); // 找到页表的物理地址
    } else { // 不存在
        if (alloc == 0) {// 不分配
            return (pte_t*)0; 
        }
        uint32_t pg_paddr = addr_alloc_page(&paddr_alloc, 1); // 分配一个页表, 物理地址
        if (pg_paddr == 0) { // 分配失败
            return (pte_t*)0; // 分配失败, 返回空指针
        }

        pde->v = pg_paddr | PDE_P; // 设置页目录项的值, 物理地址 | 表明当前表项有效

        page_table = (pte_t*)pg_paddr; // 找到页表的物理地址
        kernel_memset(page_table, 0, MEM_PAGE_SIZE); // 页表清零
    }
    return page_table + pte_index(vaddr); // 找到页表项
}

int memory_create_map(pde_t* page_dir, uint32_t vaddr, uint32_t paddr, int count, uint32_t perm) {
    for (int i = 0; i < count; i++) {
        log_printf("create map: vaddr=0x%x, paddr=0x%x, count=%d, perm=0x%x\n", vaddr, paddr, count, perm); // 显示映射信息

        // 找到虚拟地址对应的页表项, 并分配表项
        pte_t* pte = find_pte(page_dir, vaddr, 1); // 找到虚拟地址对应的页表项, 1 表示要分配表项
        if (pte == (pte_t*)0) {
            log_printf("find_pte failed.pte == 0\n");
            return -1; // 找不到页表项, 说明内存不足
        }

        log_printf("pte=0x%x\n", (uint32_t)pte); // 显示页表项的值
        ASSERT(pte->present == 0); // 确保该页表项不存在
        pte->v = paddr | perm | PDE_P | PDE_W | PDE_U; // 设置页表项的值, 物理地址 | 权限 | 表明当前表项有效

        vaddr += MEM_PAGE_SIZE; // 指向下一个虚拟地址
        paddr += MEM_PAGE_SIZE; // 指向下一个物理地址
    }
}

void create_kernel_table(void) {
    extern uint8_t s_text[], e_text[], s_data[]; // 定义在 source/kernel/kernel.lds 文件中, 通过关键字 PROVIDE 定义 让在 C语言 中可以引用, 指示了内核代码的起始地址
    extern uint8_t kernel_base[]; // 定义在 source/kernel/kernel.lds 文件中, 通过关键字 PROVIDE 定义 让在 C语言 中可以引用, 指示了内核代码的开始地址
    static memory_map_t kernel_map[] = {
        {kernel_base, s_text, kernel_base, PTE_W},// 64KB 之前的数据
        {s_text, e_text, s_text, 0}, // 让虚拟d址和物理地址相同, 也就是内核代码段的起始地址
        {s_data, (void*)MEM_EBDA_START, s_data, PTE_W} // 让虚拟地址和物理地址相同, 也就是内核数据段的起始地址
    };

    for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++) {
        memory_map_t* map = kernel_map + i;
    
        uint32_t vstart = down2((uint32_t)map->vstart, MEM_PAGE_SIZE); // 向下取整到4KB的倍数
        uint32_t vend = up2((uint32_t)map->vend, MEM_PAGE_SIZE); // 向下取整到4KB的倍数

        uint32_t pstart = down2((uint32_t)map->pstart, MEM_PAGE_SIZE); // 向下取整到4KB的倍数

        // 计算使用了多少内存页
        int page_count = (vend - vstart) / MEM_PAGE_SIZE;

        // TODO: 将虚拟地址和物理地址映射到内存表中

        memory_create_map(kernel_page_dir, vstart, (uint32_t)pstart, page_count, map->perm); // 建立一个映射, 将 vstart 的虚拟地址映射到 pstart 的物理地址, 页数为 page_count, 权限为 map->perm
    }
}

static uint32_t total_mem_size(boot_info_t* boot_info) {
    uint32_t mem_size = 0;
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        mem_size += boot_info->ram_region_cfg[i].size;
    }
    return mem_size;
}



void memory_init(boot_info_t* boot_info) {
    extern uint8_t* mem_free_start; // 定义在 source/kernel/kernel.lds 文件中, 通过关键字 PROVIDE 定义 让在 C语言 中可以引用, 指示了空闲内存的起始地址
    uint8_t* mem_fre = mem_free_start; // 空闲内存起始地址, 因为 mem_free_start 不能直接更改, 所以这里定义一个指针指向它

    log_printf("memory_init\n");

    show_mem_info(boot_info); // 显示内存信息

    uint32_t mem_up1MB_free = total_mem_size(boot_info) - MEM_EXT_START; // 计算从1MB开始的空闲内存大小
    mem_up1MB_free = down2(mem_up1MB_free, MEM_PAGE_SIZE); // 向下取整到页大小的倍数

    log_printf("free memory: 0x%x, size: 0x%x\n", MEM_EXT_START, mem_up1MB_free); // 显示空闲内存信息

    // 对1MB以下的内存进行管理
    addr_alloc_init(&paddr_alloc, mem_fre, MEM_EXT_START, mem_up1MB_free, MEM_PAGE_SIZE); // 初始化物理内存分配器, 1MB以下的内存
    mem_fre += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE); // 更新空闲内存起始地址, bitmap_byte_count 计算位图的字节数

    ASSERT(mem_fre < (uint8_t*)MEM_EBDA_START); // 确保空闲内存起始地址小于 EBDA 起始地址
    create_kernel_table();

    // 将 kernel_page_dir 映射到内存表中 (放到 cr3 寄存器 中)
    mmu_set_page_dir((uint32_t)kernel_page_dir); // 将页目录表映射到 cr3 寄存器中


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
    //     log_printf("free page %d at 0x%x\n", i, addr);
    //     addr += 8192; // 8192 = 2*4096, 即分配的页大小
    // }
}
