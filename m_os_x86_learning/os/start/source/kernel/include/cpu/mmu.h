#ifndef MMU_H
#define MMU_H

#include "comm/types.h"

#define PDE_CNT 1024 // 页目录表项数

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

#endif
