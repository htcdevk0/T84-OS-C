#include "vga.h"
#include "keyboard.h"
#include "string_utils.h"
#include "panic.h"
#include "examples/circle.h"
#include "bootscreen.h"
#include "variables.h"
#include "pre_defined.h"
#include "ramfs.h"
#include "commands.h"
#include "misc.h"
#include "tlang.h"

#include "../T84_OS/home/app/ttest.h"
#include "../T84_OS/home/app/4IDE.h"
#include "../T84_OS/home/app/cstat.h"

bool usedDailyPhrase = false;

void cmd_echo(const char *args)
{
    if (args)
    {
        terminal_writestring("\n");
        terminal_writestring(args);
    }
    terminal_writestring("\n");
}

void cmd_sum(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\nUsage: sum <num1> <num2>\n");
        terminal_writestring("Example: sum 5 3\n");
        return;
    }

    char arg1[32], arg2[32];
    int n1, n2;

    if (sscanf(args, "%s %s", arg1, arg2) != 2)
    {
        terminal_writestring("\nError: Need two numbers\n");
        return;
    }

    n1 = atoi(arg1);
    n2 = atoi(arg2);

    int result = n1 + n2;
    terminal_writeall("\n%d + %d = %d\n", n1, n2, result);
}

void cmd_sub(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\nUsage: subtract <num1> <num2>\n");
        terminal_writestring("Example: subtract 5 3\n");
        return;
    }

    char arg1[32], arg2[32];
    int n1, n2;

    if (sscanf(args, "%s %s", arg1, arg2) != 2)
    {
        terminal_writestring("\nError: Need two numbers\n");
        return;
    }

    n1 = atoi(arg1);
    n2 = atoi(arg2);

    int result = n1 - n2;
    terminal_writeall("\n%d + %d = %d\n", n1, n2, result);
}

void cmd_schar(const char *args)
{
    cmd_string(args);
}

void cmd_run_example(const char *example_name)
{
    if (!example_name || strlen(example_name) == 0)
    {
        terminal_writestring("\nUsage: run examples/<name>\n");
        terminal_writestring("Available examples:\n");
        terminal_writestring("  circle     - Draw a circle with text\n");
        terminal_writestring("  (more coming soon)\n");
        return;
    }

    const char *example = example_name;
    if (strncmp(example_name, "examples/", 9) == 0)
    {
        example = example_name + 9;
    }

    if (strcmp(example, "circle") == 0)
    {
        terminal_writestring("\nLoading Circle Example...\n");

        for (volatile int i = 0; i < 1000000; i++)
            ;

        circle_example_run();

        terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("\nReturned from Circle Example.\n");
    }
    else
    {
        terminal_writestring("\nExample not found: ");
        terminal_writestring(example);
        terminal_writestring("\n");
    }
}

void cmd_div(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\nUsage: divide <num1> <num2>\n");
        terminal_writestring("Example: divide 5 3\n");
        return;
    }

    char arg1[32], arg2[32];
    int n1, n2;

    if (sscanf(args, "%s %s", arg1, arg2) != 2)
    {
        terminal_writestring("\nError: Need two numbers\n");
        return;
    }

    n1 = atoi(arg1);
    n2 = atoi(arg2);

    if (n1 == 0 || n2 == 0)
    {
        panic_with_code("Divide by zero exception", 0x00);
    }
    int result = n1 / n2;
    terminal_writeall("\n%d / %d = %d\n", n1, n2, result);
}

void shutdown_system(void)
{
    terminal_writestring("\n=== Shutting down T84 OS ===\n");
    terminal_writestring("System powering off...\n");

    for (volatile int i = 0; i < 10000000; i++)
        ;

    __asm__ volatile("outw %0, %1" : : "a"((uint16_t)0x2000), "Nd"((uint16_t)0x604));

    __asm__ volatile("mov $0x5301, %ax");
    __asm__ volatile("xor %bx, %bx");
    __asm__ volatile("int $0x15");

    __asm__ volatile("mov $0x530e, %ax");
    __asm__ volatile("xor %bx, %bx");
    __asm__ volatile("int $0x15");

    __asm__ volatile("mov $0x5307, %ax");
    __asm__ volatile("mov $0x0001, %bx");
    __asm__ volatile("mov $0x0003, %cx");
    __asm__ volatile("int $0x15");

    terminal_writestring("Shutdown failed. Halting CPU...\n");
    __asm__ volatile("cli");
    __asm__ volatile("hlt");

    while (1)
        __asm__ volatile("hlt");
}

