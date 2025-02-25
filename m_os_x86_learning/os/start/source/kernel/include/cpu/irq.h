#ifndef IRQ_H
#define IRQ_H

#include "comm/types.h"

#define IRQ0_DE 0
#define IRQ1_DB 1
#define IRQ2_NMI 2
#define IRQ3_BP 3
#define IRQ4_OF 4
#define IRQ5_BR 5
#define IRQ6_UD 6
#define IRQ7_NM 7
#define IRQ8_DF 8
#define IRQ10_TS 10
#define IRQ11_NP 11
#define IRQ12_SS 12
#define IRQ13_GP 13
#define IRQ14_PF 14
#define IRQ16_MF 16
#define IRQ17_AC 17
#define IRQ18_MC 18
#define IRQ19_XM 19
#define IRQ20_VE 20
#define IRQ21_CP 21

#define IRQ0_TIMER 0x20

// PIC控制器相关的寄存器及位配置
#define PIC0_ICW1			0x20
#define PIC0_ICW2			0x21
#define PIC0_ICW3			0x21
#define PIC0_ICW4			0x21
#define PIC0_IMR			0x21
#define PIC0_OCW2			0x20

#define PIC1_ICW1			0xa0
#define PIC1_ICW2			0xa1
#define PIC1_ICW3			0xa1
#define PIC1_ICW4			0xa1
#define PIC1_IMR			0xa1
#define PIC1_OCW2			0xa0

#define PIC_ICW1_ALWAYS_1	(1 << 4)		// 总为1的位
#define PIC_ICW1_ICW4		(1 << 0)		// 1 - 需要初始化ICW4
#define PIC_ICW4_8086	    (1 << 0)        // 8086工作模式

#define PIC_OCW2_EOI		(1 << 5)		// 1 - 非特殊结束中断EOI命令

#define IRQ_PIC_START		0x20			// PIC中断起始号


// 创建结构体, 用于指向传入的栈的指针
typedef struct _exception_frame_t 
{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t num, error_code;
    uint32_t eip, cs, eflags;
} exception_frame_t;

// 异常处理函数的声明, 在 kernel/init/start.S 中定义
void exception_handler_unknown(exception_frame_t *frame);
// void exception_handler_divider(void);
void exception_handler_divider(exception_frame_t *frame);
void exception_handler_Debug(exception_frame_t *frame);
void exception_handler_NMI(exception_frame_t *frame);
void exception_handler_breakpoint(exception_frame_t *frame);
void exception_handler_overflow(exception_frame_t *frame);
void exception_handler_bound_range(exception_frame_t *frame);
void exception_handler_invalid_opcode(exception_frame_t *frame);
void exception_handler_device_unavailable(exception_frame_t *frame);
void exception_handler_double_fault(exception_frame_t *frame);
void exception_handler_invalid_tss(exception_frame_t *frame);
void exception_handler_segment_not_present(exception_frame_t *frame);
void exception_handler_stack_segment_fault(exception_frame_t *frame);
void exception_handler_general_protection(exception_frame_t *frame);
void exception_handler_page_fault(exception_frame_t *frame);
void exception_handler_fpu_error(exception_frame_t *frame);
void exception_handler_alignment_check(exception_frame_t *frame);
void exception_handler_machine_check(exception_frame_t *frame);
void exception_handler_simd_exception(exception_frame_t *frame);
void exception_handler_virtual_exception(exception_frame_t *frame);
void exception_handler_control_exception(exception_frame_t *frame);

typedef void (*irq_handler_t)(exception_frame_t *frame);

int irq_install(int irq_num, irq_handler_t handler);

void irq_init(void);

// 用于中断处理函数的声明
void irq_enable(int irq_num);
void irq_disable(int irq_num);
void irq_enable_global(void);
void irq_disable_global(void);

void pic_send_eoi(int irq_num);

#endif
