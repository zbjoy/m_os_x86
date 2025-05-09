#include "kernel/include/core/task.h"
#include "kernel/include/tools/log.h"
#include "applib/lib_syscall.h"

int first_task_main(void) {
    int pid = getpid();
    int count = 3;

    print_msg("first task id = %d\n", pid);

    pid = fork();
    if (pid < 0) {
        print_msg("create child proc failed\n", 0);
    } else if (pid == 0) {
        // print_msg("child task id = %d\n", getpid());
        count += 3;
        print_msg("child: %d\n", count);

        char* argv[] = {"arg0", "arg1", "arg2", "arg3", (char*)0};
        execve("/shell.elf", argv, (char**)0); // 执行 shell 进程
    } else {
        print_msg("child task id = %d\n", pid);
        print_msg("parent: %d\n", count);
    }

    for (;;) {
        // log_printf("first task.\n");
        // sys_sleep(1000);
        
        // 提供函数通过调用门来调用系统调用
        print_msg("first task id = %d", pid);
        ms_sleep(1000);
    }

    return 0;
}
