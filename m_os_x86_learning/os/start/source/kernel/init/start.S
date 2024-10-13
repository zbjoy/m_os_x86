    .text
    .extern kernel_init
    .global _start

_start:
    push %ebp             # save the base pointer 
    mov %esp, %ebp        # set the base pointer to the current stack pointer
    mov 0x8(%ebp), %eax  # get the first argument (kernel_init)

    # 上面的三步可以一步实现 (在不考虑重新回到loader的情况下)
    # mov 4(%ebp), %eax    # get the first argument (kernel_init)

    push %eax             # push the first argument (kernel_init)
    call kernel_init
    jmp .