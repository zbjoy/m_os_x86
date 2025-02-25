#include "init.h"
#include "comm/boot_info.h"
#include "kernel/include/cpu/cpu.h"
#include "kernel/include/cpu/irq.h"
#include "kernel/include/dev/time.h"
#include "kernel/include/tools/log.h"
#include "kernel/include/os_cfg.h"

void kernel_init(boot_info_t *boot_info)
{
    cpu_init();

    log_init();

    irq_init();
    time_init();
}

void init_main(void)
{
    // int a = 3 / 0; // 观察异常处理流程
    // irq_enable_global();

    log_printf("Kernel is running......\n");
    log_printf("Version: %s\n", OS_VERSION);

    for (;;)
    {

    }
}