void cmd_help(void)
{
    terminal_writestring("\n=== T84 OS Commands ===\n");
    terminal_writestring("help                - Show this help\n");
    terminal_writestring("clear, cls          - Clear screen\n");
    terminal_writestring("echo <text>         - Print text\n");
    terminal_writestring("print <text>        - Alias for echo\n");
    terminal_writestring("about               - About T84 OS\n");
    terminal_writestring("exit, quit          - Show exit message\n");
    terminal_writestring("shutdown 0          - Shutdown kernel\n");
    terminal_writestring("run (NOTE: Only for examples [run examples/avaliable examples])\n");
    terminal_writestring("\nUse ARROW KEYS to move cursor (#)\n");
    terminal_writestring("Hold SHIFT for uppercase letters\n");
}

void cmd_clear(int mode)
{

    uint8_t bg_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_LIGHT_GREY);
    uint8_t header_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    uint8_t line_color = vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_LIGHT_GREY);

    if (mode)
    {

        terminal_clear_color(bg_color);

        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_putentryat(' ', header_color, x, 0);
        }

        const char *title = "T84 OS";
        int title_len = 6;
        int title_x = (VGA_WIDTH - title_len) / 2;
        terminal_set_cursor(title_x, 0);
        terminal_setcolor(header_color);
        terminal_writestring(title);

        draw_line(0, 1, VGA_WIDTH, line_color, true);

        terminal_set_cursor(0, 3);
    }
    else
    {

        terminal_clear_color(bg_color);

        terminal_set_cursor(0, 0);
    }
}

void cmd_int(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\nUsage: INT <name> = <value>\n");
        terminal_writestring("Example: INT x = 10\n");
        return;
    }

    char name[32];
    int value;

    if (sscanf(args, "%s = %d", name, &value) != 2)
    {
        terminal_writestring("\nError: Syntax: INT <name> = <value>\n");
        return;
    }

    if (var_set_int(name, value))
    {
        terminal_writestring("\nInteger variable '");
        terminal_writestring(name);
        terminal_writestring("' set to ");
        char val_str[16];
        itoa(value, val_str, 10);
        terminal_writestring(val_str);
        terminal_writestring("\n");
    }
    else
    {
        terminal_writestring("\nError: Failed to create variable\n");
    }
}

void cmd_string(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\nUsage: STRING <name> = \"<value>\"\n");
        terminal_writestring("Example: STRING name = \"John\"\n");
        return;
    }

    char name[32];
    char value[256];

    const char *equals = strchr(args, '=');
    if (!equals)
    {
        terminal_writestring("\nError: Missing '='\n");
        return;
    }

    int name_len = equals - args;
    if (name_len > 31)
        name_len = 31;

    for (int i = 0; i < name_len; i++)
    {
        name[i] = args[i];
    }
    name[name_len] = '\0';

    while (name_len > 0 && name[name_len - 1] == ' ')
    {
        name[--name_len] = '\0';
    }

    const char *quote1 = strchr(equals, '"');
    if (!quote1)
    {
        terminal_writestring("\nError: String value must be in quotes\n");
        return;
    }

    const char *quote2 = strchr(quote1 + 1, '"');
    if (!quote2)
    {
        terminal_writestring("\nError: Missing closing quote\n");
        return;
    }

    int value_len = quote2 - quote1 - 1;
    if (value_len > 255)
        value_len = 255;

    for (int i = 0; i < value_len; i++)
    {
        value[i] = quote1[i + 1];
    }
    value[value_len] = '\0';

    if (var_set_string(name, value))
    {
        terminal_writestring("\nString variable '");
        terminal_writestring(name);
        terminal_writestring("' = \"");
        terminal_writestring(value);
        terminal_writestring("\"\n");
    }
    else
    {
        terminal_writestring("\nError: Failed to create variable\n");
    }
}

void cmd_print_var(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\nUsage: &<varname>\n");
        terminal_writestring("Example: &x\n");
        return;
    }

    const char *varname = args;
    if (args[0] == '&')
        varname = args + 1;

    terminal_writestring("\n");

    terminal_writestring("DEBUG: Looking for variable '");
    terminal_writestring(varname);
    terminal_writestring("'\n");

    Variable *var = var_get(varname);
    if (var)
    {
        terminal_writestring("DEBUG: Found variable, type=");
        switch (var->type)
        {
        case VAR_INT:
            terminal_writestring("INT");
            break;
        case VAR_STRING:
            terminal_writestring("STRING");
            break;
        case VAR_FLOAT:
            terminal_writestring("FLOAT");
            break;
        case VAR_BOOL:
            terminal_writestring("BOOL");
            break;
        case VAR_NULL:
            terminal_writestring("NULL");
            break;
        }
        terminal_writestring("\nDEBUG: Value=");
    }
    else
    {
        terminal_writestring("DEBUG: Variable not found\n");
    }

    print_var_by_name(varname);
    terminal_writestring("\n");
}

