#ifndef ISR_H
#define ISR_H

#include "stdint.h"

#define IRQ0 32
#define IRQ1 33
#define IRQ12 44

void isr_install(void);
void irq_install(void);
void irq_install_handler(int irq, void (*handler)(void));
void irq_uninstall_handler(int irq);

#endif