//
//  serial0.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//
//  Buffered serial interface 0
//

#ifndef serial0_h
#define serial0_h

#include "global.h"
#include <avr/pgmspace.h>

/**
 *  Initilize the UART for serial I/O. 9.6k baud.
 */
extern void init_serial_0 (void);

/**
 *  Writes a string to the serial output
 *  @note This function should not be called from within an interupt
 *  @param str A null terminated string to be written via serial
 */
extern void serial_0_put_string (char *str);

/**
 *  Writes a string to the serial output from program memory
 *  @note This function should not be called from within an interupt
 *  @param str A pointer to a programs space pointer to where the string is stored
 */
extern void serial_0_put_string_P (const char *str);

/**
 *  Writes a string to the serial output from EEPROM
 *  @note This function should not be called from within an interupt
 *  @param addr The addres of the string in EEPROM
 *  @param length The length of the string
 */
extern void serial_0_put_from_eeprom (uint16_t addr);

/**
 *  Write a character to the serial output
 *  @param c The character to be written
 */
extern void serial_0_put_byte (char c);

/**
 *  Read a bytes from the serial input as a string
 *  @note This function should not be called from within an interupt
 *  @param str The string in which the data should be stored
 *  @param len The maximum number of chars to be read from the serial input
 */
extern void serial_0_get_string (char *str, int len);

/**
 *  Determine if there is a full line avaliable to be read from the serial input
 *  @note This function should not be called from within an interupt
 *  @return 0 if there is no line avaliable, 1 if a line is avaliable
 */
extern int serial_0_has_line (void);

/**
 *  Read a bytes from the serial input as a string up to the next newline character
 *  @note This function should not be called from within an interupt
 *  @param str The string in which the data should be stored
 *  @param len The maximum number of chars to be read from the serial input
 */
extern void serial_0_get_line (char *str, int len);


/**
 *  Get a character from the serial input without consuming it
 *  @return The least recently recieved character in the serial buffer
 */
extern char serial_0_peak_byte (void);

/**
 *  Get a character from the serial input
 *  @return The least recently recieved character in the serial buffer
 */
extern char serial_0_get_byte (void);

/**
 *  Service to be run in each iteration of the main loop
 */
extern void serial_0_service (void);

#endif /* serial0_h */
