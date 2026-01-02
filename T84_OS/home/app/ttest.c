#include "../../../kernel/vga.h"
#include "../../../kernel/keyboard.h"
#include "../../../kernel/string_utils.h"
#include "../../../kernel/panic.h"
#include "../../../kernel/examples/circle.h"
#include "../../../kernel/bootscreen.h"
#include "../../../kernel/variables.h"
#include "../../../kernel/pre_defined.h"
#include "../../../kernel/panic.h"
#include "../../api/kernel_api.h"
#include "../../../kernel/vga.h"
#include "../../../kernel/string_utils.h"

static void app_help()
{
    terminal_writestring("      TTEST for T84 OS\n");
    terminal_writestring("      Basic: \n");
    terminal_writestring("          \"kpanic <text> <code>\". Allow you to test the T84 OS Kernel Panic screen\n");
    terminal_writestring("          \"exit\". Quit the app\n");
    terminal_writestring("          \"echo <text>\". Allows you to write any text in the terminal\n");
}

static void app_exit()
{
    GLOBAL_exit_app();
}

static void app_kpanic(const char *args)
{
    if (!args || strlen(args) == 0)
    {
        terminal_writestring("\nUsage: kpanic <message> <code>\n");
        return;
    }

    char message[128];
    char code_str[32];

    char buffer[160];
    strcpy(buffer, args);

    char *last_space = NULL;
    for (int i = 0; buffer[i]; i++)
    {
        if (buffer[i] == ' ')
            last_space = &buffer[i];
    }

    if (!last_space)
    {
        terminal_writestring("\nError: missing panic code\n");
        return;
    }

    *last_space = '\0';
    strcpy(message, buffer);
    strcpy(code_str, last_space + 1);

    uint32_t code = parse_uint(code_str);
    panic_with_code(message, code);
}

void load_ttest_app()
{
    char input[256];

    while (1)
    {
        api_process_arrow_keys();
        api_update_cursor();
        terminal_setcolor(vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_LIGHT_GREY));
        terminal_writestring("apps/ttest> ");

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
            app_help();
        }
        else if (strcmp(cmd, "exit") == 0)
        {
            app_exit();
        }
        else if (strcmp(cmd, "kpanic") == 0)
        {
            app_kpanic(arg);
        }
        else if (strcmp(cmd, "echo") == 0) {
            cmd_echo(arg);
            terminal_writestring("\n");
        }
        else if (cmd[0] != '\0')
        {
            terminal_writestring("\nCommand not recognized: '");
            terminal_writestring(cmd);
            terminal_writestring("'\nType 'help' for available commands.\n");
        }
    }
}