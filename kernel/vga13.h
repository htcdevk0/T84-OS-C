
#ifndef VGA13_H
#define VGA13_H

#include <stdint.h>

#define VGA13_WIDTH 320
#define VGA13_HEIGHT 200
#define VGA13_MEMORY 0xA0000

void vga13_init(void);
void vga13_clear(uint8_t color);
void vga13_put_pixel(int x, int y, uint8_t color);
void vga13_fill_circle(int cx, int cy, int radius, uint8_t color);
void vga13_draw_text(const char *str, int x, int y, uint8_t color);
void vga13_restore_text_mode(void);

#endif