#ifndef OS_CFG_H
#define OS_CFG_H

#define GDT_TABLE_SIZE 256

#define KERNEL_SELECTOR_CS (1 * 8)
#define KERNEL_SELECTOR_DS (2 * 8)
#define SELECTOR_SYSCALL (3 * 8)

#define KERNEL_STACK_SIZE (8 * 1024)

#define OS_TICKS_MS 10

#define OS_VERSION "1.0.0"

#define TASK_NR 128 // 任务的数量, 128个任务

#endif /* OS_CFG_H */
