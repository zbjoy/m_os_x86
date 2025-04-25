#include "kernel/include/cpu/cpu.h"
#include "kernel/include/os_cfg.h"
#include "comm/cpu_instr.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/ipc/mutex.h"

static segment_desc_t gdt_table[GDT_TABLE_SIZE];
static mutex_t mutex;

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr)
{
    // 下面代码相当于: segment_desc_t* desc = gdt_table[selector >> 3]
    // segment_desc_t* desc = gdt_table + selector / sizeof(segment_desc_t); // 因为 gdt_table 是 8 字节对齐的，所以 selector >> 3 得到的偏移量是 32 字节对齐的
    segment_desc_t* desc = gdt_table + (selector >> 3); // 因为 gdt_table 是 8 字节对齐的，所以 selector >> 3 得到的偏移量是 32 字节对齐的

    if (limit > 0xFFFFF) // 大于 20 位, 则将 G 标志 置为 1, 使 limit 单位为 4KB
    {
        attr |= 0x8000;
        // attr |= SEG_G;
        limit /= 0x1000;    // limit 除以 4KB
    }

    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->base31_24 = (base >> 24) & 0xFF;
    desc->attribute = attr | ((limit >> 16 & 0xF) << 8);
}

void gate_desc_set(gate_desc_t* desc, uint16_t selector, uint32_t offset, uint16_t attr)
{
    desc->offset15_0 = offset & 0xFFFF;
    desc->offset31_16 = (offset >> 16) & 0xFFFF;
    desc->selector = selector;
    desc->attr = attr;
}

int gdt_alloc_desc() {
    // 进入临界区
    // irq_state_t state = irq_enter_protection();
    mutex_lock(&mutex);

    // 找到空闲的段描述符
    for (int i = 1; i < GDT_TABLE_SIZE; i++) {
        segment_desc_t* desc = gdt_table + i;
        if (desc->attribute == 0) {
            // irq_leave_protection(state);
            mutex_unlock(&mutex);
            return i * sizeof(segment_desc_t);
        }
    }

    // 退出临界区
    // irq_leave_protection(state);
    mutex_unlock(&mutex);
    return -1;
}


void gdt_free_sel(int sel) {
    // 进入临界区
    // irq_state_t state = irq_enter_protection();
    mutex_lock(&mutex);

    // 释放段描述符
    gdt_table[sel / sizeof(segment_desc_t)].attribute = 0;

    // 退出临界区
    // irq_leave_protection(state);
    mutex_unlock(&mutex);
}



// TODO: 添加调用门描述符的设置函数
void init_gdt(void)
{
    // 清空 gdt 表
    for (int i = 0; i < GDT_TABLE_SIZE; i++)
    {
        // 下面一句相当于 segment_desc_set(i * sizeof(segment_desc_t), 0, 0, 0);
        segment_desc_set(i << 3, 0, 0, 0); // 左移 3 位是为了得到段描述符的偏移量, 一个 segment_desc_t 占 8 字节
    }

    // 重新加载 gdt 表 (普通平坦模型)
    segment_desc_set(KERNEL_SELECTOR_CS, 0, 0xFFFFFFFF, 
                    SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW | SEG_D); 
    segment_desc_set(KERNEL_SELECTOR_DS, 0, 0xFFFFFFFF, 
                    SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW | SEG_D); 
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}

void cpu_init(void)
{
    mutex_init(&mutex);
    init_gdt();

}

void switch_to_tss(uint32_t tss_sel) {
    far_jump(tss_sel, 0);
}
