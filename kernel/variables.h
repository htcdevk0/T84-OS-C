#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdint.h>
#include <stdbool.h>

/* Tipos de variáveis */
typedef enum {
    VAR_INT,
    VAR_STRING,
    VAR_FLOAT,
    VAR_BOOL,
    VAR_NULL
} VarType;

/* Estrutura de variável */
typedef struct Variable {
    char name[32];
    VarType type;
    union {
        int32_t int_value;
        char* string_value;
        float float_value;
        bool bool_value;
    } value;
    struct Variable* next;
} Variable;

/* Sistema de variáveis */
void vars_init(void);
Variable* var_create(const char* name, VarType type);
Variable* var_get(const char* name);
bool var_set_int(const char* name, int32_t value);
bool var_set_string(const char* name, const char* value);
bool var_set_float(const char* name, float value);
bool var_set_bool(const char* name, bool value);
void var_delete(const char* name);
void vars_list(void);

/* Funções de impressão */
void print_var(Variable* var);
void print_var_by_name(const char* name);

/* Parser de expressões */
int32_t eval_expression(const char* expr);
bool eval_condition(const char* cond);

#endif