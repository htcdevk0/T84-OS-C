#include "panic.h"
#include "vga.h"
#include "string_utils.h"
#include "ports.h"

void show_panic_screen(const char *panic_msg, uint32_t error_code)
{

    __asm__ volatile("cli");

    terminal_clear_color(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY));

    for (int x = 0; x < VGA_WIDTH; x++)
    {
        terminal_putentryat(0xCD,
                            vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY),
                            x, 2);
    }

    int title_y = 4;
    int title_x = (VGA_WIDTH - 12) / 2;

    terminal_setcolor(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY));

    terminal_putentryat('K', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y);
    terminal_putentryat('K', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + 1);
    terminal_putentryat('K', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + 2);
    terminal_putentryat('K', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('K', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 2);

    title_x += 3;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('E', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('E', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('E', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 2);

    title_x += 3;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('R', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('R', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('R', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 1);

    title_x += 3;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('N', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('N', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('N', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 2);

    title_x += 3;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('E', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('E', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('E', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 2);

    title_x += 3;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('L', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('L', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 2);

    title_x += 3;

    title_x += 1;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('P', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('P', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('P', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 1);

    title_x += 3;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('A', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('A', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('A', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 1);

    title_x += 3;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('N', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('N', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('N', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 2);

    title_x += 3;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('I', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }

    title_x += 2;
    for (int y = 0; y < 3; y++)
    {
        terminal_putentryat('C', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x, title_y + y);
    }
    terminal_putentryat('C', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y);
    terminal_putentryat('C', vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY), title_x + 1, title_y + 2);

    for (int x = 0; x < VGA_WIDTH; x++)
    {
        terminal_putentryat(0xCD,
                            vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY),
                            x, 8);
    }

    terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY));

    terminal_set_cursor(10, 11);
    terminal_writestring("This screen indicates a sudden failure in the [T84] kernel.");

    terminal_set_cursor(5, 13);
    terminal_writestring("To exit this screen, you must turn off your computer using");

    terminal_set_cursor(5, 14);
    terminal_writestring("the power button usually located on your computer case.");

    if (panic_msg && strlen(panic_msg) > 0)
    {
        terminal_set_cursor((VGA_WIDTH - strlen(panic_msg)) / 2, 16);
        terminal_writestring(panic_msg);
    }

    if (error_code != 0xFFFFFFFF)
    {
        char error_str[32];
        terminal_set_cursor(10, 18);
        terminal_writestring("Error code: 0x");

        char hex_chars[] = "0123456789ABCDEF";
        for (int i = 28; i >= 0; i -= 4)
        {
            uint8_t nibble = (error_code >> i) & 0xF;
            terminal_putchar(hex_chars[nibble]);
        }
    }

    terminal_set_cursor((VGA_WIDTH - 20) / 2, VGA_HEIGHT - 2);
    terminal_writestring("T84 Kernel, 2025.");

    for (int x = 0; x < VGA_WIDTH; x++)
    {
        terminal_putentryat(0xC4,
                            vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY),
                            x, VGA_HEIGHT - 4);
    }

    while (1)
    {
        __asm__ volatile("hlt");
    }
}

void panic(const char *message)
{
    show_panic_screen(message, 0xFFFFFFFF);
}

void panic_with_code(const char *message, uint32_t error_code)
{
    show_panic_screen(message, error_code);
}