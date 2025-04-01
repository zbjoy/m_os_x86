#ifndef BITMAP_H
#define BITMAP_H

#include "comm/types.h"

typedef struct _bitmap_t {
    int bit_count; // 位图的大小
    uint8_t* bits; // 位图的指针
} bitmap_t;

void bitmap_init(bitmap_t* bitmap, uint8_t* bits, int count, int init_bit); // 初始化位图, init_bit: 0-清空, 1-置1
int bitmap_byte_count(int bit_count); // 计算位图的字节数


#endif
