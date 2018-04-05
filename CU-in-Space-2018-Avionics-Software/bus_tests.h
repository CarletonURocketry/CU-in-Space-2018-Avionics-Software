//
//  bus_tests.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2018-03-27.
//

#ifndef bus_tests_h
#define bus_tests_h

#include "global.h"
#include <avr/pgmspace.h>


//SPI Test
extern const char menu_cmd_spitest_string[] PROGMEM;
extern const char menu_help_spitest[] PROGMEM;
extern void menu_cmd_spitest_handler(uint8_t arg_len, char** args);

// SPI Raw
extern const char menu_cmd_spiraw_string[] PROGMEM;
extern const char menu_help_spiraw[] PROGMEM;
extern void menu_cmd_spiraw_handler(uint8_t arg_len, char** args);

// SPI Conccurent
extern const char menu_cmd_spiconc_string[] PROGMEM;
extern const char menu_help_spiconc[] PROGMEM;
extern void menu_cmd_spiconc_handler(uint8_t arg_len, char** args);

// I2C Raw
extern const char menu_cmd_iicraw_string[] PROGMEM;
extern const char menu_help_iicraw[] PROGMEM;
extern void menu_cmd_iicraw_handler(uint8_t arg_len, char** args);

// I2C IO
extern const char menu_cmd_iicio_string[] PROGMEM;
extern const char menu_help_iicio[] PROGMEM;
extern void menu_cmd_iicio_handler(uint8_t arg_len, char** args);

#endif /* bus_tests_h */
