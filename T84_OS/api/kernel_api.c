#include "../kernel/vga.h"
#include "../kernel/keyboard.h"
#include "../kernel/string_utils.h"
#include "../kernel/panic.h"
#include "../kernel/examples/circle.h"
#include "../kernel/bootscreen.h"
#include "../kernel/variables.h"
#include "../kernel/pre_defined.h"

static bool cursor_visible = false;
static bool cursor_moved = false;
static int cursor_x = 40;
static int cursor_y = 12;
static char cursor_char = '#';

void api_move_cursor(int dx, int dy)
{
    if (!cursor_visible)
        return;

    terminal_putentryat(' ',
                        vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK),
                        cursor_x, cursor_y);

    cursor_x += dx;
    cursor_y += dy;

    if (cursor_x < 0)
        cursor_x = 0;
    if (cursor_x >= VGA_WIDTH)
        cursor_x = VGA_WIDTH - 1;
    if (cursor_y < 0)
        cursor_y = 0;
    if (cursor_y >= VGA_HEIGHT)
        cursor_y = VGA_HEIGHT - 1;

    cursor_moved = true;
}

void api_update_cursor(void)
{
    static int last_x = 40, last_y = 12;
    static bool was_visible = false;

    if (was_visible != cursor_visible)
    {
        was_visible = cursor_visible;

        if (!cursor_visible)
        {

            terminal_putentryat(' ',
                                vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK),
                                last_x, last_y);
        }
        else
        {

            cursor_moved = true;
        }
    }

    if (!cursor_visible || !cursor_moved)
        return;

    terminal_putentryat(' ',
                        vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK),
                        last_x, last_y);

    terminal_putentryat(cursor_char,
                        vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK),
                        cursor_x, cursor_y);

    last_x = cursor_x;
    last_y = cursor_y;
    cursor_moved = false;
}

void api_process_arrow_keys(void)
{
    static bool arrow_mode = false;

    if (!keyboard_available())
        return;

    uint8_t status = 0;
    __asm__ volatile("inb $0x64, %0" : "=a"(status));

    if (!(status & 0x01))
        return;

    uint8_t scancode = 0;
    __asm__ volatile("inb $0x60, %0" : "=a"(scancode));

    if (scancode == 0xE0)
    {
        arrow_mode = true;
        return;
    }

    if (arrow_mode)
    {
        arrow_mode = false;

        switch (scancode)
        {
        case 0x48:
            api_move_cursor(0, -1);
            break;
        case 0x50:
            api_move_cursor(0, 1);
            break;
        case 0x4B:
            api_move_cursor(-1, 0);
            break;
        case 0x4D:
            api_move_cursor(1, 0);
            break;
        }
    }
}