void trim_spaces(char *str)
{
    int start = 0, end = strlen(str) - 1;

    while (str[start] == ' ')
        start++;

    while (end >= 0 && str[end] == ' ')
    {
        str[end] = '\0';
        end--;
    }

    if (start > 0)
    {
        int i = 0;
        while (str[start + i])
        {
            str[i] = str[start + i];
            i++;
        }
        str[i] = '\0';
    }
}

char *find_substring(const char *str, const char *substr)
{
    int sublen = strlen(substr);
    if (sublen == 0)
        return (char *)str;

    for (int i = 0; str[i]; i++)
    {
        int j;
        for (j = 0; j < sublen && str[i + j] && str[i + j] == substr[j]; j++)
            ;
        if (j == sublen)
            return (char *)&str[i];
    }
    return NULL;
}

int string_to_int(const char *str)
{
    int result = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-')
    {
        sign = -1;
        i = 1;
    }

    for (; str[i]; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            result = result * 10 + (str[i] - '0');
        }
        else
        {
            break;
        }
    }

    return result * sign;
}

bool compare_strings(const char *str1, const char *str2)
{

    const char *s1 = str1;
    const char *s2 = str2;

    if (str1[0] == '"' && str1[strlen(str1) - 1] == '"')
    {
        s1 = str1 + 1;

        char temp1[256];
        int len1 = strlen(str1) - 2;
        if (len1 > 255)
            len1 = 255;
        for (int i = 0; i < len1; i++)
            temp1[i] = str1[i + 1];
        temp1[len1] = '\0';
        s1 = temp1;
    }

    if (str2[0] == '"' && str2[strlen(str2) - 1] == '"')
    {
        s2 = str2 + 1;

        char temp2[256];
        int len2 = strlen(str2) - 2;
        if (len2 > 255)
            len2 = 255;
        for (int i = 0; i < len2; i++)
            temp2[i] = str2[i + 1];
        temp2[len2] = '\0';
        s2 = temp2;
    }

    return strcmp(s1, s2) == 0;
}

bool evaluate_condition(const char *condition)
{
    char cond[256];
    strcpy(cond, condition);
    trim_spaces(cond);

    if (strcmp(cond, "true") == 0)
        return true;
    if (strcmp(cond, "false") == 0)
        return false;

    char *operators[] = {"==", "!=", ">", "<", ">=", "<="};
    char *found_op = NULL;
    int op_index = -1;

    for (int i = 0; i < 6; i++)
    {
        char *op_pos = find_substring(cond, operators[i]);
        if (op_pos)
        {
            found_op = operators[i];
            op_index = i;
            break;
        }
    }

    if (!found_op)
    {

        if (cond[0] == '&')
        {
            Variable *var = var_get(cond + 1);
            if (var)
            {
                if (var->type == VAR_INT)
                    return var->value.int_value != 0;
                if (var->type == VAR_BOOL)
                    return var->value.bool_value;
                if (var->type == VAR_STRING && var->value.string_value)
                    return strlen(var->value.string_value) > 0;
            }
        }
        return false;
    }

    int op_len = strlen(found_op);
    char *op_pos = find_substring(cond, found_op);
    int left_len = op_pos - cond;

    char left_part[128];
    char right_part[128];

    for (int i = 0; i < left_len; i++)
        left_part[i] = cond[i];
    left_part[left_len] = '\0';
    trim_spaces(left_part);

    const char *right_start = op_pos + op_len;
    int right_len = 0;
    while (right_start[right_len] && right_len < 127)
    {
        right_part[right_len] = right_start[right_len];
        right_len++;
    }
    right_part[right_len] = '\0';
    trim_spaces(right_part);

    int left_val = 0, right_val = 0;
    bool left_is_var = false, right_is_var = false;
    Variable *left_var = NULL, *right_var = NULL;

    if (left_part[0] == '&')
    {
        left_var = var_get(left_part + 1);
        if (left_var && left_var->type == VAR_INT)
        {
            left_val = left_var->value.int_value;
            left_is_var = true;
        }
    }
    else
    {
        left_val = string_to_int(left_part);
    }

    if (right_part[0] == '&')
    {
        right_var = var_get(right_part + 1);
        if (right_var && right_var->type == VAR_INT)
        {
            right_val = right_var->value.int_value;
            right_is_var = true;
        }
    }
    else
    {
        right_val = string_to_int(right_part);
    }

    switch (op_index)
    {
    case 0:
        if (left_is_var && right_is_var && left_var->type == VAR_STRING && right_var->type == VAR_STRING)
            return strcmp(left_var->value.string_value, right_var->value.string_value) == 0;
        return left_val == right_val;
    case 1:
        return left_val != right_val;
    case 2:
        return left_val > right_val;
    case 3:
        return left_val < right_val;
    case 4:
        return left_val >= right_val;
    case 5:
        return left_val <= right_val;
    }

    return false;
}

