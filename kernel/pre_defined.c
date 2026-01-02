#include "bootscreen.h"
#include "keyboard.h"
#include "vga.h"

void load_bootscreen(int isOn)
{
    if (isOn) {
        bootscreen_show();
    
        bootscreen_add_item("Initializing VGA terminal", true);
        delay_ms(600);
        terminal_initialize();
        bootscreen_update_progress(10);
    
        bootscreen_add_item("Setting up keyboard driver", true);
        delay_ms(800);
        keyboard_init();
        bootscreen_update_progress(25);
    
        bootscreen_add_item("Checking system memory", true);
        delay_ms(900);
        bootscreen_update_progress(55);
    
        bootscreen_add_item("Detecting CPU features", true);
        delay_ms(500);
        bootscreen_update_progress(65);
    
        bootscreen_add_item("Configuring interrupt controller", true);
        delay_ms(600);
        bootscreen_update_progress(75);
    
        bootscreen_add_item("Starting system timer", true);
        delay_ms(400);
        bootscreen_update_progress(85);
    
        bootscreen_add_item("Running final system checks", true);
        delay_ms(800);
        bootscreen_update_progress(95);
    
        bootscreen_add_item("Booting T84 OS kernel", true);
        delay_ms(1000);
        bootscreen_update_progress(100);
    
        bootscreen_complete();
    }
}