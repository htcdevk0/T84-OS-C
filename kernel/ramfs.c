#include "ramfs.h"
#include "vga.h"
#include "string_utils.h"
#include "commands.h"

static File files[100];
static int file_count = 0;
static File *current_dir = NULL;
static File *root = NULL;

static File *create_file(const char *name, char type)
{
    if (file_count >= 100)
        return NULL;

    File *f = &files[file_count++];

    int i = 0;
    while (name[i] && i < 31)
    {
        f->name[i] = name[i];
        i++;
    }
    f->name[i] = '\0';

    f->type = type;
    f->size = 0;
    f->content[0] = '\0';
    f->parent = NULL;
    f->child = NULL;
    f->next = NULL;

    return f;
}

void fs_init(void)
{
    file_count = 0;

    root = create_file("", 'D');
    current_dir = root;

    File *home = create_file("home", 'D');
    home->parent = root;
    root->child = home;

    File *user = create_file("user", 'D');
    user->parent = home;
    home->child = user;

    File *hello = create_file("Hello.T", 'F');
    if (hello)
    {

        const char *hello_content =
            "schar name = \"\"\n"
            "write(\"Type your name: \")\n"
            "input(name)\n"
            "line()\n"
            "write(\"Hello! \", name)\n";

        int i = 0;
        while (hello_content[i] && i < 1023)
        {
            hello->content[i] = hello_content[i];
            i++;
        }
        hello->content[i] = '\0';
        hello->size = i;

        hello->parent = user;
        hello->next = user->child;
        user->child = hello;
    }

    File *calc = create_file("Calculator.T", 'F');
    if (calc)
    {
        const char *calc_content = 
            "# T84 Calculator - Interactive Calculator\n"
            "# Usage: tlang Calculator.T\n"
            "\n"
            "int n1 = 0\n"
            "int n2 = 0\n"
            "schar op = \"\"\n"
            "line()\n"
            "write(\"== Welcome to the T84 Calculator ==\")\n"
            "line()\n"
            "write(\"Type the first number: \")\n"
            "input(n1)\n"
            "write(\"Type the second number: \")\n"
            "input(n2)\n"
            "write(\"Type the operation (+, -, *, /): \")\n"
            "input(op)\n"
            "if op == \"+\":\n"
            "    line()\n"
            "    write(\"Result: \", n1 + n2)\n"
            "elif op == \"-\":\n"
            "    line()\n"
            "    write(\"Result: \", n1 - n2)\n"
            "elif op == \"*\":\n"
            "    line()\n"
            "    write(\"Result: \", n1 * n2)\n"
            "elif op == \"/\":\n"
            "    line()\n"
            "    write(\"Result: \", n1 / n2)\n"
            "line()\n"
            "write(\"Thank you for using T84 Calculator!\")\n"
            "line()\n";

        int i = 0;
        while (calc_content[i] && i < 1023)
        {
            calc->content[i] = calc_content[i];
            i++;
        }
        calc->content[i] = '\0';
        calc->size = i;

        calc->parent = user;
        calc->next = user->child;
        user->child = calc;
    }

    current_dir = user;
}

void fs_mkdir(const char *name)
{
    File *new_dir = create_file(name, 'D');
    if (!new_dir)
    {
        terminal_writestring("Error\n");
        return;
    }

    new_dir->parent = current_dir;

    if (!current_dir->child)
    {
        current_dir->child = new_dir;
    }
    else
    {
        File *last = current_dir->child;
        while (last->next)
        {
            last = last->next;
        }
        last->next = new_dir;
    }

    terminal_writestring("Directory created\n");
}

void fs_mkfile(const char *name)
{
    File *new_file = create_file(name, 'F');
    if (!new_file)
    {
        terminal_writestring("Error\n");
        return;
    }

    new_file->parent = current_dir;

    if (!current_dir->child)
    {
        current_dir->child = new_file;
    }
    else
    {
        File *last = current_dir->child;
        while (last->next)
        {
            last = last->next;
        }
        last->next = new_file;
    }

    terminal_writestring("File created\n");
}

