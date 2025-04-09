SECTIONS
{
    PROVIDE(kernel_base = .);
    . = 0x10000;
    /* . = 0x100000; */

    PROVIDE(s_text = .);

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    PROVIDE(e_text = .);

    /* . = 0x200000; */

    PROVIDE(s_data = .);
    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    PROVIDE(mem_free_start = .);
}