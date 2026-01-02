#include "../../api/kernel_api.h"
#include "../../../kernel/stdint.h"
#include "../../../kernel/vga.h"
#include "../../../kernel/keyboard.h"
#include "../../../kernel/kernel.h"
#include <stdbool.h>

#define STAT_WIDTH 60
#define STAT_HEIGHT 18
#define STAT_START_X 10
#define STAT_START_Y 3

#define STAT_BORDER_COLOR 0x1F
#define STAT_HEADER_COLOR 0x4E
#define STAT_LABEL_COLOR 0x0B
#define STAT_VALUE_COLOR 0x0F
#define STAT_BAR_COLOR 0x2A
#define STAT_WARNING_COLOR 0x4C

static void clear_screen_area(int x, int y, int w, int h, uint8_t color)
{
    uint16_t *vga = (uint16_t *)0xB8000;
    uint16_t blank = (uint16_t)' ' | (uint16_t)color << 8;

    for (int row = 0; row < h; row++)
    {
        for (int col = 0; col < w; col++)
        {
            vga[(y + row) * 80 + (x + col)] = blank;
        }
    }
}

static void write_at_color(int x, int y, const char *text, uint8_t color)
{
    uint16_t *vga = (uint16_t *)0xB8000;
    int i = 0;

    while (text[i] && (x + i) < 80)
    {
        vga[y * 80 + (x + i)] = (uint16_t)text[i] | (uint16_t)color << 8;
        i++;
    }
}

static void draw_border(void)
{
    uint16_t *vga = (uint16_t *)0xB8000;
    uint16_t border_char = (uint16_t)' ' | (uint16_t)STAT_BORDER_COLOR << 8;

    vga[STAT_START_Y * 80 + STAT_START_X] = 0xC9 | STAT_BORDER_COLOR << 8;
    vga[STAT_START_Y * 80 + STAT_START_X + STAT_WIDTH + 1] = 0xBB | STAT_BORDER_COLOR << 8;
    vga[(STAT_START_Y + STAT_HEIGHT + 1) * 80 + STAT_START_X] = 0xC8 | STAT_BORDER_COLOR << 8;
    vga[(STAT_START_Y + STAT_HEIGHT + 1) * 80 + STAT_START_X + STAT_WIDTH + 1] = 0xBC | STAT_BORDER_COLOR << 8;

    for (int x = 1; x <= STAT_WIDTH; x++)
    {
        vga[STAT_START_Y * 80 + STAT_START_X + x] = 0xCD | STAT_BORDER_COLOR << 8;
        vga[(STAT_START_Y + STAT_HEIGHT + 1) * 80 + STAT_START_X + x] = 0xCD | STAT_BORDER_COLOR << 8;
    }

    for (int y = 1; y <= STAT_HEIGHT; y++)
    {
        vga[(STAT_START_Y + y) * 80 + STAT_START_X] = 0xBA | STAT_BORDER_COLOR << 8;
        vga[(STAT_START_Y + y) * 80 + STAT_START_X + STAT_WIDTH + 1] = 0xBA | STAT_BORDER_COLOR << 8;
    }

    write_at_color(STAT_START_X + 2, STAT_START_Y, " T84 System Monitor (cstat) ", STAT_BORDER_COLOR);
}

static void draw_bar(int x, int y, int width, int percent, uint8_t color)
{
    int filled = (percent * width) / 100;

    for (int i = 0; i < width; i++)
    {
        char c = (i < filled) ? 0xDB : 0xB0;
        write_at_color(x + i, y, &c, color);
    }

    char percent_str[8];
    percent_str[0] = ' ';
    percent_str[1] = '0' + (percent / 10);
    percent_str[2] = '0' + (percent % 10);
    percent_str[3] = '%';
    percent_str[4] = '\0';

    write_at_color(x + width + 2, y, percent_str, STAT_VALUE_COLOR);
}

