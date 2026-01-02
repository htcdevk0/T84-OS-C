#ifndef BOOTSCREEN_H
#define BOOTSCREEN_H

#include <stdint.h>
#include <stdbool.h>

void delay_ms(int milliseconds);

void bootscreen_init(void);
void bootscreen_show(void);
void bootscreen_add_item(const char *name, bool success);
void bootscreen_update_progress(int percent);
void bootscreen_complete(void);

#define BOOT_COLOR_OK vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK)
#define BOOT_COLOR_FAIL vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK)
#define BOOT_COLOR_TEXT vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK)
#define BOOT_COLOR_PROGRESS vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK)

#endif