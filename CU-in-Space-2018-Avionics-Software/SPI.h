//
//  SPI.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#ifndef SPI_h
#define SPI_h

#include "global.h"

/**
 *  Initializes the SPI interface.
 *  Clock will be 3MHz
 *  @param spi_port The output register for the IO port that the SPI pins (including CS pins) are on
 */
extern void init_spi(volatile uint8_t *spi_port);

/**
 *  Code to be run in each itteration of the main loop
 */
extern void spi_service(void);

/**
 * Determine if an SPI transaction has finished
 * @param transaction_id The identifier for the SPI transaction
 * @return 1 if the transaction has finished, 0 otherwise
 */
uint8_t spi_transaction_done(uint8_t transaction_id);

/**
 * Clear an SPI transaction
 * @note This function can not clear a transaction if it is active
 * @param transaction_id The identifier for the SPI transaction
 * @return 0 if the transaction was cleared
 */
uint8_t spi_clear_transaction(uint8_t transaction_id);

/**
 * Queue a half duplex transaction for the SPI bus
 * @param transaction_id The identifier assigned to the created transaction will be placed in this memory
 * @param cs_num The offset within the SPI port register for the chip select pin of the peripheral with which to communicate
 * @param out_buffer The memeory from which data will be sent
 * @param out_length The number of bytes to be sent
 * @param in_buffer The memory in which received data will be placed
 * @param in_length The number of bytes to be recieved
 * @return 0 if the transaction was added to the queue
 */
uint8_t spi_start_half_duplex(uint8_t *transaction_id, uint8_t cs_num, uint8_t *out_buffer, uint8_t out_length,
                              uint8_t * in_buffer, uint8_t in_length);

/**
 * Queue a full duplex transaction for the SPI bus
 * @param transaction_id The identifier assigned to the created transaction will be placed in this memory
 * @param cs_num The offset within the SPI port register for the chip select pin of the peripheral with which to communicate
 * @param out_buffer The memeory from which data will be sent
 * @param out_length The number of bytes to be sent
 * @param in_buffer The memory in which received data will be placed
 * @param attn_num The offset within the SPI port register for the attention pin of the peripheral with which to communicate
 * @return 0 if the transaction was added to the queue
 */
uint8_t spi_start_full_duplex(uint8_t *transaction_id, uint8_t cs_num, uint8_t *out_buffer, uint8_t out_length,
                              uint8_t * in_buffer, uint8_t attn_num);

#endif /* SPI_h */
