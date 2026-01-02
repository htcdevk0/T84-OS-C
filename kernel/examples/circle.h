#ifndef CIRCLE_EXAMPLE_H
#define CIRCLE_EXAMPLE_H

#include <stdint.h>

/* Função principal do exemplo circle */
void circle_example_run(void);

/* Funções auxiliares */
void draw_circle(int center_x, int center_y, int radius, uint8_t color);
void draw_text_centered(const char* text, int y, uint8_t color);

#endif