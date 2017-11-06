//
//  I2C.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#ifndef I2C_h
#define I2C_h

#include "global.h"

/**
 *  Initialize the TWI interface in fast mode (400kHz)
 */
extern void init_i2c(void);

/**
 *  Service to be run in each iteration of the main loop
 */
extern void i2c_service(void);

/**
 *  Check if an i2c transfer has finished
 *  @param transaction_id The transaction to check
 *  @return A non zero value if the transaction has finished
 */
extern uint8_t i2c_transfer_done(uint8_t transaction_id);

/**
 *  Check if an i2c transfer was successful
 *  @param transaction_id The transaction to check
 *  @return A non zero value if the transaction was successful
 */
extern uint8_t i2c_transfer_successful(uint8_t transaction_id);

/**
 *  Adds a write transaction to the queue
 *  @param transaction_id The transaction id will be placed at this address
 *  @param address The address of the slave peripheral
 *  @param reg The address of the register to be written
 *  @param data The data to be written
 *  @param length The number of byte to be written
 *  @return Zero if the transaction was sucessfully created
 */
extern uint8_t i2c_write(uint8_t *transaction_id, uint8_t address, uint8_t reg, uint8_t *data, uint8_t length);

/**
 *  Adds a read transaction to the queue
 *  @param transaction_id The transaction id will be placed at this address
 *  @param address The address of the slave peripheral
 *  @param reg The address of the register to be read
 *  @param data A buffer in which to place the data from the peripheral
 *  @param length The number of byte to be read
 *  @return Zero if the transaction was sucessfully created
 */
extern uint8_t i2c_read(uint8_t *transaction_id, uint8_t address, uint8_t reg, uint8_t *data, uint8_t length);

#endif /* I2C_h */

