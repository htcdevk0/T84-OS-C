#pragma once
void kernel_main(void);

void cmd_clear(int mode);
void cmd_print_var(const char *args);
void cmd_if(const char *args);
void cmd_for(const char *args);
void cmd_string(const char *args);
void cmd_int(const char *args);