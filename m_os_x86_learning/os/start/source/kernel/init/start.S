#include "kernel/include/os_cfg.h"
# #include "kernel/include/cpu/irq.h"

    .text
    .extern kernel_init, init_main
    .global _start

_start:
    push %ebp             # save the base pointer 
    mov %esp, %ebp        # set the base pointer to the current stack pointer
    mov 0x8(%ebp), %eax  # get the first argument (kernel_init)

    # 上面的三步可以一步实现 (在不考虑重新回到loader的情况下)
    # mov 4(%ebp), %eax    # get the first argument (kernel_init)

    push %eax             # push the first argument (kernel_init)
    call kernel_init

    jmp $KERNEL_SELECTOR_CS, $gdt_reload # 跳转到内核初始化函数, 并重新设置段寄存器

gdt_reload:
    # 重新设置段寄存器
    mov $KERNEL_SELECTOR_DS, %ax
    mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

    mov $(stack + KERNEL_STACK_SIZE), %esp
    jmp init_main

    .bss
.comm stack, KERNEL_STACK_SIZE

.macro exception_handler name num with_err_code
    .text # 代码
    .extern do_handler_\name # 未知异常处理函数声明
    .global exception_handler_\name # 未知异常处理函数实现
exception_handler_\name:

    # 判断是否有错误码
    .if \with_err_code == 0 # 没有错误码
        push $0 # 压入0作为错误码 (即没有错误码)
    .endif

    # 压入中断序号
    push $\num

    pusha
    push %ds
    push %es
    push %fs
    push %gs

    push %esp # 将异常发生时的栈指针压栈

    call do_handler_\name # 调用具体的异常处理函数

    add $(1*4), %esp # 弹出异常发生时的栈指针, 通过直接修改esp指针实现跳过4个字节达到出栈效果

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa

    add $(2*4), %esp # 弹出错误码和中断序号

    iret # 异常返回
.endm

exception_handler unknown, -1, 0
exception_handler divider, 0, 0
exception_handler Debug, 1, 0
exception_handler NMI, 2, 0
exception_handler breakpoint, 3, 0
exception_handler overflow, 4, 0
exception_handler bound_range, 5, 0
exception_handler invalid_opcode, 6, 0
exception_handler device_unavailable, 7, 0
exception_handler double_fault, 8, 1
exception_handler invalid_tss, 10, 1
exception_handler segment_not_present, 11, 1
exception_handler stack_segment_fault, 12, 1
exception_handler general_protection, 13, 1
exception_handler page_fault, 14, 1
exception_handler fpu_error, 16, 0
exception_handler alignment_check, 17, 1
exception_handler machine_check, 18, 0
exception_handler simd_exception, 19, 0
exception_handler virtual_exception, 20, 0
exception_handler control_exception, 21, 1

exception_handler time, 0x20, 0

    // simple_switch(&from, to)
    .text
    .global simple_switch
simple_switch:
    // 从参数中(在栈中保存)取出值
    mov 4(%esp), %eax // 也可以放在 eax 中, from 的 值
    mov 8(%esp), %edx // 也可以放在 edx 中, to 的 值


    push %ebp
    push %ebx
    push %esi
    push %edi
    mov %esp, (%eax) // 保存当前栈指针到 from 栈中, 内存写

    // 实现栈的切换
    mov %edx, %esp


    pop %edi
    pop %esi
    pop %ebx
    pop %ebp

    // ret 指令会从当前的栈指针中弹出返回地址, 并跳转到该地址, 而在 source/kernel/core/task.c 中的 task_init 函数中有设置 *(--pesp) = entry, 所以这里的 ret 指令会返回到 task_init 函数中, 并设置好新的栈, 然后调用 task_start 函数
    ret // 对 栈 进行设置 使返回到 to 的函数中

