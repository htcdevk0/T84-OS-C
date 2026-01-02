#include "vga.h"
#include "string_utils.h"
#include <stdarg.h>

size_t terminal_row = 0;
size_t terminal_column = 0;
uint8_t terminal_color = 0;
uint16_t *terminal_buffer = (uint16_t *)VGA_BUFFER;

void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t *)VGA_BUFFER;

    terminal_clear();
}

void terminal_clear(void)
{
    for (size_t y = 0; y < VGA_HEIGHT; y++)
    {
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_scroll(void)
{

    for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
    {
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            const size_t current_index = (y + 1) * VGA_WIDTH + x;
            const size_t new_index = y * VGA_WIDTH + x;
            terminal_buffer[new_index] = terminal_buffer[current_index];
        }
    }

    for (size_t x = 0; x < VGA_WIDTH; x++)
    {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}

void terminal_newline(void)
{
    terminal_column = 0;
    if (++terminal_row >= VGA_HEIGHT)
    {
        terminal_scroll();
        terminal_row = VGA_HEIGHT - 1;
    }
}

void terminal_putchar(char c)
{
    if (c == '\n')
    {
        terminal_newline();
        return;
    }

    if (c == '\b')
    {
        if (terminal_column > 0)
        {
            terminal_column--;
        }
        else if (terminal_row > 0)
        {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        return;
    }

    if (c == '\t')
    {
        terminal_column = (terminal_column + 8) & ~(8 - 1);
        if (terminal_column >= VGA_WIDTH)
        {
            terminal_newline();
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);

    if (++terminal_column == VGA_WIDTH)
    {
        terminal_newline();
    }
}

void terminal_write(const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char *data)
{
    size_t len = 0;
    while (data[len])
        len++;
    terminal_write(data, len);
}

void terminal_backspace(void)
{
    if (terminal_column > 0)
    {
        terminal_column--;
    }
    else if (terminal_row > 0)
    {
        terminal_row--;
        terminal_column = VGA_WIDTH - 1;
    }
    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
}

void terminal_set_cursor(size_t x, size_t y)
{
    if (x >= VGA_WIDTH)
        x = VGA_WIDTH - 1;
    if (y >= VGA_HEIGHT)
        y = VGA_HEIGHT - 1;

    terminal_column = x;
    terminal_row = y;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
        return;
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

size_t terminal_get_row(void)
{
    return terminal_row;
}

size_t terminal_get_column(void)
{
    return terminal_column;
}

bool terminal_can_scroll(void)
{
    return terminal_row >= VGA_HEIGHT - 1;
}

void terminal_writeall(const char *format, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    terminal_writestring(buffer);
}
void draw_line(int x, int y, int length, uint8_t color, bool horizontal)
{
    if (horizontal)
    {

        if (y < 0 || y >= VGA_HEIGHT)
            return;

        for (int i = 0; i < length; i++)
        {
            int current_x = x + i;
            if (current_x >= 0 && current_x < VGA_WIDTH)
            {
                terminal_putentryat(0xCD, color, current_x, y);
            }
        }
    }
    else
    {

        if (x < 0 || x >= VGA_WIDTH)
            return;

        for (int i = 0; i < length; i++)
        {
            int current_y = y + i;
            if (current_y >= 0 && current_y < VGA_HEIGHT)
            {
                terminal_putentryat(0xBA, color, x, current_y);
            }
        }
    }
}

void terminal_clear_color(uint8_t color)
{
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}