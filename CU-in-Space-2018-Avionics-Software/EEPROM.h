//
//  EEPROM.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2018-04-03.
//

#ifndef EEPROM_h
#define EEPROM_h

#include "global.h"

/**
 *  Code to be run in each itteration of the main loop
 */
extern void eeprom_service(void);

/**
 * Determine if an EEPROM transaction has finished
 * @param transaction_id The identifier for the EEPROM transaction
 * @return 1 if the transaction has finished, 0 otherwise
 */
extern uint8_t eeprom_transaction_done(uint8_t transaction_id);

/**
 * Clear an EEPROM transaction
 * @note This function can not clear a transaction if it is active
 * @param transaction_id The identifier for the EEPROM transaction
 * @return 0 if the transaction was cleared
 */
extern uint8_t eeprom_clear_transaction(uint8_t transaction_id);

/**
 * Queue data to be writen to EEPROM
 * @param transaction_id The identifier assigned to the created transaction will be placed in this memory
 * @param address The address where the EEPROM transaction will start
 * @param buffer The memory from which data will be written to EEPROM
 * @param length The number of bytes to be writen to EEPROM
 * @return 0 if the transaction was added to the queue
 */
extern uint8_t eeprom_write(uint8_t *transaction_id, uint16_t address, uint8_t * buffer, uint8_t length);

/**
 * Queue a read operation for the EEPROM
 * @param transaction_id The identifier assigned to the created transaction will be placed in this memory
 * @param address The address where the EEPROM transaction will start
 * @param buffer The memory where data from EEPROM will be stored
 * @param length The number of bytes to be read from eeprom
 * @return 0 if the transaction was added to the queue
 */
extern uint8_t eeprom_read(uint8_t *transaction_id, uint16_t address, uint8_t * buffer, uint8_t length);

/**
 * Read a byte from eeprom as quickly as possible and return it immediately
 * @param address The address to be read
 * @return The value stored in EEPROM at the address given
 */
extern uint8_t eeprom_read_byte_sync(uint16_t address);

#endif /* EEPROM_h */
