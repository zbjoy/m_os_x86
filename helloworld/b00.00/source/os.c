#include "os.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define PDE_P       (1 << 0) // 存在 分页机制
#define PDE_W       (1 << 1) // 可写位置一
#define PDE_U       (1 << 2) // 权限操作, 暂时置为一
#define PDE_PS      (1 << 7) // 使按照 4M 对 4M 的 映射

#define MAP_ADDR      0x80000000

uint8_t map_phy_buffer[4096] __attribute__((aligned(4096))) = { 0x36 };

// 二级 页表
static uint32_t page_table[1024] __attribute__((aligned(1024))) = { PDE_U };

// 设置 分页机制 的 页目录表
uint32_t pg_dir[1024] __attribute__((aligned(4096))) = {
    [0] = (0) | PDE_P | PDE_W | PDE_U | PDE_PS,
};

struct 
{
    uint16_t limit_l, base_l, basehl_attr, base_limit;
} gdt_table[256] __attribute__((aligned(8))) = {
    // 设置 为 基地址 为 0x0000, 范围为 4G, 相当于 16位 实模式下 段寄存器 的 平坦模型
    [KERNEL_CODE_SEG / 8] = {0xFFFF, 0x0000, 0x9A00, 0x00CF}, 
    [KERNEL_DATA_SEG / 8] = {0xFFFF, 0x0000, 0x9200, 0x00CF},
};

void os_init(void)
{
    pg_dir[MAP_ADDR >> 22] = (uint32_t)page_table | PDE_P | PDE_W | PDE_U; // 将 MAP_ADDR 的 高 10 位 作为 页目录项 的 索引
    page_table[MAP_ADDR >> 12 & 0x3FF] = (uint32_t)map_phy_buffer | PDE_P | PDE_W | PDE_U;
}