void cmd_if(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\n=== IF COMMAND ===\n");
        terminal_writestring("Syntax: IF <condition> THEN <command>\n\n");
        terminal_writestring("Conditions:\n");
        terminal_writestring("  true / false\n");
        terminal_writestring("  &var == value    (equal)\n");
        terminal_writestring("  &var != value    (not equal)\n");
        terminal_writestring("  &var > value     (greater than)\n");
        terminal_writestring("  &var < value     (less than)\n");
        terminal_writestring("  &var >= value    (greater or equal)\n");
        terminal_writestring("  &var <= value    (less or equal)\n\n");
        terminal_writestring("Examples:\n");
        terminal_writestring("  IF true THEN echo Hello\n");
        terminal_writestring("  IF &x == 10 THEN echo x_is_ten\n");
        terminal_writestring("  IF &y > 5 THEN echo y_greater_than_5\n");
        terminal_writestring("  IF &name == \"T84\" THEN echo correct_os\n");
        return;
    }

    char *then_pos = NULL;
    for (int i = 0; args[i]; i++)
    {
        if (args[i] == ' ' &&
            args[i + 1] == 'T' && args[i + 2] == 'H' &&
            args[i + 3] == 'E' && args[i + 4] == 'N' &&
            args[i + 5] == ' ')
        {
            then_pos = (char *)&args[i];
            break;
        }
    }

    if (!then_pos)
    {
        terminal_writestring("\nError: Missing ' THEN ' in IF statement\n");
        return;
    }

    char condition[256];
    char command[256];

    int cond_len = then_pos - args;
    if (cond_len > 255)
        cond_len = 255;

    for (int i = 0; i < cond_len; i++)
    {
        condition[i] = args[i];
    }
    condition[cond_len] = '\0';

    const char *cmd_start = then_pos + 6;
    int cmd_len = 0;

    while (cmd_start[cmd_len] && cmd_len < 255)
    {
        command[cmd_len] = cmd_start[cmd_len];
        cmd_len++;
    }
    command[cmd_len] = '\0';

    bool result = evaluate_condition(condition);

    if (result)
    {
        terminal_writestring("\n[IF TRUE] ");

        char *cmd = command;
        char *cmd_args = NULL;

        for (int i = 0; command[i]; i++)
        {
            if (command[i] == ' ')
            {
                command[i] = '\0';
                cmd_args = &command[i + 1];
                break;
            }
        }

        if (strcmp(cmd, "echo") == 0 || strcmp(cmd, "print") == 0)
        {
            if (cmd_args)
            {

                char output[512];
                int out_idx = 0;

                for (int i = 0; cmd_args[i] && out_idx < 511; i++)
                {
                    if (cmd_args[i] == '&' && cmd_args[i + 1])
                    {

                        char var_name[32];
                        int var_idx = 0;
                        i++;

                        while (cmd_args[i] && cmd_args[i] != ' ' && var_idx < 31)
                        {
                            var_name[var_idx++] = cmd_args[i++];
                        }
                        var_name[var_idx] = '\0';
                        i--;

                        Variable *var = var_get(var_name);
                        if (var)
                        {
                            if (var->type == VAR_INT)
                            {
                                char val_str[16];
                                itoa(var->value.int_value, val_str, 10);
                                for (int j = 0; val_str[j] && out_idx < 511; j++)
                                {
                                    output[out_idx++] = val_str[j];
                                }
                            }
                            else if (var->type == VAR_STRING && var->value.string_value)
                            {
                                for (int j = 0; var->value.string_value[j] && out_idx < 511; j++)
                                {
                                    output[out_idx++] = var->value.string_value[j];
                                }
                            }
                        }
                        else
                        {
                            output[out_idx++] = '?';
                        }
                    }
                    else
                    {
                        output[out_idx++] = cmd_args[i];
                    }
                }
                output[out_idx] = '\0';
                terminal_writestring(output);
            }
            terminal_writestring("\n");
        }
        else if (cmd[0] == '&')
        {

            print_var_by_name(cmd + 1);
            terminal_writestring("\n");
        }
        else if (strcmp(cmd, "INT") == 0 && cmd_args)
        {

            cmd_int(cmd_args);
        }
        else if (strcmp(cmd, "STRING") == 0 && cmd_args)
        {

            cmd_string(cmd_args);
        }
        else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0)
        {
            cmd_clear(1);
        }
        else
        {
            terminal_writestring("Executing: ");
            terminal_writestring(command);
            terminal_writestring("\n");
            terminal_writestring("(Command execution inside IF is limited)\n");
        }
    }
    else
    {
        terminal_writestring("\n[IF FALSE] Condition not met: ");
        terminal_writestring(condition);
        terminal_writestring("\n");
    }
}

