#ifndef PANIC_H
#define PANIC_H

#include <stdint.h>

void panic(const char *message);
void panic_with_code(const char *message, uint32_t error_code);
void show_panic_screen(const char *panic_msg, uint32_t error_code);

#define PANIC(msg) panic(msg)
#define PANIC_CODE(msg, code) panic_with_code(msg, code)
#define ASSERT(condition)                       \
    if (!(condition))                           \
    {                                           \
        PANIC("Assertion failed: " #condition); \
    }

#endif