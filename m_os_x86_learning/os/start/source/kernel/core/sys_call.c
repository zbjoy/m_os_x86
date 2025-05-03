#include "kernel/include/core/syscall.h"
#include "kernel/include/core/task.h"

typedef int (*sys_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

static const sys_handler_t sys_table[] = {
    [SYS_sleep] = (sys_handler_t)sys_sleep,
};
void do_handler_syscall(syscall_frame_t* frame) {

}
