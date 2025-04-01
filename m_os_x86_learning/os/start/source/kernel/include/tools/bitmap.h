#ifndef BITMAP_H
#define BITMAP_H

#include "comm/types.h"

typedef struct _bitmap_t {
    int bit_count; // 位图的大小
    uint8_t* bits; // 位图的指针
} bitmap_t;

void bitmap_init(bitmap_t* bitmap, uint8_t* bits, int count, int init_bit); // 初始化位图, init_bit: 0-清空, 1-置1
int bitmap_byte_count(int bit_count); // 计算位图的字节数
int bitmap_get_bit(bitmap_t* bitmap, int index); // 获取位图的某一位的值
void bitmap_set_bit(bitmap_t* bitmap, int index, int count, int bit); // 设置位图的某一(或者count个)位的值
int bitmap_is_set(bitmap_t* bitmap, int index); // 判断位图的某一位是否置1
int bitmap_alloc_nbits(bitmap_t* bitmap, int bit, int count); // 在位图中找连续count个为bit的位, 并将这些位取反, 为了用于分配和回收对应的页


#endif
