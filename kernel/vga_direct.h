#ifndef VGA_DIRECT_H
#define VGA_DIRECT_H

#include <stdint.h>

void vga_mode_text(void);
void vga_mode_13h(void);

void vga_clear(uint8_t color);
void vga_put_pixel(int x, int y, uint8_t color);
void vga_draw_circle(int cx, int cy, int radius, uint8_t color);

void vga_write_reg(uint16_t port, uint8_t value);
uint8_t vga_read_reg(uint16_t port);

#endif