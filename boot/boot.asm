bits 32

section .multiboot2
align 8

header_start:
    dd 0xE85250D6          ; Multiboot2 magic
    dd 0                  ; architecture (i386)
    dd header_end - header_start
    dd -(0xE85250D6 + 0 + (header_end - header_start))

    dw 0                  ; type
    dw 0                  ; flags
    dd 8                  ; size
header_end:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    and esp, 0xFFFFFFF0

    call kernel_main

    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
