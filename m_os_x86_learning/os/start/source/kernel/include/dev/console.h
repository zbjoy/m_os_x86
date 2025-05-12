#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"

#define CONSOLE_DISP_ADDR 0xb8000
#define CONSOLE_DISP_END (0xb8000 + 32 * 1024)
#define CONSOLE_ROW_MAX 25
#define CONSOLE_COL_MAX 80

typedef struct _disp_char_t {
    uint16_t v; // 第一个字节表示 ascii 码，第二个字节表示颜色
} disp_char_t;

typedef struct _console_t {
    disp_char_t* disp_base; // 显示缓冲区的基地址
    int display_rows, display_cols; // 显示缓冲区的行数和列数
} console_t;

int console_init(void);
int console_write(int console, char* data, int size); // console: 写的是哪个控制台, data: 要写入的数据, size: 要写入的数据的长度
void console_clear(int console); // console: 要清除的控制台(句柄)

#endif
