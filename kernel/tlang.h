#ifndef TLANG_H
#define TLANG_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    TLANG_INT,
    TLANG_SCHAR,
    TLANG_BOOL,
    TLANG_NONE
} TLANG_Type;

typedef struct
{
    char name[32];
    TLANG_Type type;
    union
    {
        int int_value;
        char *str_value;
        bool bool_value;
    } value;
} TLANG_Variable;

typedef struct
{
    TLANG_Variable variables[100];
    int var_count;
    int line_number;
    bool had_error;
} TLANG_Interpreter;

void tlang_init(void);
void tlang_run_file(const char *filename);
void tlang_run_line(const char *line);
void tlang_cleanup(void);

void cmd_tlang(const char *args);

#endif