void fs_cd(const char *path)
{
    if (!path || !*path)
    {

        File *home = root->child;
        if (home && home->child)
        {
            current_dir = home->child;
        }
        return;
    }

    if (strcmp(path, "..") == 0)
    {
        if (current_dir->parent)
        {
            current_dir = current_dir->parent;
        }
        return;
    }

    if (strcmp(path, "/") == 0)
    {
        current_dir = root;
        return;
    }

    if (strcmp(path, "home") == 0)
    {
        if (root->child)
        {
            current_dir = root->child;
        }
        return;
    }

    File *child = current_dir->child;
    while (child)
    {
        if (child->type == 'D' && strcmp(child->name, path) == 0)
        {
            current_dir = child;
            return;
        }
        child = child->next;
    }

    terminal_writestring("Directory not found\n");
}

void fs_ls(void)
{
    File *child = current_dir->child;

    if (!child)
    {
        terminal_writestring("(empty)\n");
        return;
    }

    while (child)
    {
        terminal_writestring(child->type == 'D' ? "[D] " : "[F] ");
        terminal_writestring(child->name);
        terminal_writestring("\n");
        child = child->next;
    }
}

void fs_write(const char *filename, const char *content)
{

    File *child = current_dir->child;
    while (child)
    {
        if (child->type == 'F' && strcmp(child->name, filename) == 0)
        {

            int i = 0;
            while (content[i] && i < 1023)
            {
                child->content[i] = content[i];
                i++;
            }
            child->content[i] = '\0';
            child->size = i;

            terminal_writestring("Written\n");
            return;
        }
        child = child->next;
    }

    terminal_writestring("File not found\n");
}

void fs_cat(const char *filename)
{
    File *child = current_dir->child;
    while (child)
    {
        if (child->type == 'F' && strcmp(child->name, filename) == 0)
        {
            if (child->size > 0)
            {
                terminal_writestring(child->content);
                if (child->content[child->size - 1] != '\n')
                {
                    terminal_writestring("\n");
                }
            }
            return;
        }
        child = child->next;
    }

    terminal_writestring("File not found\n");
}

void fs_pwd(void)
{
    if (current_dir == root)
    {
        terminal_writestring("/\n");
        return;
    }

    char *names[10];
    int depth = 0;
    File *cur = current_dir;

    while (cur && cur != root)
    {
        names[depth++] = cur->name;
        cur = cur->parent;
    }

    terminal_writestring("/");
    for (int i = depth - 1; i >= 0; i--)
    {
        terminal_writestring(names[i]);
        if (i > 0)
            terminal_writestring("/");
    }
    terminal_writestring("\n");
}

void cmd_dir(const char *args)
{
    if (!args || !*args)
    {
        terminal_writestring("dir <name>\n");
        return;
    }
    fs_mkdir(args);
}

void cmd_fs(const char *args)
{
    if (!args || !*args)
    {
        terminal_writestring("fs <name>\n");
        return;
    }
    fs_mkfile(args);
}

void cmd_cd(const char *args)
{
    fs_cd(args);
}

void cmd_write(const char *args)
{
    if (!args || !*args)
    {
        terminal_writestring("write <file> <content>\n");
        return;
    }

    int space = -1;
    for (int i = 0; args[i]; i++)
    {
        if (args[i] == ' ')
        {
            space = i;
            break;
        }
    }

    if (space == -1)
    {
        terminal_writestring("Need content\n");
        return;
    }

    char filename[32];
    for (int i = 0; i < space && i < 31; i++)
    {
        filename[i] = args[i];
    }
    filename[space] = '\0';

    const char *content = &args[space + 1];

    fs_write(filename, content);
}

void cmd_ls(const char *args)
{
    fs_ls();
}

void cmd_cat(const char *args)
{
    if (!args || !*args)
    {
        terminal_writestring("cat <file>\n");
        return;
    }
    fs_cat(args);
}

void cmd_pwd(void)
{
    fs_pwd();
}

