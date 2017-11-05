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
 *  Registers an ATTN input and cs pin for a a full duplex device.
 *  @note Only one full duplex device is supported. This function should only be called once
 *  @param attn_pin The pin register for the ATTN input
 *  @param attn_num The bit within the pin register for the ATTN input
 *  @param cs_num The bit within the spi port register for the CS pin of the device associated with the ATTN pin
 */
extern void spi_register_full_duplex(volatile uint8_t *attn_pin, uint8_t attn_num, uint8_t cs_num);

/**
 *  Code to be run in each itteration of the main loop
 */
extern void spi_service(void);

/**
 *  Determine if there is a transfer in progress on the spi bus
 *  @returns A possitive integer if there is an active transfer on the spi bus, otherwise returns 0
 */
extern uint8_t spi_transfer_active(void);

/**
 *  Determine if there is a full transmition avaliable to be read from the spi full duplex input buffer
 *  @note This function should not be called with interupts disabled
 *  @return 0 if there is no line avaliable, 1 if a line is avaliable
 */
extern uint8_t spi_full_duplex_has_transmition(void);

/**
 *  Get the least recently recieved transmition from the spi full duplex input buffer
 *  @note This function should not be called with interupts disabled
 *  @param buf The string in which the data should be stored
 *  @param len The maximum number of bytes to be read from the spi full duplex input buffer
 *  @return The number of bytes copied
 */
extern uint8_t spi_full_duplex_get_transmition(uint8_t *buf, int len);

/**
 *  Start a half duplex transmition on the SPI bus. bytes_out bytes will be shifted out from *out_buf and then
 *  bytes_in bytes will be shifted into *in_buf.
 *  @param cs_num The bit within the spi port register for the CS pin of the slave device
 *  @param bytes_out The number of bytes to shift out
 *  @param out_buf The bytes to be shfited out
 *  @param bytes_in The number of bytes to shift in
 *  @param in_buf An array to place the bytes shifted in into
 *  @return 0 if the transmition could not be started, 1 otherwise
 */
extern uint8_t spi_start_half_duplex(uint8_t cs_num, uint8_t bytes_out, uint8_t *out_buf, uint8_t bytes_in, uint8_t *in_buf);

/**
 *  Starts a transmition to the previously registered full duplex device.
 *  Data may be shifted in while this transmition is taking place if the slave pulls it's ATTN pin high.
 *  @note This function must only be called after spi_register_full_duplex
 *  @param bytes_out The number of bytes to shift out
 *  @param out_buf The bytes to be shfited out
 *  @return 0 if the transmition could not be started, 1 otherwise
 */
extern uint8_t spi_start_full_duplex_write(uint8_t bytes_out, uint8_t *out_buf);

#endif /* SPI_h */
