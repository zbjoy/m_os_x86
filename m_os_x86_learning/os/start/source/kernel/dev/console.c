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

// 向一个方向移动 n 格
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

// 删除一个字符 (backspace 键功能)
static void erase_backword(console_t* console) {
    if (move_backword(console, 1) == 0) {
        show_char(console, ' '); // 显示空格
        move_backword(console, 1); // 光标向后移动一格
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

        console->old_cursor_col = console->cursor_col;
        console->old_cursor_row = console->cursor_row;
        console->write_state = CONSOLE_WRITE_NORMAL; // 写入状态

        console->disp_base = (disp_char_t*)(CONSOLE_DISP_ADDR + i * (CONSOLE_COL_MAX * CONSOLE_ROW_MAX)); // disp_char_t 是一个 2 字节的结构体, 每次给它的指针 +1 相当于加 2 个字节, 这里是一个 80 * 25 的显示缓冲区

        // clear_display(console);
    }
    return 0;
}

// 保存光标
static void save_cursor(console_t* console) {
    console->old_cursor_col = console->cursor_col;
    console->old_cursor_row = console->cursor_row;
}

// 恢复光标
static void restore_cursor(console_t* console) {
    console->cursor_col = console->old_cursor_col;
    console->cursor_row = console->old_cursor_row;
}

static void write_normal(console_t *console, char ch) {
    switch (ch) {
    case ASCII_ESC:
        console->write_state = CONSOLE_WRITE_ESC; // 转义字符
        break;
    case 0x7F:
        erase_backword(console); // 删除一个字符
        break;
    case '\b':
        move_backword(console, 1); // 光标向后移动一格
        break;
    case '\r':
        move_to_col0(console); // 光标移动到第一列
        break;
    case '\n':
        move_to_col0(console);
        move_next_line(console);
        break;
    default:
        if ((ch >= ' ') && (ch <= '~')) { // 可显示字符
            show_char(console, ch);
        }
        break;
    }
}

static void clear_esc_param(console_t* console) { // 清除 ESC 参数
    kernel_memset(console->esc_param, 0, sizeof(console->esc_param)); // 清除 ESC 参数
    console->curr_param_index = 0; // 当前参数索引
}

static void write_esc(console_t *console, char ch) {
    switch (ch) {
    case '7':
        // console->old_cursor_col = console->cursor_col;
        // console->old_cursor_row = console->cursor_row;
        save_cursor(console); // 保存光标位置
        console->write_state = CONSOLE_WRITE_NORMAL; // 恢复写入状态
        break;
    case '8':
        // console->cursor_col = console->old_cursor_col;
        // console->cursor_row = console->old_cursor_row;
        restore_cursor(console); // 恢复光标位置
        console->write_state = CONSOLE_WRITE_NORMAL; // 恢复写入状态
        break;
    case '[':
        clear_esc_param(console); // 清除 ESC 参数
        console->write_state = CONSOLE_WRITE_SQUARE; // 方括号
        break;
    default: // 遇到处理不了的错误
        console->write_state = CONSOLE_WRITE_NORMAL; // 恢复写入状态
        break;
    }
}

static void set_font_style(console_t* console) {
    static const color_t color_table[] = {
        COLOR_Black,
        COLOR_Red,
        COLOR_Green,
        COLOR_Yellow,
        COLOR_Blue,
        COLOR_Magenta,
        COLOR_Cyan,
        COLOR_White
    };

    for (int i = 0; i <= console->curr_param_index; i++) {
        int param = console->esc_param[i];
        if ((param >= 30) && (param <= 37)) {
            console->foreground = color_table[param - 30]; // 设置前景色
        } else if ((param >= 40) && (param <= 47)) {
            console->background = color_table[param - 40]; // 设置背景色
        } else if (param == 39) { 
            console->foreground = COLOR_White; // 设置前景色为白色
        } else if (param == 49) {
            console->background = COLOR_Black; // 设置背景色为黑色
        }
    }
}

static void move_left(console_t* console, int n) {
    if (n == 0) {
        n = 1;
    }

    int col = console->cursor_col - n;
    console->cursor_col = (col >= 0) ? col : 0; // 光标不能小于 0
}

static void move_right(console_t* console, int n) {
    if (n == 0) {
        n = 1;
    }

    int col = console->cursor_col + n;
    if (col > console->display_cols) {
        console->cursor_col = console->display_cols - 1; // 光标不能大于显示列数
    } else {
        console->cursor_col = col; // 光标移动到指定位置
    }
}

static void move_cursor(console_t* console) {
    console->cursor_row = console->esc_param[0] - 1; // 行号从 0 开始
    console->cursor_col = console->esc_param[1] - 1; // 列号从 0 开始
}

static void erase_in_display(console_t* console) {
    if (console->curr_param_index < 0) {
        return ;
    }

    int param = console->esc_param[0];
    if (param == 2) {
        erase_rows(console, 0, console->display_rows - 1); // 擦除整个屏幕
        console->cursor_col = console->cursor_row = 0; // 光标移动到第一行
    }
}

// ESC [pn (n 可以是 0-9 的数字) m 
// eg: ESC [31;42m
static void write_esc_square(console_t* console, char c) {
    if ((c >= '0') && (c <= '9')) {
        int* param = &console->esc_param[console->curr_param_index];
        *param = *param * 10 + (c - '0'); // 计算参数值
    } else if ((c == ';') && (console->curr_param_index < ESC_PARAM_MAX)) {
        console->curr_param_index++; // 参数索引加 1
    } else {
        switch (c) {
        case 'm':
            set_font_style(console); // 设置字体样式
            break;
        case 'D':
            move_left(console, console->esc_param[0]); // 光标左移
            break;
        case 'C':
            move_right(console, console->esc_param[0]); // 光标右移
            break;
        case 'H':
        case 'f':
            move_cursor(console); // 移动光标到指定位置
            break;
        case 'J':
            erase_in_display(console); // 擦除屏幕部分区域
            break;
        default:
            break;
        }

        console->write_state = CONSOLE_WRITE_NORMAL; // 恢复写入状态
    }
}

// console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
int console_write(int console, char* data, int size) {
    console_t* c = console_buf + console;
    int len;

    for (len = 0; len < size; len++) {
        char ch = *data++;

        switch (c->write_state) {
        case CONSOLE_WRITE_NORMAL:
            write_normal(c, ch);
            break;
        case CONSOLE_WRITE_ESC:
            write_esc(c, ch);
            break;
        case CONSOLE_WRITE_SQUARE:
            write_esc_square(c, ch);
            break;
        default:
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
