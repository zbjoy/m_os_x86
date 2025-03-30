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
#include "kernel/include/ipc/sem.h"

void kernel_init(boot_info_t *boot_info)
{
    // ASSERT(boot_info->ram_region_count != 0);

    cpu_init();

    log_init();

    irq_init();
    time_init();

    task_manager_init();
}

static task_t first_task;
static uint32_t init_task_stack[1024];
static task_t init_task;
static sem_t sem;
 
void init_task_entry(void) {
    int count = 0;
    for (;;) {
        // sem_wait(&sem);
        log_printf("init task: %d", count++);
        // sys_sleep(3000);
        // task_switch_from_to(&init_task, task_first_task());
        // sys_sched_yield();
    }
}

void list_test() {
    list_t list;
    list_init(&list);
    log_printf("list_test: first=0x%x, last=0x%x, list_is_empty=%d, list_count=%d", list_first(&list), list_last(&list), list_is_empty(&list), list_count(&list));

    list_node_t nodes[5];
    for (int i = 0; i < 5; i++) {
        list_node_t* node = nodes + i;
        log_printf("insert first to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_first(&list, node);
    }
    log_printf("list_test: first=0x%x, last=0x%x, list_is_empty=%d, list_count=%d", list_first(&list), list_last(&list), list_is_empty(&list), list_count(&list));

    list_init(&list);
    for (int i = 0; i < 5; i++) {
        list_node_t* node = nodes + i;
        log_printf("insert last to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_last(&list, node);
    }
    log_printf("list_test: first=0x%x, last=0x%x, list_is_empty=%d, list_count=%d", list_first(&list), list_last(&list), list_is_empty(&list), list_count(&list));

    for (int i = 0; i < 5; i++)  {
        list_node_t* node = list_remove_first(&list);
        log_printf("remove first from list: %d, 0x%x", i, (uint32_t)node);
    }
    log_printf("list_test: first=0x%x, last=0x%x, list_is_empty=%d, list_count=%d", list_first(&list), list_last(&list), list_is_empty(&list), list_count(&list));

    list_init(&list);
    for (int i = 0; i < 5; i++) {
        list_node_t* node = nodes + i;
        log_printf("insert last to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_last(&list, node);
    }
    log_printf("list_test: first=0x%x, last=0x%x, list_is_empty=%d, list_count=%d", list_first(&list), list_last(&list), list_is_empty(&list), list_count(&list));

    for (int i = 0; i < 5; i++) {
        list_node_t* node = nodes + i;
        log_printf("remove last from list: %d, 0x%x", i, (uint32_t)node);
        list_remove(&list, node);
    }
    log_printf("list_test: first=0x%x, last=0x%x, list_is_empty=%d, list_count=%d", list_first(&list), list_last(&list), list_is_empty(&list), list_count(&list));

    struct type_t {
        int i;
        list_node_t node;
    } v = {0x123456};

    uint32_t addr = (uint32_t)&((struct type_t*)0)->node;
    uint32_t parent_addr = offset_in_parent(struct type_t, node);

    list_node_t* v_node = &v.node;
    struct type_t* p = list_node2parent(v_node, struct type_t, node);
    if (p->i == 0x123456) {
        log_printf("list_node2parent: success");
    }
}

void init_main(void)
{
    // list_test(); // 测试list_t的初始化


    // int a = 3 / 0; // 观察异常处理流程
    // irq_enable_global();

    log_printf("Kernel is running......\n");
    log_printf("Version: %s, %s\n", OS_VERSION, "diyx86");
    log_printf("%d %d %x %c", 123, -123456, 0x12345, 'a');

    task_init(&init_task, "init task", (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]); // x86下，esp是向下增长的，所以这里传入的是最后一个有效地址
    // task_init(&first_task, 0, 0);
    // write_tr(first_task.tss_sel);
    task_first_init();

    sem_init(&sem, 0);

    irq_enable_global();

    int count = 0;
    for (;;)
    {
        log_printf("int main: %d", count++);
        // sem_notify(&sem);
        // sys_sleep(1000);
        // task_switch_from_to(task_first_task(), &init_task);
        // sys_sched_yield();
    }
}
