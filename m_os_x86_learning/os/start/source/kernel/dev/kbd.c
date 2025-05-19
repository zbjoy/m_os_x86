#include "kernel/include/dev/kbd.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/tools/log.h"
#include "comm/cpu_instr.h"

void kbd_init(void) {
    irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
    irq_enable(IRQ1_KEYBOARD);
}


void do_handler_kbd(exception_frame_t *frame) { // 通过汇编实现
    uint32_t status = inb(KBD_PORT_STAT); // 读取键盘状态端口
    if (!(status & KBD_STAT_RECV_READY)) { // 是其他事情(不是键盘导致的中断)
        pic_send_eoi(IRQ1_KEYBOARD); // 发送结束信号
        return;
    }

    uint8_t raw_code = inb(KBD_PORT_DATA); // 读取键盘数据端口
    log_printf("key code: %x\n", raw_code); // 打印键盘扫描码

    pic_send_eoi(IRQ1_KEYBOARD); // 发送结束信号
}
