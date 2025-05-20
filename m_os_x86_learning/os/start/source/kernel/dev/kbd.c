#include "kernel/include/dev/kbd.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/tools/log.h"
#include "comm/cpu_instr.h"

// 通过键值映射实现对应键值的获取
static const key_map_t map_table[] = {
    [0x2] = {'1', '!'},
    [0x3] = {'2', '@'},
    [0x4] = {'3', '#'},
    [0x5] = {'4', '$'},
    [0x6] = {'5', '%'},
    [0x7] = {'6', '^'},
    [0x08] = {'7', '&'},
    [0x09] = {'8', '*'},
    [0x0A] = {'9', '('},
    [0x0B] = {'0', ')'},
    [0x0C] = {'-', '_'},
    [0x0D] = {'=', '+'},
    [0x0E] = {'\b', '\b'}, // 退格键 backspace
    [0x0F] = {'\t', '\t'},
    // 对于字母, 需要判断是否已经按下 shift 键
    [0x10] = {'q', 'Q'},
    [0x11] = {'w', 'W'},
    [0x12] = {'e', 'E'},
    [0x13] = {'r', 'R'},
    [0x14] = {'t', 'T'},
    [0x15] = {'y', 'Y'},
    [0x16] = {'u', 'U'},
    [0x17] = {'i', 'I'},
    [0x18] = {'o', 'O'},
    [0x19] = {'p', 'P'},
    [0x1A] = {'[', '{'},
    [0x1B] = {']', '}'},
    [0x1C] = {'\n', '\n'},
    [0x1E] = {'a', 'A'},
    [0x1F] = {'s', 'B'},
    [0x20] = {'d', 'D'},
    [0x21] = {'f', 'F'},
    [0x22] = {'g', 'G'},
    [0x23] = {'h', 'H'},
    [0x24] = {'j', 'J'},
    [0x25] = {'k', 'K'},
    [0x26] = {'l', 'L'},
    [0x27] = {';', ':'},
    [0x28] = {'\'', '"'},
    [0x29] = {'`', '~'},
    [0x2B] = {'\\', '|'},
    [0x2C] = {'z', 'Z'},
    [0x2D] = {'x', 'X'},
    [0x2E] = {'c', 'C'},
    [0x2F] = {'v', 'V'},
    [0x30] = {'b', 'B'},
    [0x31] = {'n', 'N'},
    [0x32] = {'m', 'M'},
    [0x33] = {',', '<'},
    [0x34] = {'.', '>'},
    [0x35] = {'/', '?'},
    [0x39] = {' ', ' '},
};

void kbd_init(void) {
    irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
    irq_enable(IRQ1_KEYBOARD);
}

static inline int is_make_code(uint8_t key_code) {
    return !(key_code & 0x80); // 如果高位为 1, 则是释放键; 否则是按下键
}

static inline char get_key(uint8_t key_code) {
    return key_code & 0x7F; // 取低 7 位
}

static void do_normal_key(uint8_t raw_code) {
    char key = get_key(raw_code);
    int is_make = is_make_code(raw_code);
    switch (key) {
    default:
        if (is_make) {
            key = map_table[key].normal; // 普通按键
            log_printf("key: %d\n", key);
        }
        break;
    }
}


void do_handler_kbd(exception_frame_t *frame) { // 通过汇编实现
    uint32_t status = inb(KBD_PORT_STAT); // 读取键盘状态端口
    if (!(status & KBD_STAT_RECV_READY)) { // 是其他事情(不是键盘导致的中断)
        pic_send_eoi(IRQ1_KEYBOARD); // 发送结束信号
        return;
    }

    uint8_t raw_code = inb(KBD_PORT_DATA); // 读取键盘数据端口
    // log_printf("key code: %x\n", raw_code); // 打印键盘扫描码

    pic_send_eoi(IRQ1_KEYBOARD); // 发送结束信号
}
