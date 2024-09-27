#include "os.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define PDE_P       (1 << 0) // 存在 分页机制
#define PDE_W       (1 << 1) // 可写位置一
#define PDE_U       (1 << 2) // 权限操作, 暂时置为一
#define PDE_PS      (1 << 7) // 使按照 4M 对 4M 的 映射

#define MAP_ADDR      0x80000000

// 如果 不 初始化 一个 0x36 之类的任意非0值, 则 在编译时会在 生成的 os.bin 中 没有空间, 从而导致加载到内存时 会使 数组 中的 值 随机, 导致一些需要为 0 的 位 不为0
uint8_t map_phy_buffer[4096] __attribute__((aligned(4096))) = { 0x36 };

// 二级 页表
static uint32_t page_table[1024] __attribute__((aligned(1024))) = { PDE_U };

// 设置 分页机制 的 页目录表
uint32_t pg_dir[1024] __attribute__((aligned(4096))) = {
    [0] = (0) | PDE_P | PDE_W | PDE_U | PDE_PS,
};

uint32_t task0_dpl3_stack[1024];

struct 
{
    uint16_t offset_l, selector, attr, offset_h;
} idt_table[256] __attribute__((aligned(8))); // 对于 IDT表 的 定义 (用于查找中断)

struct 
{
    uint16_t limit_l, base_l, basehl_attr, base_limit;
} gdt_table[256] __attribute__((aligned(8))) = {
    // 设置 为 基地址 为 0x0000, 范围为 4G, 相当于 16位 实模式下 段寄存器 的 平坦模型
    [KERNEL_CODE_SEG / 8] = {0xFFFF, 0x0000, 0x9A00, 0x00CF}, 
    [KERNEL_DATA_SEG / 8] = {0xFFFF, 0x0000, 0x9200, 0x00CF},

    [APP_CODE_SEG / 8] = {0xFFFF, 0x0000, 0xFA00, 0x00CF},  // 应用代码段, 设置 DPL 为 3, 即 内核态 无法访问
    [APP_DATA_SEG / 8] = {0xFFFF, 0x0000, 0xF300, 0x00CF},  // 应用数据段, 设置 DPL 为 3, 即 内核态 无法访问
};

void outb(uint8_t data, uint16_t port)
{
    __asm__ __volatile__ ("outb %[v], %[p]"::[p]"d"(port), [v]"a"(data));
}

void timer_int(void);

void os_init(void)
{
    // 初始化 8259 与 8253  定时器
    outb(0x11, 0x20);
    outb(0x11, 0xA0);
    outb(0x20, 0x21); // 设置 8259主片 在查 IDT表 时 通过 0x20 来 查找
    outb(0x28, 0xA1);
    outb(1 << 2, 0x21);
    outb(2, 0xA1);
    outb(0x1, 0x21);
    outb(0x1, 0xA1);
    outb(0xFE, 0x21);
    outb(0xFF, 0xA1);

    int tmo = 1193180 / 100; // 计算 100ms 触发的 时间
    outb(0x36, 0x43);
    outb((uint8_t)tmo, 0x40);
    outb(tmo >> 8, 0x40);

    // 给 IDT表 的 0x20 赋值
    idt_table[0x20].offset_l = (uint32_t)timer_int & 0xFFFF; // 给 低4位 赋值 (timer_int 为 中断处理函数)
    idt_table[0x20].offset_h = (uint32_t)timer_int >> 16;    // 高 16 位
    idt_table[0x20].selector = KERNEL_CODE_SEG;              // 设置 选择子 的 位置 为 代码段
    idt_table[0x20].attr = 0x8E00;                           // 设置 属性

    pg_dir[MAP_ADDR >> 22] = (uint32_t)page_table | PDE_P | PDE_W | PDE_U; // 将 MAP_ADDR 的 高 10 位 作为 页目录项 的 索引
    page_table[(MAP_ADDR >> 12) & 0x3FF] = (uint32_t)map_phy_buffer | PDE_P | PDE_W | PDE_U;
}

