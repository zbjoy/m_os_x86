#include "kernel/include/dev/console.h"

#define CONSOLE_NR 1 // 目前只做一个屏幕的处理

static console_t console_buf[CONSOLE_NR];

int console_init(void) {
    for (int i = 0; i < CONSOLE_NR; i++) {
        console_t* console = console_buf + i;

        console->display_cols = CONSOLE_COL_MAX;
        console->display_rows = CONSOLE_ROW_MAX;
        console->disp_base = (disp_char_t*)(CONSOLE_DISP_ADDR + i * (CONSOLE_COL_MAX * CONSOLE_ROW_MAX)); // disp_char_t 是一个 2 字节的结构体, 每次给它的指针 +1 相当于加 2 个字节, 这里是一个 80 * 25 的显示缓冲区
    }
    return 0;
}

int console_write(int console, char* data, int size) {
    console_t* c = console_buf + console;
    int len;

    for (len = 0; len < size; len++) {
        char c = *data++;

        // TODO: 处理控制字符
    }
    return len;
}
 // console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
void console_clear(int console) {
    
}
 // console: 要清除的控制台(句柄)
