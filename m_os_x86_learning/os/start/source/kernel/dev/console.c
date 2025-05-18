#include "kernel/include/dev/console.h"
#include "kernel/include/tools/klib.h"
#include "comm/cpu_instr.h"

#define CONSOLE_NR 1 // 目前只做一个屏幕的处理

static console_t console_buf[CONSOLE_NR];

// 读取当前光标位置
static int read_cursor_pos(void) {
    int pos;

    // 获得光标位置
    outb(0x3D4, 0xF); // 读取光标位置的低字节
    pos = inb(0x3D5); // 读取光标位置的低字节

    outb(0x3D4, 0xE); // 读取光标位置的高字节
    pos |= inb(0x3D5) << 8; // 读取光标位置的高字节

    return pos;
}

// 更新当前光标位置
static int update_cursor_pos(console_t* console) {
    // 当前光标位置
    uint16_t pos = console->cursor_row * console->display_cols + console->cursor_col;

    // 设置光标位置 (将光标位置写入屏幕)
    // 写低 8 位
    outb(0x3D4, 0xF); // 读取光标位置的低字节
    outb(0x3D5, (uint8_t)(pos & 0xFF)); // 写入光标位置的低字节

    // 写高 8 位
    outb(0x3D4, 0xE); // 读取光标位置的高字节
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF)); // 写入光标位置的高字节

    return pos;
}

// 清除从 start 行到 end 行的行
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

// 向上滚动 lines 行
static void scroll_up(console_t* console, int lines) {
    disp_char_t* dest = console->disp_base;
    disp_char_t* src = console->disp_base + lines * console->display_cols;
    uint32_t size = (console->display_rows - lines) * console->display_cols * sizeof(disp_char_t);
    kernel_memcpy(dest, src, size); // 向上滚动

    erase_rows(console, console->display_rows - lines, console->display_rows - 1); // 清除新出现的行
    console->cursor_row -= lines; // 光标向上移动
}

// 光标移动到第一列
static void move_to_col0(console_t* console) {
    console->cursor_col = 0; // 光标移动到第一列
}

// 光标移动到下一行
static void move_next_line(console_t* console) {
    console->cursor_row++;
    if (console->cursor_row >= console->display_rows) {
        scroll_up(console, 1); // 屏幕向上滚动一行
    }
}

// 光标向前移动 n 格
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

// 光标向后移动 n 格
static void show_char(console_t* console, char c) {
    // 获得显存的起始地址
    int offset = console->cursor_col + console->cursor_row * console->display_cols;

    disp_char_t* p = console->disp_base + offset;
    p->c = c; // 显示字符
    p->foreground = console->foreground; // 前景色
    p->background = console->background; // 背景色
    move_forward(console, 1);    // 光标向前移动一格

}

// 清除显示缓冲区
static void clear_display(console_t* console) {
    int size = console->display_rows * console->display_cols;
    disp_char_t* start = console->disp_base;
    for (int i = 0; i < size; i++, start++) {
        start->c = ' '; // 显示空格
        start->foreground = console->foreground; // 前景色
        start->background = console->background; // 背景色
    }
}

static int move_backword(console_t* console, int n) {
    int status = -1;
    for (int i = 0; i < n; ++i) {
        if (console->cursor_col > 0) { // 不处于每一行的开头
            console->cursor_col--;
            status = 0;
        } else if (console->cursor_col == 0 && console->cursor_row > 0) { // 处于每一行的开头, 并且不是第一行
            console->cursor_row--;
            console->cursor_col = console->display_cols - 1;
            status = 0;
        }
    }
    return status;
}

static void erase_backword(console_t* console) {
    if (move_backword(console, 1) == 0) {
        show_char(console, ' '); // 显示空格
        move_forward(console, 1); // 光标向前移动一格
    }
}


// 初始化控制台
int console_init(void) {
    for (int i = 0; i < CONSOLE_NR; i++) {
        console_t* console = console_buf + i;

        console->display_cols = CONSOLE_COL_MAX;
        console->display_rows = CONSOLE_ROW_MAX;

        console->foreground = COLOR_White; // 前景色
        console->background = COLOR_Black; // 背景色

        // 读取当前光标位置
        int cursor_pos = read_cursor_pos();

        // 光标位置初始化
        console->cursor_row = cursor_pos / console->display_cols;
        console->cursor_col = cursor_pos % console->display_cols;

        console->disp_base = (disp_char_t*)(CONSOLE_DISP_ADDR + i * (CONSOLE_COL_MAX * CONSOLE_ROW_MAX)); // disp_char_t 是一个 2 字节的结构体, 每次给它的指针 +1 相当于加 2 个字节, 这里是一个 80 * 25 的显示缓冲区

        // clear_display(console);
    }
    return 0;
}

// console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
int console_write(int console, char* data, int size) {
    console_t* c = console_buf + console;
    int len;

    for (len = 0; len < size; len++) {
        char ch = *data++;

        switch(ch) {
        case 0x7F:
            erase_backword(c); // 删除一个字符
            break;
        case '\b':
            move_backword(c, 1); // 光标向后移动一格
            break;
        case '\r':
            move_to_col0(c); // 光标移动到第一列
            break;
        case '\n':
            move_to_col0(c);
            move_next_line(c);
            break;
        default:
            if ((ch >= ' ') && (ch <= '~')) { // 可显示字符
                show_char(c, ch);
            }
            break;
        }

        // TODO: 处理控制字符
        // show_char(c, ch);

    }
    
    update_cursor_pos(c); // 更新光标位置
    return len;
}
 // console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
void console_clear(int console) {
    
}
 // console: 要清除的控制台(句柄)
