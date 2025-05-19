// 键盘相关代码
#ifndef KDB_H
#define KDB_H

#define KBD_PORT_DATA 0x60  // 键盘数据端口
#define KBD_PORT_STAT  0x64  // 键盘命令端口 (有两个功能: 读和写), 读: 读键盘控制器的状态; 写: 写命令到键盘控制器(往0x64端口写命令), 这里是读命令
#define KBD_PORT_CMD   0x64  // 键盘命令端口 (有两个功能: 读和写), 读: 读键盘控制器的状态; 写: 写命令到键盘控制器(往0x64端口写命令), 这里是写命令

#define KBD_STAT_RECV_READY (1 << 0) // 键盘接收就绪标志位 (键盘控制器的状态寄存器的第 0 位)


void kbd_init(void); // 初始化键盘
void exception_handler_kbd(void); // 键盘中断处理函数


#endif
