//
//  menu_data.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#ifndef menu_data_h
#define menu_data_h

#include <avr/pgmspace.h>

typedef void (*menu_handler_t)(uint8_t, char**);
typedef struct menu_item {
    const PGM_P string;
    const menu_handler_t handler;
    const PGM_P help_string;
} menu_item_t;

// MARK: UI
extern const char welcome_string[] PROGMEM;
extern const char version_string[] PROGMEM;
extern const char prompt_string[] PROGMEM;
extern const char menu_unkown_cmd_prt1[] PROGMEM;
extern const char menu_unkown_cmd_prt2[] PROGMEM;

// MARK: Responses

// MARK: Commands
extern const uint8_t menu_num_items;
extern const menu_item_t menu_items[] PROGMEM;

#endif /* menu_data_h */
