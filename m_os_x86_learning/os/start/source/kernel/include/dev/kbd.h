// 键盘相关代码
#ifndef KDB_H
#define KDB_H

#include "comm/types.h"

#define KBD_PORT_DATA 0x60  // 键盘数据端口
#define KBD_PORT_STAT  0x64  // 键盘命令端口 (有两个功能: 读和写), 读: 读键盘控制器的状态; 写: 写命令到键盘控制器(往0x64端口写命令), 这里是读命令
#define KBD_PORT_CMD   0x64  // 键盘命令端口 (有两个功能: 读和写), 读: 读键盘控制器的状态; 写: 写命令到键盘控制器(往0x64端口写命令), 这里是写命令

#define KBD_STAT_RECV_READY (1 << 0) // 键盘接收就绪标志位 (键盘控制器的状态寄存器的第 0 位)

#define KEY_RSHIFT 0x36 // 右 shift 键
#define KEY_LSHIFT 0x2A // 左 shift 键
#define KEY_CAPS 0x3A // caps lock 键

#define KEY_CTRL 0x1D // ctrl 键, 左 CTRL 为一个字符, 右 CRTL 为两个字符, 并且第二个字符和 左CRTL 键的 ASCII 码相同
#define KEY_RSHIFT 0x36
#define KEY_LSHIFT 0x2A
#define KEY_ALT 0x38 // alt 键

#define KEY_F1 0x3B // F1 键
#define KEY_F2 0x3C // F2 键
#define KEY_F3 0x3D // F3 键
#define KEY_F4 0x3E // F4 键
#define KEY_F5 0x3F // F5 键
#define KEY_F6 0x40 // F6 键
#define KEY_F7 0x41 // F7 键
#define KEY_F8 0x42 // F8 键
#define KEY_F9 0x43 // F9 键
#define KEY_F10 0x44 // F10 键
#define KEY_F11 0x57 // F11 键
#define KEY_F12 0x58 // F12 键

#define KEY_E0 0xE0 // 扩展键 (用于组合键) (两个字符)
#define KEY_E1 0xE1 // 扩展键 (用于组合键) (三个字符)


typedef struct _key_map_t {
    uint8_t normal; // 普通按键 (eg: 对于字母取 未按下 shift 时的 ASCII 码)
    uint8_t func;   // 第二功能 (eg: 对于字母取 按下 shift 时的 ASCII 码)
} key_map_t;

typedef struct _kbd_state_t {
    int caps_lock : 1; // caps lock 键是否被按下
    int lshift_pressed : 1; // 左 shift 键是否被按下
    int rshift_pressed : 1; // 右 shift 键是否被按下
    int lalt_press : 1; // 左 alt 键是否被按下
    int ralt_press : 1; // 右 alt 键是否被按下
    int lctrl_press : 1; // 左 ctrl 键是否被按下
    int rctrl_press : 1; // 右 ctrl 键是否被按下
} kbd_state_t;

void kbd_init(void); // 初始化键盘
void exception_handler_kbd(void); // 键盘中断处理函数


#endif
