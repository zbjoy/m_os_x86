#include "kernel/include/tools/bitmap.h"
#include "kernel/include/tools/klib.h"

int bitmap_byte_count(int bit_count) {
    return (bit_count + 7) / 8;
}

void bitmap_init(bitmap_t* bitmap, uint8_t* bits, int count, int init_bit) {
    bitmap->bit_count = count;
    bitmap->bits = bits;
    int bytes = bitmap_byte_count(bitmap->bit_count);
    kernel_memset(bitmap->bits, init_bit ? 0xFF : 0, bytes);
}

int bitmap_get_bit(bitmap_t* bitmap, int index) { // 获取位图的某一位的值
    return bitmap->bits[index / 8] & (1 << (index % 8));
}
void bitmap_set_bit(bitmap_t* bitmap, int index, int count, int bit) { // 设置位图的某一(或者count个)位的值
    for (int i = 0; (i < count) && (index < bitmap->bit_count); i++, index++) {
        if (bit) {
            bitmap->bits[index / 8] |= (1 << (index % 8)); // 设置为1
        } else {
            bitmap->bits[index / 8] &= ~(1 << (index % 8)); // 设置为0
        }
    }
}
int bitmap_is_set(bitmap_t* bitmap, int index) { // 判断位图的某一位是否置1
    return bitmap_get_bit(bitmap, index) ? 1 : 0;
}
int bitmap_alloc_nbits(bitmap_t* bitmap, int bit, int count) { // 在位图中找连续count个为bit的位, 并将这些位取反, 为了用于分配和回收对应的页
    int search_index = 0;
    int ok_index = -1; // 找到的第一个符合条件的位

    while (search_index < bitmap->bit_count) {
        if (bitmap_get_bit(bitmap, search_index) != bit) {
            search_index++;
            continue;
        }
        // 如果找到了一个符合条件的位, 那么就开始判断后面是否有count个连续的符合条件的位
        ok_index = search_index;
        for (int i = 1; (i < count) && (search_index < bitmap->bit_count); i++) {
            if (bitmap_get_bit(bitmap, search_index++) != bit) {
                ok_index = -1; // 没有找到连续的count个符合条件的位, 退出循环
                break;
            }
        }

        if (i >= count) {
            bitmap_set_bit(bitmap, ok_index, count, !bit); // 将找到的count个符合条件的位取反
            return ok_index; // 返回找到的第一个符合条件的位的索引
        }
    }

    return -1; // 没有找到符合条件的位

}
