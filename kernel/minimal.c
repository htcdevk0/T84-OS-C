#include "vga.h"
#include "keyboard.h"
#include "string_utils.h"
#include "panic.h"
#include "examples/circle.h"
#include "bootscreen.h"
#include "variables.h"
#include "pre_defined.h"
#include "ramfs.h"
#include "commands.h"
#include "misc.h"
#include "tlang.h"

#define kernel_running 1

void kernel_minimal_main(void) {
    uint8_t main_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_setcolor(main_color);
    terminal_writestring("Welcome to T84 OS - Minimal Version.\n");
    terminal_writestring("type 'help' to see the avaliable commands.\n");

    while (kernel_running) {
        
    }
}