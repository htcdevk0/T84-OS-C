
#include "variables.h"
#include "string_utils.h"
#include "vga.h"

int local_atoi(const char *str)
{
    int result = 0;
    int sign = 1;

    if (*str == '-')
    {
        sign = -1;
        str++;
    }

    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

#define MAX_VARIABLES 50

static Variable variables[MAX_VARIABLES];
static int var_count = 0;
static char string_pool[4096];
static int string_pool_used = 0;

void vars_init(void)
{
    var_count = 0;
    string_pool_used = 0;
}

Variable *var_create(const char *name, VarType type)
{
    if (var_count >= MAX_VARIABLES)
        return NULL;

    Variable *var = &variables[var_count++];
    strcpy(var->name, name);
    var->type = type;
    var->next = NULL;

    switch (type)
    {
    case VAR_INT:
        var->value.int_value = 0;
        break;
    case VAR_STRING:
        var->value.string_value = NULL;
        break;
    case VAR_FLOAT:
        var->value.float_value = 0.0f;
        break;
    case VAR_BOOL:
        var->value.bool_value = false;
        break;
    case VAR_NULL:
        break;
    }

    return var;
}

Variable *var_get(const char *name)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(variables[i].name, name) == 0)
        {
            return &variables[i];
        }
    }
    return NULL;
}

bool var_set_int(const char *name, int32_t value)
{
    Variable *var = var_get(name);
    if (!var)
    {
        var = var_create(name, VAR_INT);
        if (!var)
            return false;
    }

    var->type = VAR_INT;
    var->value.int_value = value;
    return true;
}

bool var_set_string(const char *name, const char *value)
{
    Variable *var = var_get(name);
    if (!var)
    {
        var = var_create(name, VAR_STRING);
        if (!var)
            return false;
    }

    var->type = VAR_STRING;

    int len = strlen(value);
    if (string_pool_used + len + 1 > sizeof(string_pool))
    {
        return false;
    }

    char *str = &string_pool[string_pool_used];
    strcpy(str, value);
    var->value.string_value = str;
    string_pool_used += len + 1;

    return true;
}

bool var_set_float(const char *name, float value)
{
    Variable *var = var_get(name);
    if (!var)
    {
        var = var_create(name, VAR_FLOAT);
        if (!var)
            return false;
    }

    var->type = VAR_FLOAT;
    var->value.float_value = value;
    return true;
}

bool var_set_bool(const char *name, bool value)
{
    Variable *var = var_get(name);
    if (!var)
    {
        var = var_create(name, VAR_BOOL);
        if (!var)
            return false;
    }

    var->type = VAR_BOOL;
    var->value.bool_value = value;
    return true;
}

void var_delete(const char *name)
{

    Variable *var = var_get(name);
    if (var)
    {
        var->type = VAR_NULL;
    }
}

void vars_list(void)
{
    terminal_writestring("\n=== Variables ===\n");

    int count = 0;

    for (int i = 0; i < var_count; i++)
    {
        if (variables[i].type == VAR_NULL)
            continue;

        terminal_writestring(variables[i].name);
        terminal_writestring(": ");

        switch (variables[i].type)
        {
        case VAR_INT:
            terminal_writestring("INT = ");
            char int_str[16];
            itoa(variables[i].value.int_value, int_str, 10);
            terminal_writestring(int_str);
            break;

        case VAR_STRING:
            terminal_writestring("STRING = \"");
            terminal_writestring(variables[i].value.string_value);
            terminal_writestring("\"");
            break;

        case VAR_FLOAT:
            terminal_writestring("FLOAT = ");
            char float_str[32];

            int int_part = (int)variables[i].value.float_value;
            itoa(int_part, float_str, 10);
            terminal_writestring(float_str);
            terminal_writestring(".0");
            break;

        case VAR_BOOL:
            terminal_writestring("BOOL = ");
            terminal_writestring(variables[i].value.bool_value ? "true" : "false");
            break;

        case VAR_NULL:
            continue;
        }

        terminal_writestring("\n");
        count++;
    }

    if (count == 0)
    {
        terminal_writestring("No variables defined.\n");
    }
    else
    {
        char count_str[16];
        itoa(count, count_str, 10);
        terminal_writestring("\nTotal: ");
        terminal_writestring(count_str);
        terminal_writestring(" variable(s)\n");
    }
}

void print_var(Variable *var)
{
    if (!var || var->type == VAR_NULL)
    {
        terminal_writestring("[null]");
        return;
    }

    switch (var->type)
    {
    case VAR_INT:
    {
        char buffer[16];
        itoa(var->value.int_value, buffer, 10);
        terminal_writestring(buffer);
        break;
    }
    case VAR_STRING:
        terminal_writestring(var->value.string_value);
        break;
    case VAR_FLOAT:
    {
        char buffer[32];
        int int_part = (int)var->value.float_value;
        itoa(int_part, buffer, 10);
        terminal_writestring(buffer);
        terminal_writestring(".0");
        break;
    }
    case VAR_BOOL:
        terminal_writestring(var->value.bool_value ? "true" : "false");
        break;
    }
}

void print_var_by_name(const char *name)
{
    Variable *var = var_get(name);
    if (var && var->type != VAR_NULL)
    {

        switch (var->type)
        {
        case VAR_INT:
        {
            char buffer[16];
            itoa(var->value.int_value, buffer, 10);
            terminal_writestring(buffer);
            break;
        }
        case VAR_STRING:
            terminal_writestring(var->value.string_value);
            break;
        case VAR_FLOAT:
        {
            char buffer[32];
            int int_part = (int)var->value.float_value;
            itoa(int_part, buffer, 10);
            terminal_writestring(buffer);
            terminal_writestring(".0");
            break;
        }
        case VAR_BOOL:
            terminal_writestring(var->value.bool_value ? "true" : "false");
            break;
        case VAR_NULL:
            terminal_writestring("[null]");
            break;
        }
    }
    else
    {
        terminal_writestring("[undefined]");
    }
}