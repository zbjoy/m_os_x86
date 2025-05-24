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

typedef struct _key_map_t {
    uint8_t normal; // 普通按键 (eg: 对于字母取 未按下 shift 时的 ASCII 码)
    uint8_t func;   // 第二功能 (eg: 对于字母取 按下 shift 时的 ASCII 码)
} key_map_t;

typedef struct _kbd_state_t {
    int caps_lock : 1; // caps lock 键是否被按下
    int lshift_pressed : 1; // 左 shift 键是否被按下
    int rshift_pressed : 1; // 右 shift 键是否被按下
} kbd_state_t;

void kbd_init(void); // 初始化键盘
void exception_handler_kbd(void); // 键盘中断处理函数


#endif
