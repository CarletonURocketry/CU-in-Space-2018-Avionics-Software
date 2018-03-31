//
//  25LC1024.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-11-01.
//
//  Read from and write to 25LC1024 eeprom via SPI bus
//

#ifndef _5LC1024_h
#define _5LC1024_h

#include "global.h"

/**
 *  Initilize the 25LC1024 EEPROM with the given CS num
 *  @param cs_num_0 The offset within the SPI output IO register for the CS pin of the first EEPROM
 *  @param cs_num_1 The offset within the SPI output IO register for the CS pin of the second EEPROM
 */
extern void init_25lc1024(uint8_t cs_num_0, uint8_t cs_num_1);

/**
 *  Code to be run in each iteration of the main loop
 */
extern void eeprom_25lc1024_service(void);

/**
 *  Check if an asynchronous eeprom transaction has finished
 *  @param transaction_id The identifier for the transaction
 */
extern uint8_t eeprom_25lc1024_transaction_done(uint8_t transaction_id);

/**
 *  Clear an asynchronous eeprom transaction from the queue
 *  @note This function can not clear a transction if it is currently active
 *  @param transaction_id The identifier for the transaction
 */
extern uint8_t eeprom_25lc1024_clear_transaction(uint8_t transaction_id);

/**
 *  Add a read transaction to the queue
 *  @note A read operation which runs off the end of the eeprom array will loop to be begining. Reads may span multiple pages.
 *  @param transaction_id The identifier for the transaction will be stored in this memory
 *  @param address The memory address where the read operation should start
 *  @param length The number of bytes to be read
 *  @param data The memory in which the bytes which are read will be placed
 */
extern uint8_t eeprom_25lc1024_read(uint8_t *transaction_id, uint32_t address, uint8_t length, uint8_t *data);

/**
 *  Add a write transaction to the queue
 *  @note Write operations must not cross a page boundry. Pages are 256 bytes long.
 *  @param transaction_id The identifier for the transaction will be stored in this memory
 *  @param address The memory address where the write operation should start
 *  @param length The number of bytes to be written
 *  @param data The memory from which the bytes will be written
 */
extern uint8_t eeprom_25lc1024_write(uint8_t *transaction_id, uint32_t address, uint8_t length,  uint8_t *data);


#endif /* _5LC1024_h */
