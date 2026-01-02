ENTRY(_start)

SECTIONS {
    . = 1M;

    .multiboot2 ALIGN(8) : {
        KEEP(*(.multiboot2))
    }

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(COMMON)
        *(.bss*)
    }
}
