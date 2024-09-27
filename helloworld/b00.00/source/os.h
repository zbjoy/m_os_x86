#ifndef OS_H
#define OS_H

#define KERNEL_CODE_SEG         8
#define KERNEL_DATA_SEG         16

#define APP_CODE_SEG            ((3 * 8) | 3)   // 使其在 特权值为 3 的段中运行
#define APP_DATA_SEG            ((4 * 8) | 3)   // 使其在 特权值为 3 的段中运行

#endif // OS_H
