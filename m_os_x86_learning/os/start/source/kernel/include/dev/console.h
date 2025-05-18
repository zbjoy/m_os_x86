#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"

#define CONSOLE_DISP_ADDR 0xb8000
#define CONSOLE_DISP_END (0xb8000 + 32 * 1024)
#define CONSOLE_ROW_MAX 25
#define CONSOLE_COL_MAX 80

#define ASCII_ESC 0x1b // \033
#define ESC_PARAM_MAX 10 // ESC 缓冲区的大小

typedef enum {
    COLOR_Black = 0,
    COLOR_Blue,
    COLOR_Green,
    COLOR_Cyan,
    COLOR_Red,
    COLOR_Magenta,
    COLOR_Brown,
    COLOR_Gray,
    COLOR_DarkGray,
    COLOR_Light_Blue,
    COLOR_Light_Green,
    COLOR_Light_Cyan,
    COLOR_Light_Red,
    COLOR_Light_Magenta,
    COLOR_Yellow,
    COLOR_White
} color_t;

// 通过联合体将 v 拆开方便使用
// typedef struct _disp_char_t {
    // uint16_t v; // 第一个字节表示 ascii 码，第二个字节表示颜色
// } disp_char_t;
typedef union _disp_char_t {
    struct {
        char c; // 字符
        char foreground : 4; // 前景色(4bit)
        char background : 3; // 背景色(3bit)
    };
    uint16_t v; // 第一个字节表示 ascii 码，第二个字节表示颜色
} disp_char_t;

typedef struct _console_t {
    enum {
        CONSOLE_WRITE_NORMAL = 0, // 普通写入
        CONSOLE_WRITE_ESC, // 转义字符
        CONSOLE_WRITE_SQUARE, // 方括号
    } write_state;

    disp_char_t* disp_base; // 显示缓冲区的基地址
    int cursor_row, cursor_col; // 光标所在的行和列
    int display_rows, display_cols; // 显示缓冲区的行数和列数
    color_t foreground; // 前景色
    color_t background; // 背景色

    int old_cursor_col, old_cursor_row; // 上一次光标所在的行和列
    int esc_param[ESC_PARAM_MAX]; // ESC 参数

} console_t;

int console_init(void);
int console_write(int console, char* data, int size); // console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
void console_clear(int console); // console: 要清除的控制台(句柄)

#endif
