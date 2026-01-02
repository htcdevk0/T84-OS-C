#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
char *strchr(const char *str, int c);
char *strtok(char *str, const char *delim);

void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);

int atoi(const char *str);
char *itoa(int value, char *str, int base);

int isspace(int c);
int isdigit(int c);
int isalpha(int c);
int isalnum(int c);
char toupper(char c);
char tolower(char c);

int vsprintf(char *str, const char *format, va_list ap);
int sprintf(char *str, const char *format, ...);
int sscanf(const char *str, const char *format, ...);

uint32_t parse_uint(const char *str);

#endif