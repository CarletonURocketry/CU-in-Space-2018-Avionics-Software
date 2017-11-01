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
 *  Determine if there is a transfer in progress on the i2c bus
 *  @returns 1 if there is an active transfer on the i2c bus, otherwise returns 0
 */
extern uint8_t i2c_transfer_active(void);


/**
 *  Determine if there is a byte avaliable to be read from the i2c bus RX buffer
 *  @returns 0 if there is no byte avaliable, a positive integer otherwise
 */
extern uint8_t i2c_has_byte (void);

/**
 *  Get the byte least recently placed in the i2c bus RX buffer
 *  @returns The least recently recieved byte in the i2c bus RX buffer, or 0 if the buffer is empty
 */
extern uint8_t i2c_get_byte (void);

/**
 *  Get the 2 bytes least recently placed in the i2c bus RX buffer
 *  @returns The least recently recieved word in the i2c bus RX buffer, or 0 if the buffer is empty
 */
extern uint16_t i2c_get_word (void);

/**
 *  Get the 4 bytes least recently placed in the i2c bus RX buffer
 *  @returns The least recently recieved long word in the i2c bus RX buffer, or 0 if the buffer is empty
 */
extern uint32_t i2c_get_long_word(void);


/**
 *  Writes a byte to a register via the i2c bus
 *  @param address The address of the slave to which the byte should be sent
 *  @param reg The address of the register to which the data should be written
 *  @param data The byte to be sent
 */
extern void i2c_write_byte(uint8_t address, uint8_t reg, uint8_t data);

/**
 *  Writes a word to a register via the i2c bus
 *  @param address The address of the slave to which the byte should be sent
 *  @param reg The address of the register to which the data should be written
 *  @param data The word to be sent
 */
extern void i2c_write_word(uint8_t address, uint8_t reg, uint16_t data);

/**
 *  Writes a block to a register via the i2c bus
 *  @param address The address of the slave to which the byte should be sent
 *  @param reg The address of the register to which the data should be written
 *  @param data An array of bytes to be sent
 *  @param length The number of bytes to be sent
 */
extern void i2c_write_block(uint8_t address, uint8_t reg, uint8_t *data, uint8_t length);

/**
 *  Read a one byte wide register via the i2c bus
 *  The received byte will be written to the i2c bus RX buffer
 *  @param address The address of the slave with which the transfer should be completed
 *  @param reg The address of the register which should be read
 */
extern void i2c_read_byte(uint8_t address, uint8_t reg);

/**
 *  Read one word wide a register via the i2c bus
 *  The received byte will be written to the i2c bus RX buffer
 *  @param address The address of the slave with which the transfer should be completed
 *  @param reg The address of the register which should be read
 */
extern void i2c_read_word(uint8_t address, uint8_t reg);

/**
 *  Read a register via the i2c bus
 *  The received byte will be written to the i2c bus RX buffer
 *  @param address The address of the slave with which the transfer should be completed
 *  @param reg The address of the register which should be read
 *  @param length The number of bytes to be read from the peripheral
 */
extern void i2c_read_block(uint8_t address, uint8_t reg, uint8_t length);

#endif /* I2C_h */
