#!/bin/bash
set -e

echo "=== Building T84 OS ==="

rm -rf build isodir T84-OS.iso
mkdir -p build isodir/boot/grub
mkdir -p build/apps
mkdir -p build/api

nasm -f elf32 boot/boot.asm -o build/boot.o

gcc -m32 -ffreestanding -c kernel/kernel.c -o build/kernel.o -nostdlib -fno-builtin
gcc -m32 -ffreestanding -c kernel/vga.c -o build/vga.o -nostdlib -fno-builtin
gcc -m32 -ffreestanding -c kernel/keyboard.c -o build/keyboard.o -nostdlib -fno-builtin
gcc -m32 -ffreestanding -c kernel/string_utils.c -o build/string_utils.o -nostdlib -fno-builtin
gcc -m32 -ffreestanding -c kernel/panic.c -o build/panic.o -nostdlib -fno-builtin
gcc -m32 -ffreestanding -c kernel/examples/circle.c -o build/circle.o -nostdlib -fno-builtin
gcc -m32 -ffreestanding -c kernel/vga13.c -o build/vga13.o -nostdlib -fno-builtin
gcc -m32 -ffreestanding -c kernel/vga_direct.c -o build/vga_direct.o -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c kernel/bootscreen.c -o build/bootscreen.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c kernel/variables.c -o build/variables.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c kernel/pre_defined.c -o build/pre_defined.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c T84_OS/home/app/ttest.c -o build/apps/ttest.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c T84_OS/api/kernel_api.c -o build/api/kernel_api.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c kernel/ramfs.c -o build/ramfs.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c kernel/misc.c -o build/misc.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c T84_OS/home/app/4IDE.c -o build/apps/4IDE.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c kernel/tlang.c -o build/tlang.o -I./kernel -nostdlib -fno-builtin
gcc -m32 -ffreestanding -O2 -c T84_OS/home/app/cstat.c -o build/apps/cstat.o -I./kernel -nostdlib -fno-builtin

ld -m elf_i386 \
  -T linker.ld \
  -nostdlib \
  -o build/kernel.elf \
  build/boot.o \
  build/kernel.o \
  build/vga.o \
  build/keyboard.o \
  build/string_utils.o \
  build/panic.o \
  build/circle.o \
  build/vga13.o \
  build/vga_direct.o \
  build/bootscreen.o \
  build/variables.o \
  build/pre_defined.o \
  build/apps/ttest.o \
  build/api/kernel_api.o \
  build/ramfs.o \
  build/misc.o \
  build/apps/4IDE.o \
  build/tlang.o \
  build/apps/cstat.o

cp build/kernel.elf isodir/boot/

cat > isodir/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "T84 OS" {
    insmod multiboot2
    multiboot2 /boot/kernel.elf
    boot
}
EOF

grub-mkrescue -o T84-OS.iso isodir