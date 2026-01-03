#include "keyboard.h"
#include "vga.h"
#include "string_utils.h"

static const char keymap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const char keymap_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static bool shift_pressed = false;

uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void keyboard_init(void)
{
    shift_pressed = false;
}

bool keyboard_available(void)
{
    return (inb(0x64) & 0x01) != 0;
}

uint8_t keyboard_get_scancode(void)
{
    while (!keyboard_available())
        ;

    uint8_t scancode;
    __asm__ volatile("inb $0x60, %0" : "=a"(scancode));

    return scancode;
}

char keyboard_getchar(void)
{
    static bool got_e0 = false;

    while (!keyboard_available())
    {
    }

    uint8_t scancode = inb(0x60);

    if (scancode & 0x80)
    {
        uint8_t key = scancode & 0x7F;

        if (key == 0x2A || key == 0x36)
        {
            shift_pressed = false;
        }

        if (got_e0)
        {
            got_e0 = false;
        }

        return 0;
    }

    if (scancode == 0xE0)
    {
        got_e0 = true;
        return 0;
    }

    if (got_e0)
    {
        got_e0 = false;

        switch (scancode)
        {
        case 0x48:
            return 0x11;
        case 0x50:
            return 0x12;
        case 0x4B:
            return 0x13;
        case 0x4D:
            return 0x14;
        default:
            return 0;
        }
    }

    if (scancode == 0x2A || scancode == 0x36)
    {
        shift_pressed = true;
        return 0;
    }

    if (scancode < 128)
    {
        if (shift_pressed)
        {
            return keymap_shift[scancode];
        }
        else
        {
            return keymap[scancode];
        }
    }

    return 0;
}

void keyboard_getline(char *buffer, uint32_t size)
{
    uint32_t index = 0;

    while (index < size - 1)
    {
        char c = keyboard_getchar();

        if (c == '\n' || c == '\r')
        {
            buffer[index] = '\0';
            terminal_putchar('\n');
            return;
        }
        else if (c == '\b')
        {
            if (index > 0)
            {
                index--;
                terminal_putchar('\b');
            }
        }
        else if (c >= 32 && c <= 126)
        {
            buffer[index++] = c;
            terminal_putchar(c);
        }
    }

    buffer[size - 1] = '\0';
}