void execute_command_with_vars(const char *command)
{
    char cmd[256];
    char args[256];

    strcpy(cmd, command);
    char *space_pos = NULL;

    for (int i = 0; cmd[i]; i++)
    {
        if (cmd[i] == ' ')
        {
            cmd[i] = '\0';
            space_pos = &cmd[i + 1];
            break;
        }
    }

    if (space_pos)
    {
        strcpy(args, space_pos);
    }
    else
    {
        args[0] = '\0';
    }

    if (strcmp(cmd, "echo") == 0 || strcmp(cmd, "print") == 0)
    {
        terminal_writestring("\n");
        if (args[0])
        {

            char output[512];
            int idx = 0;

            for (int i = 0; args[i] && idx < 511; i++)
            {
                if (args[i] == '&' && args[i + 1])
                {
                    char var_name[32];
                    int var_idx = 0;
                    i++;

                    while (args[i] && args[i] != ' ' && var_idx < 31)
                    {
                        var_name[var_idx++] = args[i++];
                    }
                    var_name[var_idx] = '\0';
                    i--;

                    Variable *var = var_get(var_name);
                    if (var)
                    {
                        if (var->type == VAR_INT)
                        {
                            char val[16];
                            itoa(var->value.int_value, val, 10);
                            for (int j = 0; val[j] && idx < 511; j++)
                            {
                                output[idx++] = val[j];
                            }
                        }
                        else if (var->type == VAR_STRING && var->value.string_value)
                        {
                            for (int j = 0; var->value.string_value[j] && idx < 511; j++)
                            {
                                output[idx++] = var->value.string_value[j];
                            }
                        }
                    }
                    else
                    {
                        output[idx++] = '?';
                    }
                }
                else
                {
                    output[idx++] = args[i];
                }
            }
            output[idx] = '\0';
            terminal_writestring(output);
        }
        terminal_writestring("\n");
    }
    else if (cmd[0] == '&')
    {
        print_var_by_name(cmd + 1);
        terminal_writestring("\n");
    }
}

