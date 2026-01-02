#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_help(void);
void cmd_clear(int mode);
void cmd_about(void);
void cmd_draw(void);
void cmd_test(void);
void cmd_int(const char *args);
void cmd_string(const char *args);
void cmd_schar(const char *args);
void cmd_print_var(const char *args);
void cmd_if(const char *args);
void cmd_for(const char *args);
void cmd_sum(const char *args);
void cmd_sub(const char *args);
void cmd_div(const char *args);
void cmd_run_example(const char *example_name);
void cmd_list_vars(void);
void cmd_echo(const char *args);

#endif