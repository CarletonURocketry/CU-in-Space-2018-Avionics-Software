//
//  menu.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#ifndef menu_h
#define menu_h

#include <avr/pgmspace.h>

#include "global.h"

typedef void (*menu_handler_t)(uint8_t, char**);
typedef struct menu_item {
    const PGM_P string;
    const menu_handler_t handler;
    const PGM_P help_string;
} menu_item_t;

/**
 * Initilize the menu system
 */
extern void init_menu(void);

/**
 * Process serial input in each itteration of the main loop
 */
extern void menu_service(void);

#endif /* menu_h */