void cmd_for(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\n=== FOR LOOP ===\n");
        terminal_writestring("Syntax: FOR <var> = <start> TO <end> DO <command>\n\n");
        terminal_writestring("Parameters:\n");
        terminal_writestring("  var    - Variable name (will be created as INT)\n");
        terminal_writestring("  start  - Starting value (integer)\n");
        terminal_writestring("  end    - Ending value (integer)\n");
        terminal_writestring("  command- Command to execute each iteration\n\n");
        terminal_writestring("Examples:\n");
        terminal_writestring("  FOR i = 1 TO 10 DO echo &i\n");
        terminal_writestring("  FOR x = 0 TO 5 DO echo Counting: &x\n");
        terminal_writestring("  FOR n = 10 TO 0 DO echo Countdown: &n\n");
        terminal_writestring("  FOR idx = 1 TO 3 DO INT var&idx = &idx\n");
        return;
    }

    char *equals_pos = NULL;
    for (int i = 0; args[i]; i++)
    {
        if (args[i] == ' ' && args[i + 1] == '=' && args[i + 2] == ' ')
        {
            equals_pos = (char *)&args[i];
            break;
        }
    }

    if (!equals_pos)
    {
        terminal_writestring("\nError: Missing ' = ' in FOR statement\n");
        return;
    }

    char *to_pos = NULL;
    for (int i = (equals_pos - args) + 3; args[i]; i++)
    {
        if (args[i] == ' ' &&
            args[i + 1] == 'T' && args[i + 2] == 'O' &&
            args[i + 3] == ' ')
        {
            to_pos = (char *)&args[i];
            break;
        }
    }

    if (!to_pos)
    {
        terminal_writestring("\nError: Missing ' TO ' in FOR statement\n");
        return;
    }

    char *do_pos = NULL;
    for (int i = (to_pos - args) + 4; args[i]; i++)
    {
        if (args[i] == ' ' &&
            args[i + 1] == 'D' && args[i + 2] == 'O' &&
            args[i + 3] == ' ')
        {
            do_pos = (char *)&args[i];
            break;
        }
    }

    if (!do_pos)
    {
        terminal_writestring("\nError: Missing ' DO ' in FOR statement\n");
        return;
    }

    char var_name[32];
    char start_str[32];
    char end_str[32];
    char command[256];

    int var_len = equals_pos - args;
    if (var_len > 31)
        var_len = 31;

    for (int i = 0; i < var_len; i++)
    {
        var_name[i] = args[i];
    }
    var_name[var_len] = '\0';

    while (var_len > 0 && var_name[var_len - 1] == ' ')
    {
        var_name[--var_len] = '\0';
    }

    const char *start_ptr = equals_pos + 3;
    int start_len = to_pos - start_ptr;
    if (start_len > 31)
        start_len = 31;

    for (int i = 0; i < start_len; i++)
    {
        start_str[i] = start_ptr[i];
    }
    start_str[start_len] = '\0';

    const char *end_ptr = to_pos + 4;
    int end_len = do_pos - end_ptr;
    if (end_len > 31)
        end_len = 31;

    for (int i = 0; i < end_len; i++)
    {
        end_str[i] = end_ptr[i];
    }
    end_str[end_len] = '\0';

    const char *cmd_ptr = do_pos + 4;
    int cmd_len = 0;

    while (cmd_ptr[cmd_len] && cmd_len < 255)
    {
        command[cmd_len] = cmd_ptr[cmd_len];
        cmd_len++;
    }
    command[cmd_len] = '\0';

    int start_val = 0;
    int end_val = 0;

    for (int i = 0; start_str[i]; i++)
    {
        if (start_str[i] >= '0' && start_str[i] <= '9')
        {
            start_val = start_val * 10 + (start_str[i] - '0');
        }
    }

    for (int i = 0; end_str[i]; i++)
    {
        if (end_str[i] >= '0' && end_str[i] <= '9')
        {
            end_val = end_val * 10 + (end_str[i] - '0');
        }
    }

    int step = (start_val <= end_val) ? 1 : -1;

    terminal_writestring("\n[FOR LOOP] ");
    terminal_writestring(var_name);
    terminal_writestring(" from ");

    char num_str[16];
    itoa(start_val, num_str, 10);
    terminal_writestring(num_str);

    terminal_writestring(" to ");
    itoa(end_val, num_str, 10);
    terminal_writestring(num_str);

    terminal_writestring(" (step ");
    itoa(step, num_str, 10);
    terminal_writestring(num_str);
    terminal_writestring(")\n");

    for (int i = start_val; (step > 0) ? (i <= end_val) : (i >= end_val); i += step)
    {

        var_set_int(var_name, i);

        terminal_writestring("  Iteration ");
        itoa(i, num_str, 10);
        terminal_writestring(num_str);
        terminal_writestring(": ");

        char *cmd = command;
        char *cmd_args = NULL;

        char cmd_copy[256];
        strcpy(cmd_copy, command);

        for (int j = 0; cmd_copy[j]; j++)
        {
            if (cmd_copy[j] == ' ')
            {
                cmd_copy[j] = '\0';
                cmd_args = &cmd_copy[j + 1];
                break;
            }
        }

        if (strcmp(cmd, "echo") == 0 || strcmp(cmd, "print") == 0)
        {
            if (cmd_args)
            {

                char output[512];
                int out_idx = 0;

                for (int j = 0; cmd_args[j] && out_idx < 511; j++)
                {
                    if (cmd_args[j] == '&' && cmd_args[j + 1])
                    {

                        char loop_var_name[32];
                        int var_idx = 0;
                        j++;

                        while (cmd_args[j] && cmd_args[j] != ' ' && var_idx < 31)
                        {
                            loop_var_name[var_idx++] = cmd_args[j++];
                        }
                        loop_var_name[var_idx] = '\0';
                        j--;

                        Variable *var = var_get(loop_var_name);
                        if (var)
                        {
                            if (var->type == VAR_INT)
                            {
                                char val_str[16];
                                itoa(var->value.int_value, val_str, 10);
                                for (int k = 0; val_str[k] && out_idx < 511; k++)
                                {
                                    output[out_idx++] = val_str[k];
                                }
                            }
                            else if (var->type == VAR_STRING && var->value.string_value)
                            {
                                for (int k = 0; var->value.string_value[k] && out_idx < 511; k++)
                                {
                                    output[out_idx++] = var->value.string_value[k];
                                }
                            }
                        }
                        else
                        {
                            output[out_idx++] = '?';
                        }
                    }
                    else
                    {
                        output[out_idx++] = cmd_args[j];
                    }
                }
                output[out_idx] = '\0';
                terminal_writestring(output);
            }
            terminal_writestring("\n");
        }
        else if (cmd[0] == '&')
        {

            print_var_by_name(cmd + 1);
            terminal_writestring("\n");
        }
        else if (strcmp(cmd, "INT") == 0 && cmd_args)
        {

            char int_cmd[256];
            strcpy(int_cmd, cmd_args);

            char final_cmd[256];
            int final_idx = 0;

            for (int j = 0; int_cmd[j] && final_idx < 255; j++)
            {
                if (int_cmd[j] == '&' && int_cmd[j + 1])
                {

                    char dyn_var[32];
                    int dyn_idx = 0;
                    j++;

                    while (int_cmd[j] && int_cmd[j] != ' ' && int_cmd[j] != '=' && dyn_idx < 31)
                    {
                        dyn_var[dyn_idx++] = int_cmd[j++];
                    }
                    dyn_var[dyn_idx] = '\0';
                    j--;

                    Variable *dyn = var_get(dyn_var);
                    if (dyn && dyn->type == VAR_INT)
                    {
                        char dyn_val[16];
                        itoa(dyn->value.int_value, dyn_val, 10);
                        for (int k = 0; dyn_val[k] && final_idx < 255; k++)
                        {
                            final_cmd[final_idx++] = dyn_val[k];
                        }
                    }
                }
                else
                {
                    final_cmd[final_idx++] = int_cmd[j];
                }
            }
            final_cmd[final_idx] = '\0';

            cmd_int(final_cmd);
        }
        else
        {
            terminal_writestring("Executing: ");
            terminal_writestring(command);
            terminal_writestring("\n");
        }

        for (volatile int d = 0; d < 10000; d++)
            ;
    }

    terminal_writestring("[END FOR LOOP]\n");
}

