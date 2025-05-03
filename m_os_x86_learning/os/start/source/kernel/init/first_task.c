#include "kernel/include/core/task.h"
#include "kernel/include/tools/log.h"
#include "applib/lib_syscall.h"

int first_task_main(void) {
    int pid = getpid();

    for (;;) {
        // log_printf("first task.\n");
        // sys_sleep(1000);
        
        // 提供函数通过调用门来调用系统调用
        print_msg("first task id = %d", pid);
        ms_sleep(1000);
    }

    return 0;
}
