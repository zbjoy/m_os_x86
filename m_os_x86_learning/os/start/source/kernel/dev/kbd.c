#include "kernel/include/dev/kbd.h"
#include "kernel/include/cpu/irq.h"

void kbd_init(void) {
    irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
    irq_enable(IRQ1_KEYBOARD);
}


void do_handler_kbd(exception_frame_t *frame) { // 通过汇编实现

}
