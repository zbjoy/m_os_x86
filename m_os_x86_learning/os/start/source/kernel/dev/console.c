#include "kernel/include/dev/console.h"
#include "kernel/include/tools/klib.h"

#define CONSOLE_NR 1 // 目前只做一个屏幕的处理

static console_t console_buf[CONSOLE_NR];

static void erase_rows(console_t* console, int start, int end) {
    disp_char_t* disp_start = console->disp_base + console->display_cols * start;
    disp_char_t* disp_end = console->disp_base + console->display_cols * (end + 1);
    while (disp_start < disp_end) {
        disp_start->c = ' '; // 清除字符
        disp_start->foreground = console->foreground; // 前景色
        disp_start->background = console->background; // 背景色
        disp_start++;
    }
}

static void scroll_up(console_t* console, int lines) {
    disp_char_t* dest = console->disp_base;
    disp_char_t* src = console->disp_base + lines * console->display_cols;
    uint32_t size = (console->display_rows - lines) * console->display_cols * sizeof(disp_char_t);
    kernel_memcpy(dest, src, size); // 向上滚动

    erase_rows(console, console->display_rows - lines, console->display_rows - 1); // 清除新出现的行
    console->cursor_row -= lines; // 光标向上移动
}

static void move_to_col0(console_t* console) {
    console->cursor_col = 0; // 光标移动到第一列
}

static void move_next_line(console_t* console) {
    console->cursor_row++;
    if (console->cursor_row >= console->display_rows) {
        scroll_up(console, 1); // 屏幕向上滚动一行
    }
}

static void move_forward(console_t* console, int n) {
    for (int i = 0; i < n; ++i) {
        if (++console->cursor_col >= console->display_cols) {
            console->cursor_row++;
            console->cursor_col = 0;
        
            if (console->cursor_row >= console->display_rows) {
                scroll_up(console, 1); // 屏幕向上滚动一行
            }
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

static void clear_display(console_t* console) {
    int size = console->display_rows * console->display_cols;
    disp_char_t* start = console->disp_base;
    for (int i = 0; i < size; i++, start++) {
        start->c = ' '; // 显示空格
        start->foreground = console->foreground; // 前景色
        start->background = console->background; // 背景色
    }
}

int console_init(void) {
    for (int i = 0; i < CONSOLE_NR; i++) {
        console_t* console = console_buf + i;

        // 光标位置初始化
        console->cursor_row = 0;
        console->cursor_col = 0;


        console->display_cols = CONSOLE_COL_MAX;
        console->display_rows = CONSOLE_ROW_MAX;

        console->foreground = COLOR_White; // 前景色
        console->background = COLOR_Black; // 背景色

        console->disp_base = (disp_char_t*)(CONSOLE_DISP_ADDR + i * (CONSOLE_COL_MAX * CONSOLE_ROW_MAX)); // disp_char_t 是一个 2 字节的结构体, 每次给它的指针 +1 相当于加 2 个字节, 这里是一个 80 * 25 的显示缓冲区

        clear_display(console);
    }
    return 0;
}

int console_write(int console, char* data, int size) {
    console_t* c = console_buf + console;
    int len;

    for (len = 0; len < size; len++) {
        char ch = *data++;

        switch(ch) {
        case '\n':
            move_to_col0(c);
            move_next_line(c);
            break;
        default:
            show_char(c, ch);
            break;
        }

        // TODO: 处理控制字符
        // show_char(c, ch);

    }
    return len;
}
 // console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
void console_clear(int console) {
    
}
 // console: 要清除的控制台(句柄)
