#include "bootscreen.h"
#include "vga.h"
#include "string_utils.h"
#include "ports.h"

void delay_ms(int milliseconds)
{

    volatile int loops = milliseconds * 9000 << 3;
    for (volatile int i = 0; i < loops; i++)
        ;
}

static bool bootscreen_active = false;
static int current_line = 0;
static int progress_percent = 0;

void bootscreen_init(void)
{

    terminal_clear_color(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BLACK));

    delay_ms(1000);

    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));

    terminal_set_cursor(0, 0);
    for (int i = 0; i < VGA_WIDTH; i++)
    {
        terminal_putchar('=');
        delay_ms(5);
    }

    delay_ms(1000);

    terminal_set_cursor((VGA_WIDTH - 15) / 2, 2);

    const char *title = "T84 OS BOOT MENU";
    for (int i = 0; title[i]; i++)
    {
        terminal_putchar(title[i]);
        delay_ms(50);
    }

    delay_ms(300);

    terminal_set_cursor(0, 3);
    for (int i = 0; i < VGA_WIDTH; i++)
    {
        terminal_putchar('=');
        delay_ms(5);
    }

    current_line = 5;
    bootscreen_active = true;
    progress_percent = 0;

    delay_ms(500);
}

void bootscreen_add_item(const char *name, bool success)
{
    if (!bootscreen_active)
        return;

    delay_ms(150);

    if (current_line >= VGA_HEIGHT - 10)
    {

        for (int y = 5; y < VGA_HEIGHT - 5; y++)
        {
            for (int x = 0; x < VGA_WIDTH; x++)
            {
                uint16_t char_above = terminal_buffer[(y + 1) * VGA_WIDTH + x];
                terminal_buffer[y * VGA_WIDTH + x] = char_above;
            }
        }
        current_line--;
        delay_ms(100);
    }

    terminal_set_cursor(2, current_line);
    terminal_setcolor(BOOT_COLOR_TEXT);

    for (int i = 0; name[i]; i++)
    {
        terminal_putchar(name[i]);
        delay_ms(30);
    }

    int name_len = strlen(name);
    int status_pos = VGA_WIDTH - 10;

    terminal_set_cursor(name_len + 3, current_line);
    for (int i = name_len + 3; i < status_pos - 1; i++)
    {
        terminal_putchar('.');
        delay_ms(10);
    }

    delay_ms(300);

    terminal_set_cursor(status_pos, current_line);
    if (success)
    {

        terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_GREEN));
        terminal_writestring("[    ]");

        delay_ms(100);

        terminal_set_cursor(status_pos + 2, current_line);
        terminal_putchar('O');
        delay_ms(50);
        terminal_set_cursor(status_pos + 3, current_line);
        terminal_putchar('K');

        delay_ms(100);

        terminal_setcolor(BOOT_COLOR_OK);
        terminal_set_cursor(status_pos, current_line);
        terminal_writestring("[ OK ]");
    }
    else
    {

        terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_RED));
        terminal_writestring("[    ]");

        delay_ms(100);

        terminal_set_cursor(status_pos + 2, current_line);
        terminal_putchar('F');
        delay_ms(50);
        terminal_set_cursor(status_pos + 3, current_line);
        terminal_putchar('A');
        delay_ms(50);
        terminal_set_cursor(status_pos + 4, current_line);
        terminal_putchar('I');
        delay_ms(50);
        terminal_set_cursor(status_pos + 5, current_line);
        terminal_putchar('L');

        delay_ms(100);

        terminal_setcolor(BOOT_COLOR_FAIL);
        terminal_set_cursor(status_pos, current_line);
        terminal_writestring("[FAIL]");
    }

    delay_ms(400);

    current_line++;
}

