#include "kernel/include/core/task.h"
#include "kernel/include/tools/log.h"
#include "applib/lib_syscall.h"

int first_task_main(void) {
    for (;;) {
        // log_printf("first task.\n");
        // sys_sleep(1000);
        
        // 提供函数通过调用门来调用系统调用
        ms_sleep(1000);
    }

    return 0;
}
