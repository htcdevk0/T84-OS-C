#include "tlang.h"
#include "../kernel/vga.h"
#include "../kernel/string_utils.h"
#include "../kernel/ramfs.h"
#include "../T84_OS/api/kernel_api.h"
#include "stddef.h"
#include <stdbool.h>

#define COLOR_WARNING 0x0C
#define COLOR_ERROR 0x0C
#define COLOR_INFO 0x0B

bool keyboard_available(void);
char keyboard_getchar(void);

static TLANG_Interpreter interpreter;

static void tlang_error(const char *msg);
static void tlang_warning(const char *msg);
static bool is_whitespace(char c);
static void skip_whitespace(const char **ptr);
static bool is_digit(char c);
static bool is_alpha(char c);
static bool is_alnum(char c);
static int simple_atoi(const char *str);
static char *simple_strstr(const char *haystack, const char *needle);

static TLANG_Variable *find_variable(const char *name);
static TLANG_Variable *create_variable(const char *name, TLANG_Type type);

static int parse_int_expression(const char **ptr);
static char *parse_string_literal(const char **ptr);
static bool parse_bool_literal(const char **ptr);
static void process_line_command(const char **ptr);
static int evaluate_math_expression(const char **ptr);
static bool evaluate_condition(const char **ptr);

static void process_int_declaration(const char **ptr);
static void process_schar_declaration(const char **ptr);
static void process_bool_declaration(const char **ptr);
static void process_write_command(const char **ptr);
static void process_input_command(const char **ptr);
static void process_if_statement(const char **ptr, bool *condition_met);
static void process_for_loop(const char **ptr);

static uint32_t rng_next(void);
static int random_int(int min, int max);
static void process_seed_command(const char **ptr);
static void process_random_command(const char **ptr);

void tlang_run_line(const char *line);

static uint32_t rng_state = 123456789;

