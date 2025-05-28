#ifndef FILE_H
#define FILE_H

#include "comm/types.h"

#define FILE_NAME_SIZE 32 // 文件名最大长度
#define FILE_TABLE_SIZE 2048 // 文件表大小, 用于存放打开的文件

typedef enum _file_type_t {
    FILE_UNKNOWN = 0,
    FILE_TTY, // 终端设备文件
} file_type_t;

typedef struct _file_t {
    char file_name[FILE_NAME_SIZE]; // 文件名
    file_type_t type; // 文件类型
    uint32_t size; // 文件大小
    int ref; // 引用计数, 用于文件打开计数
    int dev_id; // 设备 id, 用于标识文件所在的设备
    int pos; // 文件指针, 用于标识文件的当前位置
    int mode; // 文件打开模式 (eg: 只读, 只写, 读写等)
} file_t;

file_t* file_alloc(void); // 分配一个文件结构体
void file_free(file_t* file); // 释放一个文件结构体
void file_table_init(void); // 初始化文件表

#endif
