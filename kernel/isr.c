#include "isr.h"
#include "keyboard.h"
#include "vga.h"
#include "string_utils.h"

void (*irq_handlers[16])(void);

void isr_install(void)
{
}

void irq_install(void)
{

    for (int i = 0; i < 16; i++)
    {
        irq_handlers[i] = 0;
    }
}

void irq_install_handler(int irq, void (*handler)(void))
{
    irq_handlers[irq - IRQ0] = handler;
}

void irq_uninstall_handler(int irq)
{
    irq_handlers[irq - IRQ0] = 0;
}

void irq_handler(int irq)
{
    if (irq_handlers[irq - IRQ0])
    {
        irq_handlers[irq - IRQ0]();
    }
}