#ifndef CPU_H
#define CPU_H

#include "comm/types.h"

#pragma pack(1)

typedef struct _segment_desc_t {
    uint16_t limit15_0;
    uint16_t base15_0;
    uint8_t base23_16;
    uint16_t attribute;
    uint8_t base31_24;
} segment_desc_t;

typedef struct _gate_desc_t {
    uint16_t offset15_0;
    uint16_t selector;
    uint16_t attr;
    uint16_t offset31_16;
} gate_desc_t;

#define GATE_P_PRESENT (1 << 15)
#define GATE_DPL0 (0 << 13)
#define GATE_DPL3 (3 << 13)
#define GATE_TYPE_INT (0xE << 8)

#pragma pack()

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);
void gate_desc_set(gate_desc_t* desc, uint16_t selector, uint32_t offset, uint16_t attr);

void cpu_init(void);



// 定义 GDT 表项对应的 属性
#define SEG_G (1 << 15)         // 对应 limit 的 单位 (set : 4KB)
#define SEG_D (1 << 14)         // D/B 控制表项 代码段, 栈... 为 32 位 还是 16 位

/* 中间一些属性控制 64 位 的, 忽略 */

#define SEG_P_PRESENT (1 << 7)  // 用于控制当前 描述符 是否 存在

#define SEG_DPL0 (0 << 5)       // DPL 0, 最高权限
#define SEG_DPL3 (3 << 5)       // DPL 3, 最低权限

#define SEG_S_SYSTEM (0 << 4)   // set 0 则为 系统段
#define SEG_S_NORMAL (1 << 4)   // set 1 则为 数据段 

#define SEG_TYPE_CODE (1 << 3)  // 代码段则 set 1
#define SEG_TYPE_DATA (0 << 3)  // 数据段(可以给 变量或者栈 去使用)

#define SEG_TYPE_RW (1 << 1)    // 控制读写权限

#endif