static int get_total_memory(void)
{

    return 64;
}

static int get_used_memory(void)
{

    return 16;
}

static int get_cpu_count(void)
{

    return 1;
}

static int get_cpu_usage(void)
{

    return 15;
}

static int get_disk_space(void)
{

    return 20;
}

static int get_used_disk(void)
{

    return 8;
}

static const char *get_system_name(void)
{
    return "T84 OS v1.0";
}

static const char *get_cpu_name(void)
{
    return "Intel 8086/286 Compatible";
}

static const char *get_uptime(void)
{

    return "00:15:32";
}

void show_system_stats(void)
{
    int y = STAT_START_Y + 2;

    write_at_color(STAT_START_X + 2, y, "System Information:", STAT_HEADER_COLOR);
    y += 2;

    write_at_color(STAT_START_X + 4, y, "OS Name:      ", STAT_LABEL_COLOR);
    write_at_color(STAT_START_X + 18, y, get_system_name(), STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "CPU:          ", STAT_LABEL_COLOR);
    write_at_color(STAT_START_X + 18, y, get_cpu_name(), STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Uptime:       ", STAT_LABEL_COLOR);
    write_at_color(STAT_START_X + 18, y, get_uptime(), STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Kernel:       ", STAT_LABEL_COLOR);
    write_at_color(STAT_START_X + 18, y, "Monolithic", STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Architecture: ", STAT_LABEL_COLOR);
    write_at_color(STAT_START_X + 18, y, "x86 16-bit", STAT_VALUE_COLOR);
}

void show_memory_stats(void)
{
    int y = STAT_START_Y + 8;

    write_at_color(STAT_START_X + 2, y, "Memory Usage:", STAT_HEADER_COLOR);
    y += 2;

    int total_mem = get_total_memory();
    int used_mem = get_used_memory();
    int free_mem = total_mem - used_mem;
    int percent_used = (used_mem * 100) / total_mem;

    char mem_str[32];

    write_at_color(STAT_START_X + 4, y, "Total:  ", STAT_LABEL_COLOR);
    int pos = 0;
    mem_str[pos++] = '0' + (total_mem / 10);
    mem_str[pos++] = '0' + (total_mem % 10);
    mem_str[pos++] = ' ';
    mem_str[pos++] = 'M';
    mem_str[pos++] = 'B';
    mem_str[pos] = '\0';
    write_at_color(STAT_START_X + 13, y, mem_str, STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Used:   ", STAT_LABEL_COLOR);
    pos = 0;
    mem_str[pos++] = '0' + (used_mem / 10);
    mem_str[pos++] = '0' + (used_mem % 10);
    mem_str[pos++] = ' ';
    mem_str[pos++] = 'M';
    mem_str[pos++] = 'B';
    mem_str[pos] = '\0';
    write_at_color(STAT_START_X + 13, y, mem_str, STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Free:   ", STAT_LABEL_COLOR);
    pos = 0;
    mem_str[pos++] = '0' + (free_mem / 10);
    mem_str[pos++] = '0' + (free_mem % 10);
    mem_str[pos++] = ' ';
    mem_str[pos++] = 'M';
    mem_str[pos++] = 'B';
    mem_str[pos] = '\0';
    write_at_color(STAT_START_X + 13, y, mem_str, STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Usage:  ", STAT_LABEL_COLOR);
    draw_bar(STAT_START_X + 13, y, 20, percent_used, STAT_BAR_COLOR);
}

void show_cpu_stats(void)
{
    int y = STAT_START_Y + 14;

    write_at_color(STAT_START_X + 32, y, "CPU Stats:", STAT_HEADER_COLOR);
    y += 2;

    int cpu_count = get_cpu_count();
    int cpu_usage = get_cpu_usage();

    write_at_color(STAT_START_X + 34, y, "Cores:   ", STAT_LABEL_COLOR);
    char cores_str[4];
    cores_str[0] = '0' + cpu_count;
    cores_str[1] = '\0';
    write_at_color(STAT_START_X + 43, y, cores_str, STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 34, y, "Usage:   ", STAT_LABEL_COLOR);
    draw_bar(STAT_START_X + 43, y, 15, cpu_usage, STAT_BAR_COLOR);
    y++;

    write_at_color(STAT_START_X + 34, y, "Speed:   ", STAT_LABEL_COLOR);
    write_at_color(STAT_START_X + 43, y, "4.77 MHz", STAT_VALUE_COLOR);
}

void show_disk_stats(void)
{
    int y = STAT_START_Y + 14;

    write_at_color(STAT_START_X + 2, y, "Disk Usage:", STAT_HEADER_COLOR);
    y += 2;

    int total_disk = get_disk_space();
    int used_disk = get_used_disk();
    int free_disk = total_disk - used_disk;
    int percent_used = (used_disk * 100) / total_disk;

    char disk_str[32];

    write_at_color(STAT_START_X + 4, y, "Total:  ", STAT_LABEL_COLOR);
    int pos = 0;
    disk_str[pos++] = '0' + (total_disk / 10);
    disk_str[pos++] = '0' + (total_disk % 10);
    disk_str[pos++] = ' ';
    disk_str[pos++] = 'M';
    disk_str[pos++] = 'B';
    disk_str[pos] = '\0';
    write_at_color(STAT_START_X + 13, y, disk_str, STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Used:   ", STAT_LABEL_COLOR);
    pos = 0;
    disk_str[pos++] = '0' + (used_disk / 10);
    disk_str[pos++] = '0' + (used_disk % 10);
    disk_str[pos++] = ' ';
    disk_str[pos++] = 'M';
    disk_str[pos++] = 'B';
    disk_str[pos] = '\0';
    write_at_color(STAT_START_X + 13, y, disk_str, STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Free:   ", STAT_LABEL_COLOR);
    pos = 0;
    disk_str[pos++] = '0' + (free_disk / 10);
    disk_str[pos++] = '0' + (free_disk % 10);
    disk_str[pos++] = ' ';
    disk_str[pos++] = 'M';
    disk_str[pos++] = 'B';
    disk_str[pos] = '\0';
    write_at_color(STAT_START_X + 13, y, disk_str, STAT_VALUE_COLOR);
    y++;

    write_at_color(STAT_START_X + 4, y, "Usage:  ", STAT_LABEL_COLOR);
    draw_bar(STAT_START_X + 13, y, 20, percent_used, STAT_BAR_COLOR);
}

void show_all_stats(void)
{

    clear_screen_area(STAT_START_X + 1, STAT_START_Y + 1, STAT_WIDTH, STAT_HEIGHT, 0x07);

    show_system_stats();
    show_memory_stats();
    show_cpu_stats();
    show_disk_stats();

    write_at_color(STAT_START_X + 2, STAT_START_Y + STAT_HEIGHT + 2,
                   "Press any key to refresh, ESC to exit", STAT_BORDER_COLOR);
}

void cstat_run(void)
{
    bool running = true;

    draw_border();
    show_all_stats();

    while (running)
    {
        if (keyboard_available())
        {
            char c = keyboard_getchar();

            if (c == 27)
            {
                running = false;
                cmd_clear(1);
                break;
            }
            else
            {

                show_all_stats();
            }
        }

        for (volatile int i = 0; i < 10000; i++)
            ;
    }

    terminal_clear();
}

void cmd_cstat(const char *args)
{
    (void)args;

    terminal_writestring("\nStarting T84 System Monitor...\n");

    for (volatile int i = 0; i < 1000000; i++)
        ;

    cstat_run();
}

static void main_app(void)
{
    terminal_writestring("\nT84 System Monitor (cstat)\n");
    terminal_writestring("Usage: cstat\n");
    terminal_writestring("Shows system information and statistics\n");
}