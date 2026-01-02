#include "../vga.h"
#include "../keyboard.h"
#include "../string_utils.h"

void circle_example_run(void)
{

    size_t saved_row = terminal_row;
    size_t saved_col = terminal_column;
    uint8_t saved_color = terminal_color;

    terminal_clear();

    int center_x = 40;
    int center_y = 12;
    int radius = 10;

    const uint8_t block_chars[] = {
        0xDB,
        0xB2,
        0xB1,
        0xB0};

    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {

            int dist_sq = x * x + y * y;
            int max_dist_sq = radius * radius;

            if (dist_sq <= max_dist_sq)
            {
                int screen_x = center_x + x;
                int screen_y = center_y + y;

                int intensity;
                if (dist_sq <= max_dist_sq / 4)
                {
                    intensity = 0;
                }
                else if (dist_sq <= max_dist_sq / 2)
                {
                    intensity = 1;
                }
                else if (dist_sq <= max_dist_sq * 3 / 4)
                {
                    intensity = 2;
                }
                else
                {
                    intensity = 3;
                }

                if (screen_x >= 0 && screen_x < VGA_WIDTH &&
                    screen_y >= 0 && screen_y < VGA_HEIGHT)
                {

                    terminal_putentryat(
                        block_chars[intensity],
                        vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK),
                        screen_x, screen_y);
                }
            }
        }
    }

    const char *message = "This is a circle!";
    int text_x = center_x - strlen(message) / 2;
    int text_y = center_y;

    terminal_set_cursor(text_x, text_y);
    terminal_setcolor(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
    terminal_writestring(message);

    terminal_set_cursor(0, VGA_HEIGHT - 3);
    for (int x = 0; x < VGA_WIDTH; x++)
    {
        terminal_putchar('-');
    }

    terminal_set_cursor(0, VGA_HEIGHT - 2);
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Press ENTER to return to T84 OS...");

    while (1)
    {
        char c = keyboard_getchar();
        if (c == '\n' || c == '\r')
        {
            break;
        }
    }

    terminal_clear();
    terminal_row = saved_row;
    terminal_column = saved_col;
    terminal_color = saved_color;

    terminal_set_cursor(0, 0);
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Returned from Circle Example.\n");
    terminal_writestring("T84> ");
}