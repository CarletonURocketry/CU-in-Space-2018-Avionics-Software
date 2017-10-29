//
//  menu_data.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "menu_data.h"

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "pindefinitions.h"
#include "menu_data.h"
#include "serial0.h"

// MARK: Strings
const char welcome_string[] PROGMEM = "CU In Space Avionics - 2018\tv1.0\n";
const char prompt_string[] PROGMEM = "> ";
const char menu_unkown_cmd_prt1[] PROGMEM = "Unkown command: \"";
const char menu_unkown_cmd_prt2[] PROGMEM = "\"\nUse \"help --list\" to get a list of avaliable commands.\n";

// MARK: Commands

// Help
static const char menu_cmd_help_string[] PROGMEM = "help";
static const char menu_help_help[] PROGMEM = "Get information about how to use commands.\nValid Usage: help <command name>\nUse help --list to list all valid commands.\n";

static const char help_string_unknown_one[] PROGMEM = "Unkown Command \"";
static const char help_string_unknown_two[] PROGMEM = "\"\n";

static const char help_list_all_string[] PROGMEM = "--list";

void menu_cmd_help_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 2) {
        serial_0_put_string_P(menu_help_help);
        return;
    }
    
    if (!strcasecmp_P(args[1], help_list_all_string)) {
        for (int i = 0; i < menu_num_items; i++) {
            serial_0_put_string_P((char*)pgm_read_word(&menu_items[i].string));
            serial_0_put_byte('\n');
        }
        return;
    }
    
    for (int i = 0; i < menu_num_items; i++) {
        if (!strcasecmp_P(args[1], (char*)pgm_read_word(&menu_items[i].string))) {
            serial_0_put_string_P((char*)pgm_read_word(&menu_items[i].help_string));
            return;
        }
    }
    
    serial_0_put_string_P(help_string_unknown_one);
    serial_0_put_string(args[1]);
    serial_0_put_string_P(help_string_unknown_two);
}

// Clear
static const char menu_cmd_clear_string[] PROGMEM = "clear";
static const char menu_help_clear[] PROGMEM = "Clear the terminal screen\n";

void menu_cmd_clear_handler(uint8_t arg_len, char** args)
{
    // Clear screen
    serial_0_put_byte(0x1B);
    serial_0_put_string("[2J");
    // Bring cursor home
    serial_0_put_byte(0x1B);
    serial_0_put_string("[H");
}

const uint8_t menu_num_items = 2;
const menu_item_t menu_items[] PROGMEM = {
    {.string = menu_cmd_help_string, .handler = menu_cmd_help_handler, .help_string = menu_help_help},
    {.string = menu_cmd_clear_string, .handler = menu_cmd_clear_handler, .help_string = menu_help_clear},
};