static void process_t84_line(const char *line)
{
    char cmd[256];
    char *arg = NULL;

    int i;
    for (i = 0; line[i] && i < 255; i++)
    {
        cmd[i] = line[i];
    }
    cmd[i] = '\0';

    for (i = 0; cmd[i]; i++)
    {
        if (cmd[i] == ' ')
        {
            cmd[i] = '\0';
            arg = &cmd[i + 1];
            break;
        }
    }

    char upper_cmd[256];
    for (i = 0; cmd[i]; i++)
    {
        upper_cmd[i] = (cmd[i] >= 'a' && cmd[i] <= 'z') ? cmd[i] - 32 : cmd[i];
    }
    upper_cmd[i] = '\0';

    if (strcmp(cmd, "ECHO") == 0 || strcmp(cmd, "echo") == 0)
    {
        cmd_echo(arg);
    }
    else if (strcmp(cmd, "INT") == 0 || strcmp(cmd, "int") == 0)
    {
        cmd_int(arg);
    }
    else if (strcmp(cmd, "STRING") == 0 || strcmp(cmd, "string") == 0)
    {
        cmd_string(arg);
    }
    else if (strcmp(cmd, "SCHAR") == 0 || strcmp(cmd, "schar") == 0)
    {
        cmd_schar(arg);
    }
    else if (strcmp(cmd, "FOR") == 0 || strcmp(cmd, "for") == 0)
    {
        cmd_for(arg);
    }
    else if (strcmp(cmd, "IF") == 0 || strcmp(cmd, "if") == 0)
    {
        cmd_if(arg);
    }
    else if (cmd[0] == '&')
    {
        cmd_print_var(cmd);
    }
    else if (strcmp(cmd, "CLEAR") == 0 || strcmp(cmd, "clear") == 0 ||
             strcmp(cmd, "CLS") == 0 || strcmp(cmd, "cls") == 0)
    {
        cmd_clear(1);
    }
    else if (strcmp(cmd, "HELP") == 0 || strcmp(cmd, "help") == 0)
    {
        cmd_help();
    }
    else if (strcmp(cmd, "SUM") == 0 || strcmp(cmd, "sum") == 0)
    {
        cmd_sum(arg);
    }
    else if (strcmp(cmd, "SUBTRACT") == 0 || strcmp(cmd, "subtract") == 0)
    {
        cmd_sub(arg);
    }
    else if (strcmp(cmd, "DIVIDE") == 0 || strcmp(cmd, "divide") == 0)
    {
        cmd_div(arg);
    }
}

void cmd_tparse(const char *args)
{
    if (!args || !*args)
    {
        terminal_writestring("tparse <file.T>\n");
        return;
    }

    int len = strlen(args);
    if (len < 3 || !(args[len - 2] == '.' && args[len - 1] == 'T'))
    {
        terminal_writestring("Not a .T file\n");
        return;
    }

    File *child = current_dir->child;
    while (child)
    {
        if (child->type == 'F' && strcmp(child->name, args) == 0)
        {
            terminal_writestring("\n=== Executing ");
            terminal_writestring(args);
            terminal_writestring(" ===\n\n");

            char *content = child->content;
            int pos = 0;
            int line_num = 1;

            while (pos < child->size)
            {

                char line[256];
                int line_len = 0;

                while (pos < child->size && content[pos] != '\n' && line_len < 255)
                {
                    line[line_len++] = content[pos++];
                }
                line[line_len] = '\0';

                if (content[pos] == '\n')
                    pos++;

                if (line_len > 0)
                {

                    terminal_writestring("[");
                    char num_str[8];
                    itoa(line_num, num_str, 10);
                    terminal_writestring(num_str);
                    terminal_writestring("] ");

                    process_t84_line(line);

                    line_num++;
                }
            }

            terminal_writestring("\n=== Done ===\n");
            return;
        }
        child = child->next;
    }

    terminal_writestring("File not found\n");
}

File *fs_find_file(const char *filename)
{
    File *child = current_dir->child;
    while (child)
    {
        if (child->type == 'F' && strcmp(child->name, filename) == 0)
        {
            return child;
        }
        child = child->next;
    }
    return NULL;
}