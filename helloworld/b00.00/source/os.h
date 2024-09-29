#ifndef OS_H
#define OS_H

#define KERNEL_CODE_SEG         8
#define KERNEL_DATA_SEG         16

#define APP_CODE_SEG            ((3 * 8) | 3)   // 使其在 特权值为 3 的段中运行
#define APP_DATA_SEG            ((4 * 8) | 3)   // 使其在 特权值为 3 的段中运行

#define TASK0_TSS_SEG           ((5 * 8))       // 相当于 特权值为 0 的段中运行
#define TASK1_TSS_SEG           ((6 * 8))

#define SYSCALL_SEG             ((7 * 8))       // 系统调用的索引

#endif // OS_H
