#include "../../api/kernel_api.h"
#include "../../../kernel/stdint.h"
#include "../../../kernel/stddef.h"
#include "../../../kernel/keyboard.h"
#include "../../../kernel/vga.h"
#include "../../../kernel/kernel.h"
#include "../../../kernel/ramfs.h"
#include <stdbool.h>

#define IDE_WIDTH 70
#define IDE_HEIGHT 18
#define MAX_LINES 50
#define MAX_LINE_LEN 80

#define KEY_ESC 0x01
#define KEY_ENTER 0x1C
#define KEY_BACKSPACE 0x0E
#define KEY_LCTRL 0x1D
#define KEY_S 0x1F
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D

#define KEY_LSHIFT 0x2A
#define KEY_RSHIFT 0x36
#define KEY_CAPSLOCK 0x3A

#define KEY_TAB 0x0F

static char ide_clipboard[256] = "";
static bool ide_clipboard_has_content = false;

typedef struct
{
    char filename[32];
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
    int cursor_line;
    int cursor_col;
    bool modified;
    bool should_exit;
    bool ctrl_pressed;
    bool shift_pressed;
    bool caps_lock;
    bool insert_mode;
    int scroll_offset;
} IDE_Editor;

static IDE_Editor editor;

static int str_len(const char *s)
{
    int len = 0;
    while (s[len])
        len++;
    return len;
}

