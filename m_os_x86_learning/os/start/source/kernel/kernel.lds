SECTIONS
{
    . = 0x10000;
    /* . = 0x100000; */

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    /* . = 0x200000; */

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}