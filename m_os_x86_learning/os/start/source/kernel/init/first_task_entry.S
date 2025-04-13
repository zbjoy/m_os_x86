	.text
	.global first_task_entry
	.extern load_kernel
first_task_entry:
	; 第一个任务入口函数
	mov %ss, %ax 
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	; jmp $8, $load_kernel
    ; 不在使用选择子来跳转了
    ; 直接跳转到first_task.c中的代码中
    jmp 