#include "string_utils.h"
#include <stdint.h>

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

char *strcpy(char *dest, const char *src)
{
    char *ptr = dest;
    while ((*ptr++ = *src++))
        ;
    return dest;
}

char *strcat(char *dest, const char *src)
{
    char *ptr = dest + strlen(dest);
    while ((*ptr++ = *src++))
        ;
    return dest;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

int strncmp(const char *str1, const char *str2, size_t n)
{
    while (n && *str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
        n--;
    }
    if (n == 0)
        return 0;
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

char *strchr(const char *str, int c)
{
    while (*str)
    {
        if (*str == c)
            return (char *)str;
        str++;
    }
    return NULL;
}

char *strtok(char *str, const char *delim)
{
    static char *saved = NULL;

    if (str)
        saved = str;
    if (!saved)
        return NULL;

    while (*saved && strchr(delim, *saved))
        saved++;
    if (!*saved)
        return NULL;

    char *token = saved;

    while (*saved && !strchr(delim, *saved))
        saved++;

    if (*saved)
    {
        *saved = '\0';
        saved++;
    }
    else
    {
        saved = NULL;
    }

    return token;
}

void *memset(void *ptr, int value, size_t num)
{
    unsigned char *p = ptr;
    while (num--)
        *p++ = (unsigned char)value;
    return ptr;
}

void *memcpy(void *dest, const void *src, size_t num)
{
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (num--)
        *d++ = *s++;
    return dest;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    const unsigned char *p1 = ptr1;
    const unsigned char *p2 = ptr2;
    while (num--)
    {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

int atoi(const char *str)
{
    int result = 0;
    int sign = 1;

    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

char *itoa(int value, char *str, int base)
{
    if (base < 2 || base > 36)
    {
        *str = '\0';
        return str;
    }

    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;

    if (value == 0)
    {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    if (value < 0 && base == 10)
    {
        *ptr++ = '-';
        value = -value;
        ptr1++;
    }

    while (value != 0)
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[tmp_value - value * base];
    }

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return str;
}

int isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
}

int isdigit(int c)
{
    return (c >= '0' && c <= '9');
}

int isalpha(int c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}

char toupper(char c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';
    return c;
}

char tolower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    return c;
}

int vsprintf(char *str, const char *format, va_list ap)
{
    char *ptr = str;
    const char *fmt = format;

    while (*fmt)
    {
        if (*fmt != '%')
        {
            *ptr++ = *fmt++;
            continue;
        }

        fmt++;

        switch (*fmt)
        {
        case 'd':
        case 'i':
        {
            int num = va_arg(ap, int);
            char num_buf[32];
            itoa(num, num_buf, 10);
            char *num_ptr = num_buf;
            while (*num_ptr)
                *ptr++ = *num_ptr++;
            break;
        }
        case 'u':
        {
            unsigned int num = va_arg(ap, unsigned int);
            char num_buf[32];
            char *num_ptr = num_buf;

            if (num == 0)
            {
                *ptr++ = '0';
            }
            else
            {
                char temp[32];
                int i = 0;
                while (num > 0)
                {
                    temp[i++] = (num % 10) + '0';
                    num /= 10;
                }
                while (i > 0)
                    *ptr++ = temp[--i];
            }
            break;
        }
        case 'x':
        case 'X':
        {
            unsigned int num = va_arg(ap, unsigned int);
            char hex_buf[9];
            char *hex_ptr = hex_buf;
            const char *hex_chars = (*fmt == 'x') ? "0123456789abcdef" : "0123456789ABCDEF";

            if (num == 0)
            {
                *ptr++ = '0';
            }
            else
            {
                char temp[9];
                int i = 0;
                while (num > 0)
                {
                    temp[i++] = hex_chars[num & 0xF];
                    num >>= 4;
                }
                while (i > 0)
                    *ptr++ = temp[--i];
            }
            break;
        }
        case 'c':
        {
            char c = (char)va_arg(ap, int);
            *ptr++ = c;
            break;
        }
        case 's':
        {
            char *s = va_arg(ap, char *);
            while (*s)
                *ptr++ = *s++;
            break;
        }
        case '%':
            *ptr++ = '%';
            break;
        default:
            *ptr++ = '%';
            *ptr++ = *fmt;
            break;
        }
        fmt++;
    }

    *ptr = '\0';
    return ptr - str;
}

int sprintf(char *str, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int result = vsprintf(str, format, ap);
    va_end(ap);
    return result;
}

int sscanf(const char *str, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    const char *p = str;
    int count = 0;

    while (*format && *p)
    {
        if (*format == '%')
        {
            format++;

            switch (*format)
            {
            case 'd':
            case 'i':
            {
                int *num = va_arg(ap, int *);
                *num = 0;
                int sign = 1;

                while (*p == ' ')
                    p++;

                if (*p == '-')
                {
                    sign = -1;
                    p++;
                }
                else if (*p == '+')
                {
                    p++;
                }

                while (*p >= '0' && *p <= '9')
                {
                    *num = *num * 10 + (*p - '0');
                    p++;
                }

                *num *= sign;
                count++;
                break;
            }
            case 's':
            {
                char *s = va_arg(ap, char *);
                int i = 0;

                while (*p == ' ')
                    p++;

                while (*p && *p != ' ' && *p != '\t' && *p != '\n')
                {
                    s[i++] = *p++;
                }
                s[i] = '\0';
                count++;
                break;
            }
            case 'c':
            {
                char *c = va_arg(ap, char *);
                *c = *p++;
                count++;
                break;
            }
            }
            format++;
        }
        else if (*format == *p)
        {
            format++;
            p++;
        }
        else
        {
            break;
        }
    }

    va_end(ap);
    return count;
}

uint32_t parse_uint(const char *str)
{
    uint32_t result = 0;
    int base = 10;

    if (!str)
        return 0;

        if (str[0] == '0' && str[1] == 'x')
    {
        base = 16;
        str += 2;
    }

    while (*str)
    {
        char c = *str;

        if (base == 10)
        {
            if (c < '0' || c > '9')
                break;
            result = result * 10 + (c - '0');
        }
        else
        {
            if (c >= '0' && c <= '9')
                result = result * 16 + (c - '0');
            else if (c >= 'a' && c <= 'f')
                result = result * 16 + (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F')
                result = result * 16 + (c - 'A' + 10);
            else
                break;
        }

        str++;
    }

    return result;
}