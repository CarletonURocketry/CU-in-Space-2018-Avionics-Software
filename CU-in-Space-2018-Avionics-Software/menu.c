//
//  menu.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "menu.h"

#include <avr/interrupt.h>
#include <string.h>

#include "pindefinitions.h"
#include "serial0.h"

#include "menu_data.h"

// MARK: Constants
#define MENU_BUFFER_SIZE 200

// MARK: Vairables
static char menu_buffer[MENU_BUFFER_SIZE];

// MARK: Static Function prototypes
static inline void print_prompt(void);

// MARK: Function Definitions
void init_menu(void)
{
    cli();
    init_serial_0();
    flags |= (1<<FLAG_SERIAL_0_LOOPBACK);       // Enable serial loopback
    sei();
    
//    // Clear screen
//    serial_0_put_byte(0x1B);
//    serial_0_put_string("[2J");
//    // Bring cursor home
//    serial_0_put_byte(0x1B);
//    serial_0_put_string("[H");
    
    serial_0_put_string_P(welcome_string);
    serial_0_put_string_P(version_string);
    print_prompt();
}

void menu_service(void)
{
    serial_0_service();
    if (serial_0_has_line('\n')) {
        serial_0_get_line('\n', menu_buffer, MENU_BUFFER_SIZE);
        char *line = menu_buffer;
        
        int num_tokens = 1;
        for (int i = 0; i < strlen(line); i++) {
            num_tokens += menu_buffer[i] == ' ';
        }
        
        char *args[num_tokens];
        for (int i = 0; (args[i] = strsep(&line, " ")) != NULL; i++);
        
        for (int i = 0; i < menu_num_items; i++) {
            if (!strcasecmp_P(args[0], (char*)pgm_read_word(&menu_items[i].string))) {
                (*((menu_handler_t)pgm_read_word(&menu_items[i].handler)))(num_tokens, args);
                print_prompt();
                return;
            }
        }
        
        if (menu_buffer[0] != '\0') {
            serial_0_put_string_P(menu_unkown_cmd_prt1);
            serial_0_put_string(args[0]);
            serial_0_put_string_P(menu_unkown_cmd_prt2);
        }
        print_prompt();
    }
}

static inline void print_prompt(void)
{
    serial_0_put_string_P(prompt_string);
}
