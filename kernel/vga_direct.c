#include "vga_direct.h"
#include "ports.h"

#define VGA13_MEMORY 0xA0000
#define VGA13_WIDTH 320
#define VGA13_HEIGHT 200

void vga_mode_text(void)
{

    outb(0x3D4, 0x03);
    outb(0x3D5, 0x80);

    outb(0x3C4, 0x00);
    outb(0x3C5, 0x01);
    outb(0x3C4, 0x01);
    outb(0x3C5, 0x00);

    outb(0x3D4, 0x11);
    outb(0x3D5, 0x00);

    uint8_t gfx[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F, 0x0D};
    for (int i = 0; i < 9; i++)
    {
        outb(0x3CE, i);
        outb(0x3CF, gfx[i]);
    }

    outb(0x3C4, 0x01);
    outb(0x3C5, 0x01);
    outb(0x3D4, 0x00);
    outb(0x3D5, 0x67);
    outb(0x3D4, 0x01);
    outb(0x3D5, 0x58);
}

void vga_mode_13h(void)
{

    asm volatile("cli");

    outb(0x3C4, 0x01);
    outb(0x3C5, 0x01);

    uint8_t seq_regs[] = {0x03, 0x01, 0x0F, 0x00, 0x0E};
    for (int i = 0; i < 5; i++)
    {
        outb(0x3C4, i);
        outb(0x3C5, seq_regs[i]);
    }

    uint8_t crt_regs[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF};
    for (int i = 0; i < 25; i++)
    {
        outb(0x3D4, i);
        outb(0x3D5, crt_regs[i]);
    }

    uint8_t gfx_regs[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF};
    for (int i = 0; i < 9; i++)
    {
        outb(0x3CE, i);
        outb(0x3CF, gfx_regs[i]);
    }

    for (int i = 0; i < 16; i++)
    {
        outb(0x3C0, i);
        outb(0x3C0, i);
    }
    outb(0x3C0, 0x20);

    outb(0x3C4, 0x01);
    outb(0x3C5, 0x01);
}

void vga_clear(uint8_t color)
{
    volatile uint8_t *vga = (volatile uint8_t *)VGA13_MEMORY;
    for (int i = 0; i < VGA13_WIDTH * VGA13_HEIGHT; i++)
    {
        vga[i] = color;
    }
}

void vga_put_pixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= VGA13_WIDTH || y < 0 || y >= VGA13_HEIGHT)
        return;

    volatile uint8_t *vga = (volatile uint8_t *)VGA13_MEMORY;
    vga[y * VGA13_WIDTH + x] = color;
}

void vga_draw_circle(int cx, int cy, int radius, uint8_t color)
{
    int radius_sq = radius * radius;

    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if (x * x + y * y <= radius_sq)
            {
                vga_put_pixel(cx + x, cy + y, color);
            }
        }
    }
}