void bootscreen_update_progress(int percent)
{
    if (!bootscreen_active)
        return;

    if (percent < 0)
        percent = 0;
    if (percent > 100)
        percent = 100;

    delay_ms(50);

    progress_percent = percent;

    int bar_width = VGA_WIDTH - 10;
    int filled = (bar_width * percent) / 100;

    terminal_set_cursor(5, VGA_HEIGHT - 6);
    terminal_setcolor(BOOT_COLOR_TEXT);
    terminal_writestring("Loading: [");

    terminal_setcolor(BOOT_COLOR_PROGRESS);
    static int last_filled = 0;

    if (filled > last_filled)
    {
        for (int i = last_filled; i < filled; i++)
        {
            terminal_putchar('=');
            delay_ms(20);
        }
        last_filled = filled;
    }

    terminal_setcolor(BOOT_COLOR_TEXT);
    for (int i = filled; i < bar_width; i++)
    {
        terminal_putchar(' ');
    }

    terminal_putchar(']');

    char percent_str[8];
    itoa(percent, percent_str, 10);
    terminal_writestring(" ");
    terminal_writestring(percent_str);
    terminal_writestring("%");

    delay_ms(100);
}

void bootscreen_complete(void)
{
    if (!bootscreen_active)
        return;

    delay_ms(800);

    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            if (y > 0)
            {
                terminal_buffer[(y - 1) * VGA_WIDTH + x] =
                    terminal_buffer[y * VGA_WIDTH + x];
            }
        }
        delay_ms(10);
    }

    for (int x = 0; x < VGA_WIDTH; x++)
    {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            vga_entry(' ', vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BLACK));
    }

    delay_ms(500);

    int center_y = VGA_HEIGHT / 2 - 3;

    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));

    delay_ms(300);

    terminal_set_cursor(35, center_y);
    terminal_writestring("TTTTT");
    delay_ms(150);

    terminal_set_cursor(37, center_y + 1);
    terminal_writestring("T");
    delay_ms(100);

    terminal_set_cursor(37, center_y + 2);
    terminal_writestring("T");
    delay_ms(150);

    terminal_set_cursor(43, center_y);
    terminal_writestring("888888");
    delay_ms(150);

    terminal_set_cursor(43, center_y + 1);
    terminal_writestring("    88");
    delay_ms(150);

    terminal_set_cursor(43, center_y + 2);
    terminal_writestring("888888");
    delay_ms(200);

    terminal_set_cursor(50, center_y);
    terminal_writestring("4444");
    delay_ms(150);

    terminal_set_cursor(50, center_y + 1);
    terminal_writestring("  44");
    delay_ms(150);

    terminal_set_cursor(50, center_y + 2);
    terminal_writestring("444444");
    delay_ms(200);

    terminal_set_cursor(57, center_y);
    terminal_writestring(" OOO ");
    delay_ms(150);

    terminal_set_cursor(57, center_y + 1);
    terminal_writestring("O   O");
    delay_ms(150);

    terminal_set_cursor(57, center_y + 2);
    terminal_writestring(" SSS ");
    delay_ms(200);

    terminal_set_cursor(34, center_y + 4);
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));

    const char *version = "Version 1.0 - Complete Terminal System";
    for (int i = 0; version[i]; i++)
    {
        terminal_putchar(version[i]);
        delay_ms(30);
    }

    delay_ms(500);

    terminal_set_cursor(20, center_y + 6);
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("[");

    for (int i = 0; i < 40; i++)
    {
        terminal_putchar('=');

        delay_ms(80);

        terminal_set_cursor(62, center_y + 6);
        char percent_str[8];
        int percent = (i + 1) * 2;
        itoa(percent, percent_str, 10);
        terminal_writestring(percent_str);
        terminal_writestring("%");
    }

    terminal_writestring("]");

    delay_ms(300);

    terminal_set_cursor(30, center_y + 8);
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));

    const char *final_msg = "System ready. Starting kernel...";
    for (int i = 0; final_msg[i]; i++)
    {
        terminal_putchar(final_msg[i]);
        delay_ms(40);
    }

    delay_ms(2000);

    bootscreen_active = false;
}

void bootscreen_show(void)
{
    bootscreen_init();
}