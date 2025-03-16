#include "init.h"
#include "comm/boot_info.h"
#include "kernel/include/cpu/cpu.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/dev/time.h"
#include "kernel/include/tools/log.h"
#include "kernel/include/os_cfg.h"
#include "kernel/include/tools/klib.h"
#include "kernel/include/core/task.h"
#include "comm/cpu_instr.h"
#include "kernel/include/tools/list.h"

void kernel_init(boot_info_t *boot_info)
{
    // ASSERT(boot_info->ram_region_count != 0);

    cpu_init();

    log_init();

    irq_init();
    time_init();
}

static task_t first_task;
static uint32_t init_task_stack[1024];
static task_t init_task;
 
void init_task_entry(void) {
    int count = 0;
    for (;;) {
        log_printf("init task: %d", count++);
        task_switch_from_to(&init_task, &first_task);
    }
}

void list_test() {
    list_t list;
    list_init(&list);
}

void init_main(void)
{
    list_test(); // 测试list_t的初始化
    // int a = 3 / 0; // 观察异常处理流程
    // irq_enable_global();

    log_printf("Kernel is running......\n");
    log_printf("Version: %s, %s\n", OS_VERSION, "diyx86");
    log_printf("%d %d %x %c", 123, -123456, 0x12345, 'a');

    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]); // x86下，esp是向下增长的，所以这里传入的是最后一个有效地址
    task_init(&first_task, 0, 0);
    write_tr(first_task.tss_sel);

    int count = 0;
    for (;;)
    {
        log_printf("int main: %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }
}
