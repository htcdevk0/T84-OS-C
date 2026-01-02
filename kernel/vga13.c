
#include "vga13.h"
#include "ports.h"
#include "string_utils.h"

static uint8_t *vga_mem = (uint8_t *)VGA13_MEMORY;

void vga13_init(void)
{
    __asm__ volatile("mov $0x13, %%ax\n\t"
                     "int $0x10\n\t" ::: "eax", "memory");
}

void vga13_clear(uint8_t color)
{
    for (int i = 0; i < VGA13_WIDTH * VGA13_HEIGHT; i++)
    {
        vga_mem[i] = color;
    }
}

void vga13_put_pixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= VGA13_WIDTH || y < 0 || y >= VGA13_HEIGHT)
    {
        return;
    }

    vga_mem[y * VGA13_WIDTH + x] = color;
}

static void draw_scanlines(int cx, int x, int y, int current_y, uint8_t color)
{

    int start_x = cx - x;
    int end_x = cx + x;

    if (start_x < 0)
        start_x = 0;
    if (end_x >= VGA13_WIDTH)
        end_x = VGA13_WIDTH - 1;

    if (current_y + y >= 0 && current_y + y < VGA13_HEIGHT)
    {
        for (int px = start_x; px <= end_x; px++)
        {
            vga_mem[(current_y + y) * VGA13_WIDTH + px] = color;
        }
    }

    if (current_y - y >= 0 && current_y - y < VGA13_HEIGHT)
    {
        for (int px = start_x; px <= end_x; px++)
        {
            vga_mem[(current_y - y) * VGA13_WIDTH + px] = color;
        }
    }
}

void vga13_fill_circle(int cx, int cy, int radius, uint8_t color)
{
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y)
    {

        draw_scanlines(cx, x, y, cy, color);
        draw_scanlines(cx, y, x, cy, color);

        if (d < 0)
        {
            d = d + 4 * x + 6;
        }
        else
        {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void vga13_draw_text(const char *str, int x, int y, uint8_t color)
{

    uint16_t cursor_func = 0x0200;
    uint8_t cursor_pos = (y << 8) | x;

    __asm__ volatile("int $0x10"
                     :
                     : "a"(cursor_func), "b"(0), "d"(cursor_pos)
                     : "memory");

    for (int i = 0; str[i] != '\0'; i++)
    {
        uint16_t write_func = 0x0E00 | str[i];
        __asm__ volatile("int $0x10"
                         :
                         : "a"(write_func), "b"(color)
                         : "memory");
    }
}

void vga13_restore_text_mode(void)
{
    __asm__ volatile("mov $0x03, %%ax\n\t"
                     "int $0x10\n\t" ::: "eax", "memory");
}