#ifndef DEV_H
#define DEV_H

#define DEV_NAME_SIZE 32 // 设备名称的最大长度

// 用来 describe 设备的类型
enum {
    DEV_UNKNOWN = 0,
    DEV_TTY, 
};

struct _dev_desc_t; // 前向声明
// device_t 描述了设备的详细类型
typedef struct _device_t {
    struct _dev_desc_t* desc; // 指向设备描述符的指针

    int mode; // 设备支持的模式 (eg: 打开, 只读, 可写...)
    int minor; // 设备的次设备号 (表示设备的具体实例, eg: 表示是计算机上的哪一块磁盘或定时器...)
    void* data;
    int open_count; // 设备被打开的次数
} device_t;

// dev_desc_t 描述了某种类型的设备 (eg: 描述了这是一个手机, 对于手机的具体类型通过结构体 device_t 进行描述)
typedef struct _dev_desc_t {
    char name[DEV_NAME_SIZE]; // 设备名称
    int major; // 主设备号 (表示设备类型 eg: 硬盘, 显示器等)
    
    int (*open)(device_t* dev);
    int (*read)(device_t* dev, int addr, char* buf, int size);
    int (*write)(device_t* dev, int addr, char* buf, int size);
    int (*control)(device_t* dev, int cmd, int arg0, int arg1);
    void (*close)(device_t* dev);
} dev_desc_t;

#endif
