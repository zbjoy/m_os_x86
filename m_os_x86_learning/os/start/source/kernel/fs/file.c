#include "kernel/include/fs/file.h"
#include "kernel/include/ipc/mutex.h"

static file_t file_table[FILE_TABLE_SIZE]; // 文件表, 用于存放打开的文件

file_t* file_alloc(void) { // 分配一个文件结构体

}

void file_free(file_t* file) { // 释放一个文件结构体

}

void file_table_init(void) { // 初始化文件表

}
