#include "kernel/include/dev/kbd.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/tools/log.h"
#include "comm/cpu_instr.h"
#include "kernel/include/tools/klib.h"

static kbd_state_t kbd_state; // 键盘状态

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
    static int inited = 0;
    if (!inited) {
        kernel_memset(&kbd_state, 0, sizeof(kbd_state)); // 初始化键盘状态
        irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
        irq_enable(IRQ1_KEYBOARD);
        inited = 1; // 标记为已初始化
    }
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
    case KEY_RSHIFT:
        kbd_state.rshift_pressed = is_make ? 1 : 0; // 右 shift 键
        break;
    case KEY_LSHIFT:
        kbd_state.lshift_pressed = is_make ? 1 : 0; // 左 shift 键
        break;
    case KEY_CAPS:
        if (is_make) {
            kbd_state.caps_lock = ~kbd_state.caps_lock; // 切换 caps lock 状态
        }
        break; // caps lock 键
    case KEY_ALT: // alt 键
        kbd_state.lalt_press = is_make;
        break; 
    case KEY_CTRL: // ctrl 键
        kbd_state.lctrl_press = is_make;
        break;
    case KEY_F1: // F1 键
    case KEY_F2: // F2 键
    case KEY_F3: // F3 键
    case KEY_F4: // F4 键
    case KEY_F5: // F5 键
    case KEY_F6: // F6 键
    case KEY_F7: // F7 键
    case KEY_F8: // F8 键
    case KEY_F9: // F9 键
    case KEY_F10: // F10 键
    case KEY_F11: // F11 键
    case KEY_F12: // F12 键
        break;
    default:
        if (is_make) {
            if (kbd_state.lshift_pressed || kbd_state.rshift_pressed) { // 如果按下了 shift 键
                key = map_table[key].func; // 第二功能
            } else {
                key = map_table[key].normal; // 普通按键
            }

            if (kbd_state.caps_lock) { // 如果 caps lock 键被按下
                if ((key >= 'A') && (key <= 'Z')) { // 如果是大写字母
                    key = key - 'A' + 'a'; // 转换为小写字母
                } else if ((key >= 'a') && (key <= 'z')) { // 如果是小写字母
                    key = key - 'a' + 'A'; // 转换为大写字母
                }
            }
            log_printf("key: %c\n", key);
        }
        break;
    }
}

static void do_e0_key(uint8_t raw_code) {
    char key = get_key(raw_code);
    int is_make = is_make_code(raw_code);
    switch (key) {
    case KEY_ALT: // 右 alt 键
        kbd_state.ralt_press = is_make;
        break; 
    case KEY_CTRL: // 右 ctrl 键
        kbd_state.rctrl_press = is_make;
        break;
    default:
        break;
    }
}


void do_handler_kbd(exception_frame_t *frame) { // 通过汇编实现
    static enum {
        NORMAL, 
        BEGIN_E0,
        BBEGIN_E1,
    } recv_state = NORMAL;
    uint32_t status = inb(KBD_PORT_STAT); // 读取键盘状态端口
    if (!(status & KBD_STAT_RECV_READY)) { // 是其他事情(不是键盘导致的中断)
        pic_send_eoi(IRQ1_KEYBOARD); // 发送结束信号
        return;
    }

    uint8_t raw_code = inb(KBD_PORT_DATA); // 读取键盘数据端口
    // do_normal_key(raw_code);
    // log_printf("key code: %x\n", raw_code); // 打印键盘扫描码

    pic_send_eoi(IRQ1_KEYBOARD); // 发送结束信号

    if (raw_code == KEY_E0) {
        recv_state = BEGIN_E0;
    } else if (raw_code == KEY_E1) {
        recv_state = BBEGIN_E1;
    } else {
        switch (recv_state) {
        case NORMAL:
            do_normal_key(raw_code);
            break;
        case BEGIN_E0:
            do_e0_key(raw_code);
            recv_state = NORMAL;
            break;
        case BBEGIN_E1: // TODO: 处理 E1 键
            recv_state = NORMAL;
            break;
        default:
            break;
        }
    }
}
