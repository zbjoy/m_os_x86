#ifndef MMU_H
#define MMU_H

#include "comm/types.h"
#include "comm/cpu_instr.h"

#define PDE_CNT 1024 // 页目录表项数

#define PTE_P (1 << 0) // 存在位置, 页表项有效
#define PDE_P (1 << 0) // 存在位置, 页目录项有效

#define PDE_W (1 << 1) // 可写位置, 页目录项可写
#define PTE_W (1 << 1) // 可写位置, 页表项可写

#define PDE_U (1 << 2) // 用户态位置, 页目录项用户态
#define PTE_U (1 << 2) // 用户态位置, 页表项用户态

typedef union _pde_t { // page directory entry (第一个表, 页目录表)
    uint32_t v;
    struct {
        uint32_t present : 1; // 是否存在
        uint32_t write_enable : 1; // 是否可写
        uint32_t user_mode_acc : 1; // 是否用户态
        uint32_t write_through : 1; // 是否写透
        uint32_t cache_disable : 1; // 是否缓存
        uint32_t accessed : 1; // 是否被访问过
        uint32_t  : 1; // 没有使用到, 用匿名的
        uint32_t ps : 1; // 是否为大页
        uint32_t  : 4; // 没有使用到, 用匿名的
        uint32_t phy_pt_addr : 20; // 物理页表地址
    };
} pde_t;

typedef union _pte_t { // page table entry (第二个表, 页表)
    uint32_t v;
    struct {
        uint32_t present : 1; // 是否存在
        uint32_t write_enable : 1; // 是否可写
        uint32_t user_mode_acc : 1; // 是否用户态
        uint32_t write_through : 1; // 是否写透
        uint32_t cache_disable : 1; // 是否缓存
        uint32_t accessed : 1; // 是否被访问过
        uint32_t dirty : 1; // 是否被修改过
        uint32_t pat : 1; // 是否支持PAT
        uint32_t global : 1; // 是否全局
        uint32_t  : 3; // 没有使用到, 用匿名的
        uint32_t phy_page_addr : 20; // 物理页地址
    };
} pte_t;

// pde_index: 页目录索引, 高 10 位
static inline uint32_t pde_index(uint32_t vaddr) {
    int index = (vaddr >> 22);
    return index;
}

// pte_index: 页表索引, 中间 10 位
static inline uint32_t pte_index(uint32_t vaddr) {
    int index = (vaddr >> 12) & 0x3FF; // 取中间 10 位, 将 高位 清零
    return index;
}

// 计算 pde 物理地址
static inline uint32_t pde_paddr(pde_t* pde) {
    return pde->phy_pt_addr << 12;
}

// 计算 pte 物理地址
static inline uint32_t pte_paddr(pte_t* pte) {
    return pte->phy_page_addr << 12;
}


// 设置页目录表地址
static inline void mmu_set_page_dir(uint32_t paddr) {
    write_cr3(paddr);
}

#endif
