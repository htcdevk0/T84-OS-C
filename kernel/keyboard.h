#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);
char keyboard_getchar(void);
void keyboard_getline(char *buffer, uint32_t size);
bool keyboard_available(void);
uint8_t keyboard_read_scancode(void);
uint8_t keyboard_get_scancode(void);
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);

#endif