  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start, loader_entry
_start:
	jmp loader_entry

	.code32
	.text
	.global protect_mode_entry
	.extern load_kernel
protect_mode_entry:
	// 保护模式入口函数
	mov $16, %ax // 切换到32位模式
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	jmp $8, $load_kernel
