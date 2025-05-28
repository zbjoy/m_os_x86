#include "kernel/include/fs/file.h"
#include "kernel/include/ipc/mutex.h"
#include "kernel/include/tools/klib.h"

static file_t file_table[FILE_TABLE_SIZE]; // 文件表, 用于存放打开的文件
static mutex_t file_alloc_mutex; // 文件分配互斥锁

void file_table_init(void) { // 初始化文件表
    mutex_init(&file_alloc_mutex); // 初始化互斥锁
    kernel_memset(file_table, 0, sizeof(file_table)); // 初始化文件表
}

file_t* file_alloc(void) { // 分配一个文件结构体
    file_t* file = (file_t*)0; // 文件指针初始化为 NULL

    mutex_lock(&file_alloc_mutex); // 加锁

    for (int i = 0; i < FILE_TABLE_SIZE; i++) {
        file_t* p_file = file_table + i; // 指向当前文件结构体
        if (p_file->ref == 0) { // 如果文件引用计数为 0
            kernel_memset(p_file, 0, sizeof(file_t)); // 初始化文件结构体
            p_file->ref = 1; // 设置引用计数为 1
            file = p_file; // 分配成功, 返回文件结构体指针
            break; // 退出循环
        }
    }

    mutex_unlock(&file_alloc_mutex); // 解锁
    return file; // 返回文件结构体指针
}

void file_free(file_t* file) { // 释放一个文件结构体
    mutex_lock(&file_alloc_mutex); // 加锁

    if (file->ref) { // 如果文件引用计数大于 0
        file->ref--; // 引用计数减 1
    }

    mutex_unlock(&file_alloc_mutex); // 解锁
}