static uint32_t rng_next(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static void process_seed_command(const char **ptr)
{
    skip_whitespace(ptr);

    if (**ptr != '(')
    {
        tlang_error("Expected '(' after seed");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    int seed_value = parse_int_expression(ptr);

    skip_whitespace(ptr);

    if (**ptr != ')')
    {
        tlang_error("Expected ')' after seed");
        return;
    }

    (*ptr)++;

    rng_state = (seed_value == 0) ? 123456789 : (uint32_t)seed_value;
}

static int random_int(int min, int max)
{
    if (min >= max)
        return min;
    uint32_t range = max - min + 1;
    return min + (rng_next() % range);
}

static void tlang_error(const char *msg)
{
    terminal_writestring("[TLANG ERROR] ");
    terminal_writestring(msg);
    terminal_writestring("\n");
    interpreter.had_error = true;
}

static void process_random_command(const char **ptr)
{
    skip_whitespace(ptr);

    if (**ptr != '(')
    {
        tlang_error("Expected '(' after random");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    if (**ptr == '"')
    {

        char *strings[20];
        int string_count = 0;

        while (**ptr && **ptr != ')')
        {
            skip_whitespace(ptr);

            if (**ptr == '"')
            {

                char *str = parse_string_literal(ptr);
                if (!str)
                {
                    tlang_error("Invalid string in random");
                    return;
                }

                if (string_count < 20)
                {

                    static char string_buffer[20][256];
                    strcpy(string_buffer[string_count], str);
                    strings[string_count] = string_buffer[string_count];
                    string_count++;
                }
                else
                {
                    tlang_error("Too many strings in random (max 20)");
                    return;
                }
            }
            else if (**ptr == ',')
            {
                (*ptr)++;
                skip_whitespace(ptr);
            }
            else
            {
                break;
            }
        }

        skip_whitespace(ptr);
        if (**ptr != ',')
        {
            tlang_error("Expected ',' before variable name");
            return;
        }

        (*ptr)++;
        skip_whitespace(ptr);

        char var_name[32];
        int i = 0;
        while (is_alnum(**ptr) && i < 31)
        {
            var_name[i++] = **ptr;
            (*ptr)++;
        }
        var_name[i] = '\0';

        skip_whitespace(ptr);
        if (**ptr != ')')
        {
            tlang_error("Expected ')' after random");
            return;
        }
        (*ptr)++;

        TLANG_Variable *var = find_variable(var_name);
        if (!var)
        {

            var = create_variable(var_name, TLANG_SCHAR);
        }

        if (!var)
        {
            tlang_error("Failed to create/find variable");
            return;
        }

        if (string_count > 0)
        {
            int index = random_int(0, string_count - 1);

            if (var->type != TLANG_SCHAR)
            {
                tlang_error("Target variable must be string (schar) for string random");
                return;
            }

            static char result_buffer[256];
            strcpy(result_buffer, strings[index]);
            var->value.str_value = result_buffer;
        }
        else
        {
            tlang_error("No strings provided to random");
        }
    }
    else
    {

        int min_value = parse_int_expression(ptr);

        skip_whitespace(ptr);

        bool use_arrow = false;
        if (**ptr == '-' && *(*ptr + 1) == '>')
        {
            use_arrow = true;
            (*ptr) += 2;
        }
        else if (**ptr == ',')
        {
            (*ptr)++;
        }
        else
        {
            tlang_error("Expected '->' or ',' in random");
            return;
        }

        skip_whitespace(ptr);

        int max_value = parse_int_expression(ptr);

        skip_whitespace(ptr);

        if (**ptr != ',')
        {
            tlang_error("Expected ',' before variable name");
            return;
        }

        (*ptr)++;
        skip_whitespace(ptr);

        char var_name[32];
        int i = 0;
        while (is_alnum(**ptr) && i < 31)
        {
            var_name[i++] = **ptr;
            (*ptr)++;
        }
        var_name[i] = '\0';

        skip_whitespace(ptr);
        if (**ptr != ')')
        {
            tlang_error("Expected ')' after random");
            return;
        }
        (*ptr)++;

        TLANG_Variable *var = find_variable(var_name);
        if (!var)
        {

            var = create_variable(var_name, TLANG_INT);
        }

        if (!var)
        {
            tlang_error("Failed to create/find variable");
            return;
        }

        if (var->type != TLANG_INT)
        {
            tlang_error("Target variable must be integer for number random");
            return;
        }

        var->value.int_value = random_int(min_value, max_value);
    }
}

static bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void skip_whitespace(const char **ptr)
{
    while (is_whitespace(**ptr))
    {
        (*ptr)++;
    }
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_alnum(char c)
{
    return is_alpha(c) || is_digit(c);
}

static int simple_atoi(const char *str)
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

static char *simple_strstr(const char *haystack, const char *needle)
{
    if (!haystack || !needle || !*needle)
        return NULL;

    for (int i = 0; haystack[i]; i++)
    {
        int j = 0;
        while (needle[j] && haystack[i + j] && needle[j] == haystack[i + j])
        {
            j++;
        }
        if (!needle[j])
        {
            return (char *)&haystack[i];
        }
    }

    return NULL;
}

static TLANG_Variable *find_variable(const char *name)
{
    for (int i = 0; i < interpreter.var_count; i++)
    {
        if (strcmp(interpreter.variables[i].name, name) == 0)
        {
            return &interpreter.variables[i];
        }
    }
    return NULL;
}

static TLANG_Variable *create_variable(const char *name, TLANG_Type type)
{
    if (interpreter.var_count >= 100)
    {
        tlang_error("Too many variables");
        return NULL;
    }

    TLANG_Variable *var = &interpreter.variables[interpreter.var_count++];
    strcpy(var->name, name);
    var->type = type;

    switch (type)
    {
    case TLANG_INT:
        var->value.int_value = 0;
        break;
    case TLANG_SCHAR:
        var->value.str_value = "";
        break;
    case TLANG_BOOL:
        var->value.bool_value = false;
        break;
    default:
        break;
    }

    return var;
}

static int parse_int_expression(const char **ptr)
{
    skip_whitespace(ptr);

    int result = 0;
    bool is_negative = false;

    if (**ptr == '-')
    {
        is_negative = true;
        (*ptr)++;
    }

    while (is_digit(**ptr))
    {
        result = result * 10 + (**ptr - '0');
        (*ptr)++;
    }

    return is_negative ? -result : result;
}

static char *parse_string_literal(const char **ptr)
{
    static char buffer[256];
    int i = 0;

    if (**ptr != '"')
        return NULL;

    (*ptr)++;

    while (**ptr && **ptr != '"' && i < 255)
    {
        if (**ptr == '\\')
        {
            (*ptr)++;
            switch (**ptr)
            {
            case 'n':
                buffer[i++] = '\n';
                break;
            case 't':
                buffer[i++] = '\t';
                break;
            case '"':
                buffer[i++] = '"';
                break;
            case '\\':
                buffer[i++] = '\\';
                break;
            default:
                buffer[i++] = **ptr;
                break;
            }
        }
        else
        {
            buffer[i++] = **ptr;
        }
        (*ptr)++;
    }

    if (**ptr == '"')
    {
        (*ptr)++;
    }

    buffer[i] = '\0';
    return buffer;
}

static bool parse_bool_literal(const char **ptr)
{
    skip_whitespace(ptr);

    if (strncmp(*ptr, "true", 4) == 0)
    {
        *ptr += 4;
        return true;
    }
    else if (strncmp(*ptr, "false", 5) == 0)
    {
        *ptr += 5;
        return false;
    }
    else if (is_digit(**ptr))
    {
        int value = parse_int_expression(ptr);
        return value != 0;
    }

    return false;
}

static void process_line_command(const char **ptr)
{
    skip_whitespace(ptr);

    if (**ptr != '(')
    {
        tlang_error("Expected '(' after line");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    if (**ptr != ')')
    {
        tlang_error("Expected ')' in line()");
        return;
    }

    (*ptr)++;

    terminal_writestring("\n");
}

static int evaluate_math_expression(const char **ptr)
{
    skip_whitespace(ptr);

    int result = 0;
    char operator = '+';

    while (**ptr && **ptr != ')' && **ptr != ',' && !is_whitespace(**ptr))
    {
        skip_whitespace(ptr);

        if (is_digit(**ptr) || **ptr == '-')
        {
            int value = parse_int_expression(ptr);

            switch (operator)
            {
            case '+':
                result += value;
                break;
            case '-':
                result -= value;
                break;
            case '*':
                result *= value;
                break;
            case '/':
                if (value != 0)
                    result /= value;
                else
                    tlang_error("Division by zero");
                break;
            default:
                result = value;
                break;
            }
        }
        else if (is_alpha(**ptr))
        {
            char var_name[32];
            int i = 0;

            while (is_alnum(**ptr) && i < 31)
            {
                var_name[i++] = **ptr;
                (*ptr)++;
            }
            var_name[i] = '\0';

            TLANG_Variable *var = find_variable(var_name);
            if (var && var->type == TLANG_INT)
            {
                int value = var->value.int_value;

                switch (operator)
                {
                case '+':
                    result += value;
                    break;
                case '-':
                    result -= value;
                    break;
                case '*':
                    result *= value;
                    break;
                case '/':
                    if (value != 0)
                        result /= value;
                    else
                        tlang_error("Division by zero");
                    break;
                default:
                    result = value;
                    break;
                }
            }
            else
            {
                tlang_error("Variable not found or not integer");
            }
        }
        else if (**ptr == '+' || **ptr == '-' || **ptr == '*' || **ptr == '/')
        {
            operator = **ptr;
            (*ptr)++;
        }
        else
        {
            break;
        }

        skip_whitespace(ptr);
    }

    return result;
}

static bool evaluate_condition(const char **ptr)
{
    skip_whitespace(ptr);

    if (**ptr == '"')
    {

        char *left_str = parse_string_literal(ptr);

        skip_whitespace(ptr);

        char op[3] = {0};
        int op_len = 0;

        while (**ptr && (**ptr == '=' || **ptr == '!') && op_len < 2)
        {
            op[op_len++] = **ptr;
            (*ptr)++;
        }

        skip_whitespace(ptr);

        if (**ptr == '"')
        {
            char *right_str = parse_string_literal(ptr);

            if (strcmp(op, "==") == 0 || strcmp(op, "=") == 0)
            {
                return strcmp(left_str, right_str) == 0;
            }
            else if (strcmp(op, "!=") == 0)
            {
                return strcmp(left_str, right_str) != 0;
            }
            else
            {
                tlang_error("String comparison only supports == and !=");
                return false;
            }
        }
        else if (is_alpha(**ptr))
        {
            char var_name[32];
            int i = 0;

            while (is_alnum(**ptr) && i < 31)
            {
                var_name[i++] = **ptr;
                (*ptr)++;
            }
            var_name[i] = '\0';

            skip_whitespace(ptr);

            char op[3] = {0};
            int op_len = 0;

            while (**ptr && (**ptr == '<' || **ptr == '>' || **ptr == '=' || **ptr == '!') && op_len < 2)
            {
                op[op_len++] = **ptr;
                (*ptr)++;
            }

            skip_whitespace(ptr);

            TLANG_Variable *left_var = find_variable(var_name);
            if (!left_var)
            {
                tlang_error("Variable not found in condition");
                return false;
            }

            if (left_var->type == TLANG_BOOL)
            {
                bool left_value = left_var->value.bool_value;
                bool right_value = false;

                skip_whitespace(ptr);

                if (strncmp(*ptr, "true", 4) == 0 && !is_alnum((*ptr)[4]))
                {
                    right_value = true;
                    *ptr += 4;
                }
                else if (strncmp(*ptr, "false", 5) == 0 && !is_alnum((*ptr)[5]))
                {
                    right_value = false;
                    *ptr += 5;
                }

                else if (is_alpha(**ptr))
                {
                    char right_var_name[32];
                    int i = 0;
                    while (is_alnum(**ptr) && i < 31)
                    {
                        right_var_name[i++] = **ptr;
                        (*ptr)++;
                    }
                    right_var_name[i] = '\0';

                    TLANG_Variable *right_var = find_variable(right_var_name);
                    if (!right_var || right_var->type != TLANG_BOOL)
                    {
                        tlang_error("Right-hand side is not a bool");
                        return false;
                    }
                    right_value = right_var->value.bool_value;
                }

                else if (is_digit(**ptr))
                {
                    int num = parse_int_expression(ptr);
                    right_value = (num != 0);
                }
                else
                {
                    tlang_error("Expected true, false, bool variable, or 0/1");
                    return false;
                }

                skip_whitespace(ptr);

                char op[3] = {0};
                int op_len = 0;
                while (**ptr && (**ptr == '=' || **ptr == '!') && op_len < 2)
                    op[op_len++] = **ptr;
                (*ptr)++;

                if (strcmp(op, "==") == 0 || strcmp(op, "=") == 0)
                    return left_value == right_value;
                if (strcmp(op, "!=") == 0)
                    return left_value != right_value;

                tlang_error("Bool comparison only supports == and !=");
                return false;
            }
        }
        else
        {
            tlang_error("Expected string or string variable for comparison");
            return false;
        }
    }
    else if (is_alpha(**ptr))
    {

        char var_name[32];
        int i = 0;

        while (is_alnum(**ptr) && i < 31)
        {
            var_name[i++] = **ptr;
            (*ptr)++;
        }
        var_name[i] = '\0';

        skip_whitespace(ptr);

        char op[3] = {0};
        int op_len = 0;

        while (**ptr && (**ptr == '<' || **ptr == '>' || **ptr == '=' || **ptr == '!') && op_len < 2)
        {
            op[op_len++] = **ptr;
            (*ptr)++;
        }

        skip_whitespace(ptr);

        TLANG_Variable *left_var = find_variable(var_name);

        if (!left_var)
        {
            tlang_error("Variable not found in condition");
            return false;
        }

        if (left_var->type == TLANG_SCHAR)
        {
            if (**ptr == '"')
            {

                char *right_str = parse_string_literal(ptr);

                if (strcmp(op, "==") == 0 || strcmp(op, "=") == 0)
                {
                    return strcmp(left_var->value.str_value, right_str) == 0;
                }
                else if (strcmp(op, "!=") == 0)
                {
                    return strcmp(left_var->value.str_value, right_str) != 0;
                }
                else
                {
                    tlang_error("String comparison only supports == and !=");
                    return false;
                }
            }
            else if (is_alpha(**ptr))
            {

                char right_var_name[32];
                i = 0;

                while (is_alnum(**ptr) && i < 31)
                {
                    right_var_name[i++] = **ptr;
                    (*ptr)++;
                }
                right_var_name[i] = '\0';

                TLANG_Variable *right_var = find_variable(right_var_name);
                if (right_var && right_var->type == TLANG_SCHAR)
                {
                    if (strcmp(op, "==") == 0 || strcmp(op, "=") == 0)
                    {
                        return strcmp(left_var->value.str_value, right_var->value.str_value) == 0;
                    }
                    else if (strcmp(op, "!=") == 0)
                    {
                        return strcmp(left_var->value.str_value, right_var->value.str_value) != 0;
                    }
                    else
                    {
                        tlang_error("String comparison only supports == and !=");
                        return false;
                    }
                }
                else
                {
                    tlang_error("Right side is not a string variable");
                    return false;
                }
            }
            else
            {
                tlang_error("String comparison requires string on right side");
                return false;
            }
        }
        else
        {

            int left_value = 0;

            if (left_var->type == TLANG_INT)
            {
                left_value = left_var->value.int_value;
            }
            else if (left_var->type == TLANG_BOOL)
            {

                int left_value = left_var->value.bool_value ? 1 : 0;
                int right_value = 0;

                skip_whitespace(ptr);

                if (strncmp(*ptr, "true", 4) == 0 && !is_alnum((*ptr)[4]))
                {
                    right_value = 1;
                    *ptr += 4;
                }
                else if (strncmp(*ptr, "false", 5) == 0 && !is_alnum((*ptr)[5]))
                {
                    right_value = 0;
                    *ptr += 5;
                }

                else if (is_digit(**ptr) || **ptr == '-')
                {
                    right_value = parse_int_expression(ptr);
                }

                else if (is_alpha(**ptr))
                {
                    char right_var_name[32];
                    int i = 0;

                    while (is_alnum(**ptr) && i < 31)
                    {
                        right_var_name[i++] = **ptr;
                        (*ptr)++;
                    }
                    right_var_name[i] = '\0';

                    TLANG_Variable *right_var = find_variable(right_var_name);
                    if (!right_var)
                    {
                        tlang_error("Variable not found in condition");
                        return false;
                    }

                    if (right_var->type == TLANG_BOOL)
                    {
                        right_value = right_var->value.bool_value ? 1 : 0;
                    }
                    else if (right_var->type == TLANG_INT)
                    {
                        right_value = right_var->value.int_value;
                    }
                    else
                    {
                        tlang_error("Cannot compare bool with string");
                        return false;
                    }
                }
                else
                {
                    tlang_error("Expected true, false, number, or variable");
                    return false;
                }

                if (strcmp(op, "==") == 0 || strcmp(op, "=") == 0)
                {
                    return left_value == right_value;
                }
                else if (strcmp(op, "!=") == 0)
                {
                    return left_value != right_value;
                }
                else if (strcmp(op, "<") == 0)
                {
                    return left_value < right_value;
                }
                else if (strcmp(op, ">") == 0)
                {
                    return left_value > right_value;
                }
                else if (strcmp(op, "<=") == 0)
                {
                    return left_value <= right_value;
                }
                else if (strcmp(op, ">=") == 0)
                {
                    return left_value >= right_value;
                }
                else
                {
                    tlang_error("Unknown comparison operator");
                    return false;
                }
            }

            int right_value = 0;

            if (is_digit(**ptr) || **ptr == '-')
            {
                right_value = parse_int_expression(ptr);
            }
            else if (is_alpha(**ptr))
            {
                char right_var_name[32];
                i = 0;

                while (is_alnum(**ptr) && i < 31)
                {
                    right_var_name[i++] = **ptr;
                    (*ptr)++;
                }
                right_var_name[i] = '\0';

                TLANG_Variable *right_var = find_variable(right_var_name);
                if (right_var && right_var->type == TLANG_INT)
                {
                    right_value = right_var->value.int_value;
                }
                else if (right_var && right_var->type == TLANG_BOOL)
                {
                    right_value = right_var->value.bool_value ? 1 : 0;
                }
                else
                {
                    tlang_error("Variable not found in condition");
                    return false;
                }
            }

            if (strcmp(op, "==") == 0 || strcmp(op, "=") == 0)
            {
                return left_value == right_value;
            }
            else if (strcmp(op, "!=") == 0)
            {
                return left_value != right_value;
            }
            else if (strcmp(op, "<") == 0)
            {
                return left_value < right_value;
            }
            else if (strcmp(op, ">") == 0)
            {
                return left_value > right_value;
            }
            else if (strcmp(op, "<=") == 0)
            {
                return left_value <= right_value;
            }
            else if (strcmp(op, ">=") == 0)
            {
                return left_value >= right_value;
            }
            else
            {
                tlang_error("Unknown comparison operator");
                return false;
            }
        }
    }
    else
    {

        int left_value = 0;

        if (is_digit(**ptr) || **ptr == '-')
        {
            left_value = parse_int_expression(ptr);
        }

        skip_whitespace(ptr);

        char op[3] = {0};
        int op_len = 0;

        while (**ptr && (**ptr == '<' || **ptr == '>' || **ptr == '=' || **ptr == '!') && op_len < 2)
        {
            op[op_len++] = **ptr;
            (*ptr)++;
        }

        skip_whitespace(ptr);

        int right_value = 0;

        if (is_digit(**ptr) || **ptr == '-')
        {
            right_value = parse_int_expression(ptr);
        }
        else if (is_alpha(**ptr))
        {
            char var_name[32];
            int i = 0;

            while (is_alnum(**ptr) && i < 31)
            {
                var_name[i++] = **ptr;
                (*ptr)++;
            }
            var_name[i] = '\0';

            TLANG_Variable *var = find_variable(var_name);
            if (var && var->type == TLANG_INT)
            {
                right_value = var->value.int_value;
            }
            else if (var && var->type == TLANG_BOOL)
            {
                right_value = var->value.bool_value ? 1 : 0;
            }
            else
            {
                tlang_error("Variable not found in condition");
                return false;
            }
        }

        if (strcmp(op, "==") == 0 || strcmp(op, "=") == 0)
        {
            return left_value == right_value;
        }
        else if (strcmp(op, "!=") == 0)
        {
            return left_value != right_value;
        }
        else if (strcmp(op, "<") == 0)
        {
            return left_value < right_value;
        }
        else if (strcmp(op, ">") == 0)
        {
            return left_value > right_value;
        }
        else if (strcmp(op, "<=") == 0)
        {
            return left_value <= right_value;
        }
        else if (strcmp(op, ">=") == 0)
        {
            return left_value >= right_value;
        }
        else
        {
            tlang_error("Unknown comparison operator");
            return false;
        }
    }
}

static void process_int_declaration(const char **ptr)
{
    skip_whitespace(ptr);

    char var_name[32];
    int i = 0;

    while (is_alnum(**ptr) && i < 31)
    {
        var_name[i++] = **ptr;
        (*ptr)++;
    }
    var_name[i] = '\0';

    skip_whitespace(ptr);

    if (**ptr != '=')
    {
        tlang_error("Expected '=' in int declaration");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    int value = parse_int_expression(ptr);

    TLANG_Variable *var = find_variable(var_name);
    if (!var)
    {
        var = create_variable(var_name, TLANG_INT);
    }

    if (var)
    {
        var->value.int_value = value;
    }
}

static void process_schar_declaration(const char **ptr)
{
    skip_whitespace(ptr);

    char var_name[32];
    int i = 0;

    while (is_alnum(**ptr) && i < 31)
    {
        var_name[i++] = **ptr;
        (*ptr)++;
    }
    var_name[i] = '\0';

    skip_whitespace(ptr);

    if (**ptr != '=')
    {
        tlang_error("Expected '=' in schar declaration");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    char *value = parse_string_literal(ptr);
    if (!value)
    {
        tlang_error("Expected string literal");
        return;
    }

    TLANG_Variable *var = find_variable(var_name);
    if (!var)
    {
        var = create_variable(var_name, TLANG_SCHAR);
    }

    if (var)
    {
        static char string_buffer[256];
        strcpy(string_buffer, value);
        var->value.str_value = string_buffer;
    }
}

static void process_bool_declaration(const char **ptr)
{
    skip_whitespace(ptr);

    char var_name[32];
    int i = 0;

    while (is_alnum(**ptr) && i < 31)
    {
        var_name[i++] = **ptr;
        (*ptr)++;
    }
    var_name[i] = '\0';

    skip_whitespace(ptr);

    if (**ptr != '=')
    {
        tlang_error("Expected '=' in bool declaration");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    bool value = parse_bool_literal(ptr);

    TLANG_Variable *var = find_variable(var_name);
    if (!var)
    {
        var = create_variable(var_name, TLANG_BOOL);
    }

    if (var)
    {
        var->value.bool_value = value;
    }
}

static void process_write_command(const char **ptr)
{
    skip_whitespace(ptr);

    if (**ptr != '(')
    {
        tlang_error("Expected '(' after write");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    while (**ptr && **ptr != ')')
    {
        skip_whitespace(ptr);

        if (**ptr == '"')
        {

            char *str = parse_string_literal(ptr);
            if (str)
            {
                terminal_writestring(str);
            }
        }
        else if (is_alpha(**ptr))
        {

            const char *start = *ptr;

            char var_name[32];
            int i = 0;
            while (is_alnum(**ptr) && i < 31)
            {
                var_name[i++] = **ptr;
                (*ptr)++;
            }
            var_name[i] = '\0';

            skip_whitespace(ptr);

            if (**ptr == '+' || **ptr == '-' || **ptr == '*' || **ptr == '/')
            {

                *ptr = start;
                int result = evaluate_math_expression(ptr);

                char num_str[16];
                itoa(result, num_str, 10);
                terminal_writestring(num_str);
            }
            else
            {

                *ptr = start;

                i = 0;
                while (is_alnum(**ptr) && i < 31)
                {
                    var_name[i++] = **ptr;
                    (*ptr)++;
                }
                var_name[i] = '\0';

                TLANG_Variable *var = find_variable(var_name);
                if (var)
                {
                    switch (var->type)
                    {
                    case TLANG_INT:
                    {
                        char num_str[16];
                        itoa(var->value.int_value, num_str, 10);
                        terminal_writestring(num_str);
                        break;
                    }
                    case TLANG_SCHAR:
                        terminal_writestring(var->value.str_value);
                        break;
                    case TLANG_BOOL:
                        terminal_writestring(var->value.bool_value ? "true" : "false");
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    char error_msg[64];
                    strcpy(error_msg, "Undefined variable: ");
                    strcat(error_msg, var_name);
                    tlang_error(error_msg);
                }
            }
        }
        else if (is_digit(**ptr) || **ptr == '-')
        {

            int result = evaluate_math_expression(ptr);

            char num_str[16];
            itoa(result, num_str, 10);
            terminal_writestring(num_str);
        }
        else if (**ptr == ',')
        {

            (*ptr)++;
            skip_whitespace(ptr);

            continue;
        }
        else if (is_whitespace(**ptr))
        {

            (*ptr)++;
        }
        else
        {

            char error_msg[40];
            strcpy(error_msg, "Unexpected character in write: '");
            error_msg[31] = **ptr;
            error_msg[32] = '\'';
            error_msg[33] = '\0';
            tlang_error(error_msg);
            (*ptr)++;
        }
    }

    if (**ptr == ')')
    {
        (*ptr)++;
    }

    terminal_writestring("\n");
}

static void process_input_command(const char **ptr)
{
    skip_whitespace(ptr);

    if (**ptr != '(')
    {
        tlang_error("Expected '(' after input");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    char var_name[32];
    int i = 0;

    while (is_alnum(**ptr) && i < 31)
    {
        var_name[i++] = **ptr;
        (*ptr)++;
    }
    var_name[i] = '\0';

    skip_whitespace(ptr);

    if (**ptr != ')')
    {
        tlang_error("Expected ')' after input");
        return;
    }

    (*ptr)++;

    TLANG_Variable *var = find_variable(var_name);
    if (!var)
    {
        tlang_error("Variable not found for input");
        return;
    }

    terminal_writestring("Input> ");

    char input_buffer[256];
    int pos = 0;

    while (pos < 255)
    {
        if (keyboard_available())
        {
            char c = keyboard_getchar();

            if (c == '\n' || c == '\r')
            {
                terminal_writestring("\n");
                break;
            }
            else if (c == 8 && pos > 0)
            {
                pos--;
                terminal_putchar(8);
                terminal_putchar(' ');
                terminal_putchar(8);
            }
            else if (c >= 32 && c <= 126)
            {
                input_buffer[pos++] = c;
                terminal_putchar(c);
            }
        }
    }

    input_buffer[pos] = '\0';

    switch (var->type)
    {
    case TLANG_INT:
        var->value.int_value = simple_atoi(input_buffer);
        break;
    case TLANG_SCHAR:
        strcpy((char *)var->value.str_value, input_buffer);
        break;
    case TLANG_BOOL:
        var->value.bool_value = (strcmp(input_buffer, "true") == 0 ||
                                 strcmp(input_buffer, "1") == 0);
        break;
    default:
        break;
    }
}

static void process_if_statement(const char **ptr, bool *condition_met)
{
    skip_whitespace(ptr);

    bool condition_result = evaluate_condition(ptr);

    skip_whitespace(ptr);

    if (**ptr != ':')
    {
        tlang_error("Expected ':' after if condition");
        return;
    }

    (*ptr)++;

    *condition_met = condition_result;
}

static void process_for_loop(const char **ptr)
{
    skip_whitespace(ptr);

    char var_name[32];
    int i = 0;

    while (is_alnum(**ptr) && i < 31)
    {
        var_name[i++] = **ptr;
        (*ptr)++;
    }
    var_name[i] = '\0';

    skip_whitespace(ptr);

    if (**ptr != '=')
    {
        tlang_error("Expected '=' in for loop");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    int start_value = parse_int_expression(ptr);

    skip_whitespace(ptr);

    if (**ptr != ';')
    {
        tlang_error("Expected ';' in for loop");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    const char *condition_start = *ptr;

    while (**ptr && **ptr != ';')
    {
        (*ptr)++;
    }

    if (**ptr != ';')
    {
        tlang_error("Expected ';' in for loop");
        return;
    }

    (*ptr)++;
    skip_whitespace(ptr);

    char increment[32];
    i = 0;

    while (**ptr && **ptr != ':' && i < 31)
    {
        increment[i++] = **ptr;
        (*ptr)++;
    }
    increment[i] = '\0';

    if (**ptr != ':')
    {
        tlang_error("Expected ':' after for loop");
        return;
    }

    (*ptr)++;

    const char *loop_body = *ptr;

    TLANG_Variable *loop_var = find_variable(var_name);
    if (!loop_var)
    {
        loop_var = create_variable(var_name, TLANG_INT);
    }

    if (!loop_var)
    {
        return;
    }

    loop_var->value.int_value = start_value;

    while (1)
    {
        const char *cond_ptr = condition_start;
        bool condition_result = evaluate_condition(&cond_ptr);

        if (!condition_result)
        {
            break;
        }

        const char *body_ptr = loop_body;

        while (*body_ptr && *body_ptr != '\n')
        {
            if (is_alpha(*body_ptr))
            {
                if (strncmp(body_ptr, "write(", 6) == 0)
                {
                    body_ptr += 5;
                    process_write_command(&body_ptr);
                }
            }
            else
            {
                body_ptr++;
            }
        }

        char *found = simple_strstr(increment, "++");
        if (found)
        {
            loop_var->value.int_value++;
        }
        else
        {
            found = simple_strstr(increment, "--");
            if (found)
            {
                loop_var->value.int_value--;
            }
            else
            {
                found = simple_strstr(increment, "+=");
                if (found)
                {
                    const char *val_str = found + 2;
                    int add_value = simple_atoi(val_str);
                    loop_var->value.int_value += add_value;
                }
            }
        }
    }
}

void tlang_run_line(const char *line)
{
    interpreter.line_number++;
    const char *ptr = line;

    skip_whitespace(&ptr);

    if (!*ptr || *ptr == '#')
        return;

    if (*ptr == '@')
    {
        uint8_t warn_color = vga_entry_color(VGA_COLOR_RED, VGA_COLOR_LIGHT_GREY);
        uint8_t text_color = vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_LIGHT_GREY);
        terminal_setcolor(warn_color);
        terminal_writestring("[WARN] ");

        ptr++;
        skip_whitespace(&ptr);

        while (*ptr && *ptr != '\n' && *ptr != '\r')
        {
            terminal_putchar(*ptr);
            ptr++;
        }

        terminal_writestring("\n");
        terminal_setcolor(text_color);
        return;
    }

    static bool skip_mode = false;
    static int if_depth = 0;
    static bool else_found = false;

    if (strncmp(ptr, "line", 4) == 0 && (ptr[4] == '\0' || is_whitespace(ptr[4]) || ptr[4] == ';' || ptr[4] == '('))
    {
        if (!skip_mode)
        {
            if (ptr[4] == '(')
            {

                ptr += 4;
                process_line_command(&ptr);
            }
            else
            {

                terminal_writestring("\n");
            }
        }
        return;
    }

    if (strncmp(ptr, "int ", 4) == 0 ||
        strncmp(ptr, "schar ", 6) == 0 ||
        strncmp(ptr, "bool ", 5) == 0)
    {
        skip_mode = false;
        if_depth = 0;
        else_found = false;
    }

    if (skip_mode)
    {

        if (strncmp(ptr, "elif ", 5) == 0)
        {

            ptr += 5;
            bool condition_result = evaluate_condition(&ptr);

            skip_whitespace(&ptr);
            if (*ptr == ':')
                ptr++;

            if (condition_result)
            {

                skip_mode = false;
            }

            return;
        }
        else if (strncmp(ptr, "else", 4) == 0)
        {

            skip_mode = false;
            else_found = true;
            return;
        }
        else if (strncmp(ptr, "if ", 3) == 0)
        {

            if_depth++;
        }

        return;
    }

    if (strncmp(ptr, "int ", 4) == 0)
    {
        ptr += 4;
        process_int_declaration(&ptr);
        return;
    }
    else if (strncmp(ptr, "schar ", 6) == 0)
    {
        ptr += 6;
        process_schar_declaration(&ptr);
        return;
    }
    else if (strncmp(ptr, "bool ", 5) == 0)
    {
        ptr += 5;
        process_bool_declaration(&ptr);
        return;
    }
    else if (strncmp(ptr, "write(", 6) == 0)
    {
        ptr += 5;
        process_write_command(&ptr);
        return;
    }
    else if (strncmp(ptr, "input(", 6) == 0)
    {
        ptr += 5;
        process_input_command(&ptr);
        return;
    }
    else if (strncmp(ptr, "line(", 5) == 0)
    {
        ptr += 4;
        process_line_command(&ptr);
        return;
    }

    if (strncmp(ptr, "if ", 3) == 0)
    {
        ptr += 3;
        bool condition_result = evaluate_condition(&ptr);

        skip_whitespace(&ptr);
        if (*ptr == ':')
            ptr++;

        if (!condition_result)
        {

            skip_mode = true;
            if_depth = 1;
            else_found = false;
        }

        return;
    }
    else if (strncmp(ptr, "elif ", 5) == 0)
    {

        ptr += 5;
        bool condition_result = evaluate_condition(&ptr);

        skip_whitespace(&ptr);
        if (*ptr == ':')
            ptr++;

        if (!condition_result)
        {

            skip_mode = true;
        }
        else
        {

            skip_mode = false;
        }
        return;
    }
    else if (strncmp(ptr, "else", 4) == 0)
    {

        ptr += 4;
        skip_whitespace(&ptr);
        if (*ptr == ':')
            ptr++;

        skip_mode = false;
        else_found = true;
        return;
    }
    else if (strncmp(ptr, "for ", 4) == 0)
    {
        ptr += 4;
        process_for_loop(&ptr);
        return;
    }
    else if (strncmp(ptr, "random(", 7) == 0)
    {
        ptr += 6;
        process_random_command(&ptr);
        return;
    }

    else if (strncmp(ptr, "seed(", 5) == 0)
    {
        ptr += 4;
        process_seed_command(&ptr);
        return;
    }

    char var_name[32];
    int i = 0;

    while (is_alnum(*ptr) && i < 31)
    {
        var_name[i++] = *ptr;
        ptr++;
    }
    var_name[i] = '\0';

    TLANG_Variable *var = find_variable(var_name);
    if (var)
    {
        skip_whitespace(&ptr);

        if (*ptr == '=')
        {
            ptr++;
            skip_whitespace(&ptr);

            switch (var->type)
            {
            case TLANG_INT:
                var->value.int_value = evaluate_math_expression(&ptr);
                break;
            case TLANG_SCHAR:
                var->value.str_value = parse_string_literal(&ptr);
                break;
            case TLANG_BOOL:
                var->value.bool_value = parse_bool_literal(&ptr);
                break;
            default:
                break;
            }
            return;
        }
    }

    char error_msg[64];
    strcpy(error_msg, "Unknown command at line ");
    char line_num[8];
    itoa(interpreter.line_number, line_num, 10);
    strcat(error_msg, line_num);
    tlang_error(error_msg);
}

void tlang_run_file(const char *filename)
{
    File *file = fs_find_file(filename);

    if (!file || file->type != 'F')
    {
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return;
    }

    terminal_writestring("\n=== Running ");
    terminal_writestring(filename);
    terminal_writestring(" ===\n\n");

    interpreter.var_count = 0;
    interpreter.line_number = 0;
    interpreter.had_error = false;

    char *content = file->content;
    int pos = 0;
    int line_start = 0;

    while (pos < file->size)
    {
        if (content[pos] == '\n' || pos == file->size - 1)
        {
            int line_len = pos - line_start;
            if (pos == file->size - 1 && content[pos] != '\n')
                line_len++;

            char line[256];
            int i;
            for (i = 0; i < line_len && i < 255; i++)
            {
                line[i] = content[line_start + i];
            }
            line[i] = '\0';

            tlang_run_line(line);

            line_start = pos + 1;
        }
        pos++;
    }

    terminal_writestring("\n=== ");

    if (interpreter.had_error)
    {
        terminal_writestring("Finished with errors ===\n");
    }
    else
    {
        terminal_writestring("Successfully executed ===\n");
    }
}

void tlang_init(void)
{
    interpreter.var_count = 0;
    interpreter.line_number = 0;
    interpreter.had_error = false;
}

void tlang_cleanup(void)
{
    interpreter.var_count = 0;
}

void cmd_tlang(const char *args)
{
    if (!args || !*args)
    {
        terminal_writestring("Usage: tlang <file.T> or tlang run <code>\n");
        terminal_writestring("Examples:\n");
        terminal_writestring("  tlang script.T          - Run a .T file\n");
        terminal_writestring("  tlang run int x = 10    - Run single line\n");
        return;
    }

    if (strncmp(args, "run ", 4) == 0)
    {
        const char *code = args + 4;
        tlang_init();
        tlang_run_line(code);
        tlang_cleanup();
    }
    else
    {
        int len = strlen(args);
        if (len < 3 || !(args[len - 2] == '.' && args[len - 1] == 'T'))
        {
            terminal_writestring("Error: File must have .T extension\n");
            return;
        }

        tlang_init();
        tlang_run_file(args);
        tlang_cleanup();
    }
}