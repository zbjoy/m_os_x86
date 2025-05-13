#include "kernel/include/dev/console.h"

#define CONSOLE_NR 1 // 目前只做一个屏幕的处理

static console_t console_buf[CONSOLE_NR];

static void move_forward(console_t* console, int n) {
    for (int i = 0; i < n; ++i) {
        if (++console->cursor_col >= console->display_cols) {
            console->cursor_row++;
            console->cursor_col = 0;
        }
    }
}

static void show_char(console_t* console, char c) {
    // 获得显存的起始地址
    int offset = console->cursor_col + console->cursor_row * console->display_cols;

    disp_char_t* p = console->disp_base + offset;
    p->c = c; // 显示字符
    p->foreground = console->foreground; // 前景色
    p->background = console->background; // 背景色
    move_forward(console, 1);    // 光标向前移动一格

}

int console_init(void) {
    for (int i = 0; i < CONSOLE_NR; i++) {
        console_t* console = console_buf + i;

        // 光标位置初始化
        console->cursor_row = 0;
        console->cursor_col = 0;


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
        char ch = *data++;

        // TODO: 处理控制字符
        show_char(ch);
    }
    return len;
}
 // console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
void console_clear(int console) {
    
}
 // console: 要清除的控制台(句柄)
