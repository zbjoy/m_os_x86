#include "boot.h"

  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start, boot_entry
_start:
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	// 将程序加载到 栈顶
	mov $_start, %esp	// esp 寄存器指向栈顶

	mov $0xE, %ah
	mov $'L', %al
	INT $0x10	// 输出字符 'L'

read_loader:
	mov $0x02, %ah
	mov $0x64, %al
	mov $0x02, %cx
	mov $0x80, %dx
	mov $0x8000, %bx
	INT $0x13	// 读取 loader.bin 到内存 0x8000
	jc read_loader	// 读取失败，重试

	// 跳转到 boot.c 中的 boot_entry() 函数
	jmp boot_entry

	jmp .
	
	// 引导结束段
	.section boot_end, "ax"
boot_sig: .byte 0x55, 0xaa
