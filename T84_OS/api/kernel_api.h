#pragma once
#include "../../kernel/kernel.h"

void api_update_cursor(void);
void api_process_arrow_keys(void);
void api_move_cursor(int dx, int dy);

void GLOBAL_exit_app() {
    kernel_main();
}

void cmd_echo(const char *args);