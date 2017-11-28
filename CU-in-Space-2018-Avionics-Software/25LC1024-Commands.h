//
//  25LC1024-Commands.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-11-13.
//
//  Commands avaliable on the 25LC1024 EEPROM
//

#ifndef _5LC1024_Commands_h
#define _5LC1024_Commands_h


#define READ        0b00000011      // Read from memory
#define WRITE       0b00000010      // Write to memory

#define WREN        0b00000110      // Set the write enable latch (enable write operations)
#define WRDI        0b00000100      // Reset the write enable latch (disable write operations)

#define RDSR        0b00000101      // Read the STATUS register
#define WRSR        0b00000001      // Write the STATUS register

#define PE          0b01000010      // Page erase
#define SE          0b11011000      // Sector erase
#define CE          0b11000111      // Chip erase

#define RDID        0b10101011      // Release from deep power-down mode and read electronic signature
#define DPD         0b10111001      // Enter deep power-down mode


#define SR_WPEN     7               // Write Protect Enable - Enable input for write protect pin
#define SR_BP1      3               // Block Protection 1 - Selects which areas of the array can be written
#define SR_BP2      2               // Block Protection 2 - Selects which areas of the array can be written
#define SR_WEL      1               // Write Enable Latch - Indicates the status of the write enable latch
#define SR_WIP      0               // Write in progress - Indicates whether the device is busy with a write operation


#define MAX_ADDRESS 0x1FFFF         // The highest valid address

#endif /* _5LC1024_Commands_h */
