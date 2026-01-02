.section .text
.global kernel_main
.extern main

kernel_main:
    push %ebx
    push %eax
    
    call main
    
    cli
1:  hlt
    jmp 1b