void cmd_list_vars(void)
{
    vars_list();
}

void cmd_about(void)
{
    terminal_writestring("\n=== T84 OS ===\n");
    terminal_writestring("\n    Working!  \n");
}

void kernel_main(void)
{

    terminal_initialize();
    keyboard_init();
    fs_init();
    vars_init();

    uint8_t bg_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_LIGHT_GREY);
    uint8_t header_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    uint8_t line_color = vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_LIGHT_GREY);
    uint8_t text_color = vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_LIGHT_GREY);

    terminal_clear_color(bg_color);

    for (int x = 0; x < VGA_WIDTH; x++)
    {
        terminal_putentryat(' ', header_color, x, 0);
    }

    const char *title = "T84 OS";
    int title_len = 6;
    int title_x = (VGA_WIDTH - title_len) / 2;
    terminal_set_cursor(title_x, 0);
    terminal_setcolor(header_color);
    terminal_writestring(title);

    draw_line(0, 1, VGA_WIDTH, line_color, true);

    terminal_set_cursor(0, 3);

    terminal_setcolor(text_color);
    terminal_writestring("System initialized successfully.\n");
    terminal_writestring("Type 'help' for available commands.\n");
    terminal_writestring("Hold SHIFT for uppercase letters.\n");
    terminal_writestring("\n");

    char input[256];

    while (1)
    {
        terminal_setcolor(text_color);
        terminal_writestring("T84> ");

        keyboard_getline(input, sizeof(input));

        char *cmd = input;
        char *arg = NULL;

        for (int i = 0; input[i]; i++)
        {
            if (input[i] == ' ')
            {
                input[i] = '\0';
                arg = &input[i + 1];
                break;
            }
        }

        if (strcmp(cmd, "help") == 0)
        {
            cmd_help();
        }
        else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0)
        {
            if (arg && strcmp(arg, "max") == 0)
            {
                cmd_clear(0);
            }
            else
            {
                cmd_clear(1);
            }
        }
        else if (strcmp(cmd, "echo") == 0 || strcmp(cmd, "print") == 0)
        {
            if (arg)
            {
                terminal_writestring("\n");
                terminal_writestring(arg);
            }
            terminal_writestring("\n");
        }
        else if (strcmp(cmd, "about") == 0)
        {
            cmd_about();
        }
        else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0)
        {
            terminal_writestring("\nT84 OS kernel cannot be exited.\n");
            terminal_writestring("Use Ctrl+Alt+Delete to reboot the system.\n");
        }
        else if (strcmp(cmd, "shutdown") == 0)
        {
            if (arg && strcmp(arg, "0") == 0)
            {
                shutdown_system();
            }
            else if (arg && strcmp(arg, "-1") == 0)
            {
                panic_with_code("Shutdown with \"-1\" arguments exception.", 0x10);
            }
            else
            {
                terminal_writestring("\nUsage: shutdown 0\n");
                terminal_writestring("This will power off the system.\n");
            }
        }
        else if (strcmp(cmd, "kernel") == 0)
        {
            if (arg && strcmp(arg, "-version") == 0)
            {
                terminal_writestring("\nT84 Kernel version: 1.8.4 (NOTE: the x.8.4 is a reference of the kernel name)\n");
            }
            else
            {
                terminal_writestring("\nUsage: -version ");
                terminal_writestring("\nThis will show you the T84 Kernel version\n");
            }
        }
        else if (strcmp(cmd, "sum") == 0)
        {
            cmd_sum(arg);
        }
        else if (strcmp(cmd, "subtract") == 0)
        {
            cmd_sub(arg);
        }
        else if (strcmp(cmd, "divide") == 0)
        {
            cmd_div(arg);
        }
        else if (strcmp(cmd, "run") == 0)
        {
            if (arg && strncmp(arg, "examples/", 9) == 0)
            {
                cmd_run_example(arg);
            }
        }
        else if (strcmp(cmd, "INT") == 0 || strcmp(cmd, "int") == 0)
        {
            cmd_int(arg);
        }
        else if (strcmp(cmd, "STRING") == 0 || strcmp(cmd, "string") == 0 ||
                 strcmp(cmd, "SCHAR") == 0 || strcmp(cmd, "schar") == 0)
        {
            cmd_string(arg);
        }
        else if (strcmp(cmd, "DEC") == 0 || strcmp(cmd, "dec") == 0 ||
                 strcmp(cmd, "FLOAT") == 0 || strcmp(cmd, "float") == 0)
        {
            terminal_writestring("\nFLOAT variables coming soon...\n");
        }
        else if (cmd[0] == '&')
        {
            cmd_print_var(cmd);
        }
        else if (strcmp(cmd, "if") == 0 || strcmp(cmd, "IF") == 0)
        {
            cmd_if(arg);
        }
        else if (strcmp(cmd, "for") == 0 || strcmp(cmd, "FOR") == 0)
        {
            cmd_for(arg);
        }
        else if (strcmp(cmd, "vars") == 0 || strcmp(cmd, "variables") == 0)
        {
            cmd_list_vars();
        }
        else if (strcmp(cmd, "open") == 0)
        {
            if (arg && strcmp(arg, "ttest") == 0)
            {
                terminal_writestring("\n\n");
                load_ttest_app();
            }
            /* Adicione esta parte: */
            else if (arg && strncmp(arg, "4ide ", 5) == 0)
            {
                /* Chamar a função do 4IDE */
                cmd_open_ide(arg);
            }
        }
        else if (strcmp(cmd, "dir") == 0)
        {
            cmd_dir(arg);
        }
        else if (strcmp(cmd, "fs") == 0)
        {
            cmd_fs(arg);
        }
        else if (strcmp(cmd, "cd") == 0)
        {
            cmd_cd(arg);
        }
        else if (strcmp(cmd, "write") == 0)
        {
            cmd_write(arg);
        }
        else if (strcmp(cmd, "ls") == 0)
        {
            cmd_ls(arg);
        }
        else if (strcmp(cmd, "cat") == 0)
        {
            cmd_cat(arg);
        }
        else if (strcmp(cmd, "pwd") == 0)
        {
            cmd_pwd();
        }
        else if (strcmp(cmd, "tparse") == 0)
        {
            cmd_tparse(arg);
        }
        else if (strcmp(cmd, "tlang") == 0)
        {
            cmd_tlang(arg);
        }
        else if (strcmp(cmd, "daily_phrase") == 0)
        {
            static uint32_t rng_state = 0xA3F1C9E7;

            if (!(arg && strcmp(arg, "-force") == 0) && usedDailyPhrase)
            {
                terminal_writestring(
                    "You already used the daily phrase! \n"
                    "(TIP: Type daily_phrase -force to run anyway!)\n");
                return;
            }

            rng_state ^= rng_state << 13;
            rng_state ^= rng_state >> 17;
            rng_state ^= rng_state << 5;

            uint32_t phrase_index = rng_state % DAILY_PHRASES_COUNT;

            terminal_writeall("\nDaily phrase: ");
            terminal_writeall(daily_phrases[phrase_index]);
            terminal_writeall("\n");

            usedDailyPhrase = true;
        }
        else if (strcmp(cmd, "cstat") == 0)
        {
            cmd_cstat(arg);
        }
        else if (cmd[0] != '\0')
        {
            terminal_writestring("\nCommand not recognized: '");
            terminal_writestring(cmd);
            terminal_writestring("'\nType 'help' for available commands.\n");
        }
        terminal_writestring("\n");
    }
}