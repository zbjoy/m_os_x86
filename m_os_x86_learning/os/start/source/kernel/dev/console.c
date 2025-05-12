#include "kernel/include/dev/console.h"

#define CONSOLE_NR 1 // 目前只做一个屏幕的处理

static console_t console_buf[CONSOLE_NR];

int console_init(void) {
    
}

int console_write(int console, char* data, int size) {

}
 // console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
void console_clear(int console) {

}
 // console: 要清除的控制台(句柄)
