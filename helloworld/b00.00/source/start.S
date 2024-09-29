	#include "os.h"

	// 声明本地以下符号是全局的，在其它源文件中可以访问
	.global _start, timer_int, syscall_handler

	.extern os_init

	// 指定以下的代码生成16位的机器指令，这样才能在启动时的实模式下运行
  	.code16

	// 以下是代码区
 	.text
_start: 
	// 将 cs 置为 0
	jmp $0, $offset
offset:

	// 将 实模式下 的 段寄存器 全部设置为 0 为 基地址(平坦模式)
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	// TODO: 不理解, 为什么 要把 _start 存到 esp(32位) 寄存器中
	mov $_start, %esp
	
read_self_all:

	// 读取 剩余 代码 (os.c)
	mov $_start_32, %bx // 读取到内存的地址, 相当于 0x7E00
	mov $0x2, %cx  	 	// 读取扇区的开始位置
	mov $0x240, %ax	 	// 读取的大小为 0x240(576) - 0x200(512) = 0x40(64) 个 扇区
	mov $0x80, %dx	 	// 表示 计算器上的 第一块 磁盘 (对于 qemu)
	int $0x13		 	// 通过 INT 13 x86的软中断 跳转到 BIOS中断提供的 BIOS代码 的 读取 磁盘的 函数位置

	jc read_self_all	// 判断是否成功, 失败重新调用 BIOS提供的读取磁盘的代码

	cli 				// 关中断
	lgdt gdt_desc 		// 通过 lgdt 指令 将 os.c 中 定义的 gdt_table 的 地址读取进来
	lidt idt_desc		// 通过 lidt 指令 将 os.c 中 定义的 idt_table 的 地址读取进来
	mov $1, %eax		// CR0 是 32位, 所以 给到 exa
	lmsw %ax			// 会将 CR0 寄存器 中 的 PE(最低位) 置为 1, 开启 保护模式
	jmp $KERNEL_CODE_SEG, $_start_32	// 原来 为 16位 实模式, 要进入 保护模式, 需要 做一些 跳转 工作(会 清除 一些 流水线 等)

	// 510处  最后 两位 放 0xAA 与 0x55
	.org 0x1FE     
	.byte 0x55, 0xAA

	.code32 	// 后面 跟 32 位 的指令
	.text		// 恢复 为 之前 状态

_start_32:
	// .fill 64*1024, 1, 0x35  // 填充 64KB 数据, 每个为 1Byte, 数据为 0x35
	
	// 将 寄存器 中的 值 赋上 对应 的 索引值
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov $_start, %esp

	call os_init

	// 将 pg_dir 设置 到 cr3 寄存器中 (将 cr3 寄存器 的 Page Address 设置为了 pg_dir 的 地址)
	mov $pg_dir, %eax
	mov %eax, %cr3

	// 设置 cr4 寄存器 的 第四位 PSE 使 允许 4M 到 4M 的 映射
	mov %cr4, %eax
	orl $(1 << 4), %eax
	mov %eax, %cr4

	// 将 cr0 最高位 置为 1, 使 打开 分页机制
	mov %cr0, %eax
	orl $(1 << 31), %eax
	mov %eax, %cr0

	// sti // 打开中断
	
	// jmp .

	mov $TASK0_TSS_SEG, %ax
	ltr %ax								// 告诉 CPU 当前运行的 是 TASK0

	push $APP_DATA_SEG 	 				// 保存 APP_DATA_SEG 到栈中 (放入 ss 寄存器)
	// push $0		   					// 给 一定 的 栈空间
	push $task0_dpl3_stack + 1024 * 4  	// 给到 栈的 地址 (给 数组 的 末尾) (给到 ESP 寄存器)
	// push $0 // $0x202							// 给到 EFLAGS 寄存器, 给相应位 置1 打开了 中断(0x202)
	push $0x202							// 打开 中断
	push $APP_CODE_SEG 					// 给到 CS 寄存器
	push $task_0_entry 					// 给到 EIP 寄存器	
	iret 			   					// 结束中断, 重新执行 被中断的 指令


syscall_handler:
	// 保存 寄存器 的 值
	push %ds
	pusha

	// 将 ds 寄存器 中的 值 切换为 内核数据段
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds

	// 将 对应 参数的 值 压入 栈中
	// push color
	// push str
	// push func

	// ================= DEBUG START ====================

	push $1
	push $2
	push $3

	// ================= DEBUG END   ====================
	call do_syscall

	popa
	pop %ds

	ret

// 任务 0
task_0_entry:
	mov %ss, %ax // 保存 ss 寄存器 中的 值
	mov %ax, %ds // 将 ss 寄存器 中的 值 赋给 ds 寄存器
	mov %ax, %es // 将 ss 寄存器 中的 值 赋给 es 寄存器
	mov %ax, %fs // 将 ss 寄存器 中的 值 赋给 fs 寄存器
	mov %ax, %gs // 将 ss 寄存器 中的 值 赋给 gs 寄存器
	// jmp .
	jmp task_0	 // 跳转到 c语言 的 对应函数

timer_int:
	push %ds // 保存 ds 寄存器 中的 值
	pusha	 // 保存 ax, bx, cx, dx, ... 等 寄存器的 值

	mov $0x20, %al
	outb %al, $0x20

	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds

	call task_sched	// 进行 进程间切换(调用对应的 C代码)

	popa  	 // 恢复 ax, bx, cx, dx, ... 等 寄存器的 值
	pop %ds  // 恢复 ds 寄存器 中的 值

	iret // 结束中断, 重新执行 被中断的 指令

gdt_desc:
	.word (256*8) - 1  	 // 界限, gdt表项 定义了 256 个, 每个为 8 字节(uint_8), 因为 界限 不是大小, 从 0 开始, 所以 要 减去 1
	.long gdt_table		 // 起始地址, gdt_table 的 地址

idt_desc:
	.word (256*8) - 1	 
	.long idt_table		 // idt_table 的 地址
