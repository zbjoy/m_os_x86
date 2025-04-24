#include "kernel/include/core/task.h"
#include "kernel/include/tools/log.h"

int first_task_main(void) {
    for (;;) {
        // log_printf("first task.\n");
        // sys_sleep(1000);
    }

    return 0;
}
