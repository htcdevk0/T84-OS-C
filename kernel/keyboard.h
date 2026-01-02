#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);
char keyboard_getchar(void);
void keyboard_getline(char *buffer, uint32_t size);
bool keyboard_available(void);
uint8_t keyboard_read_scancode(void);

#endif