static void str_copy(char *dest, const char *src)
{
    int i = 0;
    while (src[i])
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void overwrite_char(char c)
{
    if (editor.cursor_line >= editor.line_count)
        return;

    char *line = editor.lines[editor.cursor_line];
    int len = str_len(line);

    if (editor.cursor_col < len)
    {

        line[editor.cursor_col] = c;
    }
    else
    {

        if (len < MAX_LINE_LEN - 1)
        {
            line[len] = c;
            line[len + 1] = '\0';
        }
    }

    editor.cursor_col++;
    editor.modified = true;
}

static uint8_t read_scancode(void)
{
    while (!keyboard_available())
        ;
    return keyboard_get_scancode();
}

static uint8_t get_key_scancode(void)
{
    while (!keyboard_available())
        ;

    uint8_t scancode;

    __asm__ volatile("inb $0x60, %0" : "=a"(scancode));
    return scancode;
}

static void clear_screen(void)
{
    uint16_t *vga = (uint16_t *)0xB8000;
    for (int i = 0; i < 80 * 25; i++)
    {
        vga[i] = (uint16_t)' ' | (uint16_t)0x07 << 8;
    }
}

static void write_at(int x, int y, const char *text, uint8_t color)
{
    uint16_t *vga = (uint16_t *)0xB8000;
    int i = 0;
    while (text[i] && (x + i) < 80)
    {
        vga[y * 80 + (x + i)] = (uint16_t)text[i] | (uint16_t)color << 8;
        i++;
    }
}

static void write_char_at(int x, int y, char c, uint8_t color)
{
    if (x >= 0 && x < 80 && y >= 0 && y < 25)
    {
        uint16_t *vga = (uint16_t *)0xB8000;
        vga[y * 80 + x] = (uint16_t)c | (uint16_t)color << 8;
    }
}

static void clear_memory(void *ptr, int size)
{
    char *p = (char *)ptr;
    for (int i = 0; i < size; i++)
    {
        p[i] = 0;
    }
}

void ide_open_file(const char *filename)
{

    str_copy(editor.filename, filename);
    editor.line_count = 1;
    editor.cursor_line = 0;
    editor.cursor_col = 0;
    editor.modified = false;
    editor.should_exit = false;
    editor.ctrl_pressed = false;
    editor.shift_pressed = false;
    editor.caps_lock = false;
    editor.scroll_offset = 0;

    for (int i = 0; i < MAX_LINES; i++)
    {
        for (int j = 0; j < MAX_LINE_LEN; j++)
        {
            editor.lines[i][j] = '\0';
        }
    }

    File *file = fs_find_file(filename);

    if (file && file->type == 'F')
    {
        char *content = file->content;
        int size = file->size;
        int line = 0;
        int col = 0;
        int pos = 0;

        int last_valid_line = 0;
        int last_valid_col = 0;

        while (pos < size && line < MAX_LINES)
        {
            char c = content[pos];

            if (c == '\n')
            {

                if (col < MAX_LINE_LEN)
                {
                    editor.lines[line][col] = '\0';
                }

                if (col > 0)
                {
                    last_valid_line = line;
                    last_valid_col = col;
                }

                line++;
                col = 0;
                pos++;
            }
            else if (c == '\r')
            {

                pos++;
            }
            else if (c == '\0')
            {

                break;
            }
            else
            {

                if (col < MAX_LINE_LEN - 1)
                {
                    editor.lines[line][col] = c;
                    col++;
                    last_valid_line = line;
                    last_valid_col = col;
                }
                pos++;
            }
        }

        if (col > 0 && line < MAX_LINES)
        {
            editor.lines[line][col] = '\0';
            last_valid_line = line;
            last_valid_col = col;
            line++;
        }

        if (line > 0)
        {
            editor.line_count = line;
        }

        editor.cursor_line = last_valid_line;
        editor.cursor_col = last_valid_col;

        if (editor.line_count == 1 && editor.lines[0][0] == '\0')
        {
            editor.cursor_line = 0;
            editor.cursor_col = 0;
        }
    }
}

void draw_editor(void)
{
    clear_screen();

    for (int x = 2; x < 78; x++)
    {
        write_char_at(x, 1, '-', 0x1F);
        write_char_at(x, IDE_HEIGHT + 2, '-', 0x1F);
    }

    for (int y = 2; y <= IDE_HEIGHT + 1; y++)
    {
        write_char_at(2, y, '|', 0x1F);
        write_char_at(77, y, '|', 0x1F);
    }

    write_at(4, 1, "T84 IDE - ", 0x1F);
    write_at(14, 1, editor.filename, 0x1F);

    
    for (int i = 0; i < IDE_HEIGHT; i++)
    {
        int line_y = i + 2;
        int file_line = i + editor.scroll_offset;

        char line_num[6];
        if (file_line + 1 < 10)
        {
            line_num[0] = ' ';
            line_num[1] = '0' + (file_line + 1);
            line_num[2] = ' ';
            line_num[3] = ' ';
            line_num[4] = '\0';
        }
        else
        {
            line_num[0] = '0' + ((file_line + 1) / 10);
            line_num[1] = '0' + ((file_line + 1) % 10);
            line_num[2] = ' ';
            line_num[3] = ' ';
            line_num[4] = '\0';
        }
        write_at(4, line_y, line_num, 0x0B);

        if (file_line < editor.line_count)
        {
            write_at(8, line_y, editor.lines[file_line], 0x07);

            
            if (file_line == editor.cursor_line)
            {
                write_char_at(8 + editor.cursor_col, line_y, '_', 0x0F);
            }
        }
    }

    write_at(4, IDE_HEIGHT + 3, "ESC:Exit  ENTER:NewLine  CTRL+S:Save  BACKSPACE:Delete", 0x1F);

    char status[60]; 
    str_copy(status, "Line: ");

    char line_num[4];
    if (editor.cursor_line + 1 < 10)
    {
        line_num[0] = '0' + (editor.cursor_line + 1);
        line_num[1] = '\0';
    }
    else
    {
        line_num[0] = '0' + ((editor.cursor_line + 1) / 10);
        line_num[1] = '0' + ((editor.cursor_line + 1) % 10);
        line_num[2] = '\0';
    }
    str_copy(status + str_len(status), line_num);

    str_copy(status + str_len(status), " Col: ");

    char col_num[4];
    if (editor.cursor_col + 1 < 10)
    {
        col_num[0] = '0' + (editor.cursor_col + 1);
        col_num[1] = '\0';
    }
    else
    {
        col_num[0] = '0' + ((editor.cursor_col + 1) / 10);
        col_num[1] = '0' + ((editor.cursor_col + 1) % 10);
        col_num[2] = '\0';
    }
    str_copy(status + str_len(status), col_num);

    
    str_copy(status + str_len(status), " [");

    char scroll_info[10];
    if (editor.line_count <= IDE_HEIGHT)
    {
        str_copy(scroll_info, "ALL");
    }
    else
    {
        char scroll_start[4], scroll_end[4];

        
        if (editor.scroll_offset + 1 < 10)
        {
            scroll_start[0] = '0' + (editor.scroll_offset + 1);
            scroll_start[1] = '\0';
        }
        else
        {
            scroll_start[0] = '0' + ((editor.scroll_offset + 1) / 10);
            scroll_start[1] = '0' + ((editor.scroll_offset + 1) % 10);
            scroll_start[2] = '\0';
        }

        
        int last_visible = editor.scroll_offset + IDE_HEIGHT;
        if (last_visible > editor.line_count)
            last_visible = editor.line_count;

        if (last_visible < 10)
        {
            scroll_end[0] = '0' + last_visible;
            scroll_end[1] = '\0';
        }
        else
        {
            scroll_end[0] = '0' + (last_visible / 10);
            scroll_end[1] = '0' + (last_visible % 10);
            scroll_end[2] = '\0';
        }

        str_copy(scroll_info, scroll_start);
        str_copy(scroll_info + str_len(scroll_info), "-");
        str_copy(scroll_info + str_len(scroll_info), scroll_end);
    }

    str_copy(status + str_len(status), scroll_info);
    str_copy(status + str_len(status), "/");

    char total_lines[6];
    if (editor.line_count < 10)
    {
        total_lines[0] = '0' + editor.line_count;
        total_lines[1] = '\0';
    }
    else if (editor.line_count < 100)
    {
        total_lines[0] = '0' + (editor.line_count / 10);
        total_lines[1] = '0' + (editor.line_count % 10);
        total_lines[2] = '\0';
    }
    else
    {
        str_copy(total_lines, "99+");
    }
    str_copy(status + str_len(status), total_lines);
    str_copy(status + str_len(status), "] ");

    str_copy(status + str_len(status), editor.modified ? "[MODIFIED]" : "[SAVED]");

    str_copy(status + str_len(status), " ");
    if (editor.shift_pressed)
        str_copy(status + str_len(status), "[SHIFT]");
    if (editor.caps_lock)
        str_copy(status + str_len(status), "[CAPS]");
    if (editor.insert_mode)
        str_copy(status + str_len(status), "[INS]");
    else
        str_copy(status + str_len(status), "[OVR]");

    write_at(4, IDE_HEIGHT + 4, status, 0x1F);
}

void insert_char(char c)
{
    if (editor.cursor_line >= MAX_LINES)
        return;

    char *line = editor.lines[editor.cursor_line];
    int len = str_len(line);

    if (len >= MAX_LINE_LEN - 1)
        return;

    for (int i = len; i >= editor.cursor_col; i--)
    {
        line[i + 1] = line[i];
    }

    line[editor.cursor_col] = c;
    editor.cursor_col++;
    editor.modified = true;
}

void delete_char(void)
{
    if (editor.cursor_line >= editor.line_count)
        return;

    char *line = editor.lines[editor.cursor_line];
    int len = str_len(line);

    if (editor.cursor_col > 0)
    {

        for (int i = editor.cursor_col - 1; i < len; i++)
        {
            line[i] = line[i + 1];
        }
        editor.cursor_col--;
        editor.modified = true;
    }
    else if (editor.cursor_line > 0)
    {

        char *prev_line = editor.lines[editor.cursor_line - 1];
        int prev_len = str_len(prev_line);

        if (prev_len + len < MAX_LINE_LEN - 1)
        {

            for (int i = 0; i < len; i++)
            {
                prev_line[prev_len + i] = line[i];
            }
            prev_line[prev_len + len] = '\0';

            for (int i = editor.cursor_line; i < editor.line_count - 1; i++)
            {
                str_copy(editor.lines[i], editor.lines[i + 1]);
            }
            editor.line_count--;

            editor.cursor_line--;
            editor.cursor_col = prev_len;
            editor.modified = true;
        }
    }
}

void new_line(void)
{
    if (editor.line_count >= MAX_LINES - 1)
        return;

    char *current_line = editor.lines[editor.cursor_line];
    int len = str_len(current_line);

    if (editor.cursor_col < len)
    {
        for (int i = editor.line_count; i > editor.cursor_line + 1; i--)
        {
            str_copy(editor.lines[i], editor.lines[i - 1]);
        }

        char *new_line = editor.lines[editor.cursor_line + 1];
        int j = 0;
        for (int i = editor.cursor_col; i < len; i++)
        {
            new_line[j++] = current_line[i];
        }
        new_line[j] = '\0';

        current_line[editor.cursor_col] = '\0';

        editor.line_count++;
    }
    else
    {
        if (editor.line_count < MAX_LINES - 1)
        {
            for (int i = editor.line_count; i > editor.cursor_line + 1; i--)
            {
                str_copy(editor.lines[i], editor.lines[i - 1]);
            }
            editor.lines[editor.cursor_line + 1][0] = '\0';
            editor.line_count++;
        }
    }

    editor.cursor_line++;
    editor.cursor_col = 0;
    editor.modified = true;
    
    
    if (editor.cursor_line >= editor.scroll_offset + IDE_HEIGHT) {
        editor.scroll_offset = editor.cursor_line - IDE_HEIGHT + 1;
    }
    
    draw_editor();
}

void save_file(void)
{

    char content[2048];
    int pos = 0;

    for (int i = 0; i < editor.line_count && pos < 2047; i++)
    {
        int len = str_len(editor.lines[i]);

        for (int j = 0; j < len && pos < 2047; j++)
        {
            content[pos++] = editor.lines[i][j];
        }

        if (i < editor.line_count - 1 && pos < 2047)
        {
            content[pos++] = '\n';
        }
    }

    content[pos] = '\0';

    fs_write(editor.filename, content);

    editor.modified = false;

    write_at(4, IDE_HEIGHT + 5, "File saved successfully!", 0x0A);

    for (volatile int i = 0; i < 500000; i++)
        ;

    write_at(4, IDE_HEIGHT + 5, "                       ", 0x07);
}

static char scancode_to_char(uint8_t scancode, bool shift_pressed, bool caps_lock)
{
    static const char lowercase_map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0,
        ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    static const char uppercase_map[] = {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
        0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0,
        ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (scancode >= sizeof(lowercase_map))
        return 0;

    char base_char;
    bool should_uppercase = shift_pressed ^ caps_lock;

    if (should_uppercase && scancode < sizeof(uppercase_map))
    {
        base_char = uppercase_map[scancode];
    }
    else
    {
        base_char = lowercase_map[scancode];
    }

    return base_char;
}

void handle_scancode(uint8_t scancode)
{
    static bool got_e0 = false;

    if (scancode == 0xE0)
    {
        got_e0 = true;
        return;
    }

    if (got_e0)
    {
        got_e0 = false;

        if (!(scancode & 0x80))
        {
            int old_line = editor.cursor_line;
            int old_col = editor.cursor_col;

            switch (scancode)
            {
            case KEY_UP:
                if (editor.cursor_line > 0)
                {
                    editor.cursor_line--;

                    
                    if (editor.cursor_line < editor.scroll_offset)
                    {
                        editor.scroll_offset = editor.cursor_line;
                    }

                    int line_len = str_len(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col > line_len)
                    {
                        editor.cursor_col = line_len;
                    }
                }
                break;

            case KEY_DOWN:
                if (editor.cursor_line < editor.line_count - 1)
                {
                    editor.cursor_line++;

                    
                    if (editor.cursor_line >= editor.scroll_offset + IDE_HEIGHT)
                    {
                        editor.scroll_offset = editor.cursor_line - IDE_HEIGHT + 1;
                    }

                    int line_len = str_len(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col > line_len)
                    {
                        editor.cursor_col = line_len;
                    }
                }
                break;

            case KEY_LEFT:
                if (editor.cursor_col > 0)
                {
                    editor.cursor_col--;
                }
                else if (editor.cursor_line > 0)
                {
                    editor.cursor_line--;
                    editor.cursor_col = str_len(editor.lines[editor.cursor_line]);

                    
                    if (editor.cursor_line < editor.scroll_offset)
                    {
                        editor.scroll_offset = editor.cursor_line;
                    }
                }
                break;

            case KEY_RIGHT:
                if (editor.cursor_col < str_len(editor.lines[editor.cursor_line]))
                {
                    editor.cursor_col++;
                }
                else if (editor.cursor_line < editor.line_count - 1)
                {
                    editor.cursor_line++;
                    editor.cursor_col = 0;

                    
                    if (editor.cursor_line >= editor.scroll_offset + IDE_HEIGHT)
                    {
                        editor.scroll_offset = editor.cursor_line - IDE_HEIGHT + 1;
                    }
                }
                break;
            }

            if (editor.cursor_line != old_line || editor.cursor_col != old_col)
            {
                draw_editor();
            }
        }
        return;
    }

    if (scancode & 0x80)
    {
        uint8_t key = scancode & 0x7F;

        if (key == KEY_LCTRL)
        {
            editor.ctrl_pressed = false;
        }
        else if (key == KEY_LSHIFT || key == KEY_RSHIFT)
        {
            editor.shift_pressed = false;
        }
        return;
    }

    switch (scancode)
    {
    case KEY_ESC:
        if (editor.modified)
        {
            write_at(4, IDE_HEIGHT + 5, "Save changes? (Y/N): ", 0x0C);
            draw_editor();

            while (1)
            {
                uint8_t resp = get_key_scancode();
                if (!(resp & 0x80))
                {
                    if (resp == 0x15)
                    {
                        save_file();
                        editor.should_exit = true;
                        break;
                    }
                    else if (resp == 0x31)
                    {
                        editor.should_exit = true;
                        break;
                    }
                }
            }
        }
        else
        {
            editor.should_exit = true;
        }
        cmd_clear(1);
        break;

    case KEY_ENTER:
        new_line();
        draw_editor();
        break;

    case KEY_TAB:
        for (int i = 0; i < 4; i++)
        {
            insert_char(' ');
        }
        draw_editor();
        break;

    case KEY_BACKSPACE:
        delete_char();
        draw_editor();
        break;

    case KEY_LCTRL:
        editor.ctrl_pressed = true;
        break;

    case 0x2E:
        if (editor.ctrl_pressed)
        {

            char *current_line = editor.lines[editor.cursor_line];
            int len = str_len(current_line);

            if (len > 0)
            {
                int copy_len = (len < 255) ? len : 255;
                for (int i = 0; i < copy_len; i++)
                {
                    ide_clipboard[i] = current_line[i];
                }
                ide_clipboard[copy_len] = '\0';
                ide_clipboard_has_content = true;

                write_at(4, IDE_HEIGHT + 5, "Line copied to clipboard!", 0x0A);
                for (volatile int i = 0; i < 200000; i++)
                    ;
                write_at(4, IDE_HEIGHT + 5, "                         ", 0x07);
            }
        }
        else
        {

            char c = scancode_to_char(scancode, editor.shift_pressed, editor.caps_lock);
            if (c)
            {
                insert_char(c);
                draw_editor();
            }
        }
        break;

    case 0x2F:
        if (editor.ctrl_pressed)
        {

            if (ide_clipboard_has_content)
            {
                for (int i = 0; ide_clipboard[i]; i++)
                {
                    insert_char(ide_clipboard[i]);
                }

                write_at(4, IDE_HEIGHT + 5, "Pasted from clipboard!", 0x0A);
                for (volatile int i = 0; i < 200000; i++)
                    ;
                write_at(4, IDE_HEIGHT + 5, "                       ", 0x07);

                draw_editor();
            }
        }
        else
        {

            char c = scancode_to_char(scancode, editor.shift_pressed, editor.caps_lock);
            if (c)
            {
                insert_char(c);
                draw_editor();
            }
        }
        break;

    case KEY_LSHIFT:
    case KEY_RSHIFT:
        editor.shift_pressed = true;
        break;

    case KEY_CAPSLOCK:
        editor.caps_lock = !editor.caps_lock;
        break;

    case KEY_S:
        if (editor.ctrl_pressed)
        {
            save_file();
            draw_editor();
        }
        else
        {
            char c = scancode_to_char(scancode, editor.shift_pressed, editor.caps_lock);
            if (c)
            {
                insert_char(c);
                draw_editor();
            }
        }
        break;

    default:

        char c = scancode_to_char(scancode, editor.shift_pressed, editor.caps_lock);
        if (c)
        {
            insert_char(c);
            draw_editor();
        }
        break;
    }
}

void ide_run(void)
{
    draw_editor();

    while (!editor.should_exit)
    {
        if (keyboard_available())
        {
            uint8_t scancode = get_key_scancode();
            handle_scancode(scancode);
        }

        for (volatile int i = 0; i < 1000; i++)
            ;
    }

    clear_screen();
    write_at(30, 12, "Returning to T84 OS...", 0x0F);

    for (volatile int i = 0; i < 2000000; i++)
        ;

    clear_screen();

    for (int y = 0; y < 25; y++)
    {
        for (int x = 0; x < 80; x++)
        {
            uint16_t *vga = (uint16_t *)0xB8000;
            vga[y * 80 + x] = (uint16_t)' ' | (uint16_t)0x70 << 8;
        }
    }

    write_at(35, 1, "T84 OS", 0x1F);
    write_at(32, 3, "T84 Terminal v1.0", 0x17);
    cmd_clear(1);
}

void cmd_open_ide(const char *args)
{
    if (!args || args[0] == '\0')
    {
        terminal_writestring("Usage: open 4ide <filename>\n");
        return;
    }

    const char *prefix = "4ide ";
    for (int i = 0; i < 5; i++)
    {
        if (args[i] != prefix[i])
        {
            terminal_writestring("Usage: open 4ide <filename>\n");
            return;
        }
    }

    const char *filename = args + 5;
    if (filename[0] == '\0')
    {
        terminal_writestring("Error: Missing filename\n");
        return;
    }

    terminal_writestring("\nOpening T84 IDE...\n");

    for (volatile int i = 0; i < 1000000; i++)
        ;

    ide_open_file(filename);
    ide_run();
}

void main_app(void)
{
    terminal_writestring("\nT84 Integrated Development Environment\n");
    terminal_writestring("Usage: open 4ide <filename>\n");
}