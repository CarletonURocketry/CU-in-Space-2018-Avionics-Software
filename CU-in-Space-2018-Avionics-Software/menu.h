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

/**
 * Initilize the menu system
 */
extern void init_menu(void);

/**
 * Process serial input in each iteration of the main loop
 */
extern void menu_service(void);

#endif /* menu_h */
