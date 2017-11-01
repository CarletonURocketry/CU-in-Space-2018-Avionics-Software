//
//  I2C.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "I2C.h"

#include <avr/io.h>

// MARK: Constants
#define I2C_RX_BUFFER_LENGTH    256

#define I2C_WRITE_BIT   0
#define I2C_READ_BIT    1

#define I2C_MAX_ERRORS  3     // Number of errors before an I2C interaction is aborted

#define I2C_FLAGS_ACTIVE            0
#define I2C_FLAGS_SUCCESSFUL        1

// MARK: Variable Definitions
static char i2c_rx_buffer[I2C_RX_BUFFER_LENGTH];
static volatile uint8_t i2c_rx_insert_point;
static volatile uint8_t i2c_rx_withdraw_point;

static uint8_t *i2c_tx_buffer;
static uint16_t i2c_short_tx_data_buffer;

static volatile uint8_t i2c_address;
static volatile uint8_t i2c_reg;
static volatile uint8_t i2c_tx_length;
static volatile uint8_t i2c_rx_length;
static volatile uint8_t i2c_position;
static volatile uint8_t i2c_error_count;
static volatile uint8_t i2c_flags;

// MARK: Function Definitions
void init_i2c(void)
{
    
}

void i2c_service(void)
{
    
}

uint8_t i2c_transfer_active(void)
{
    return 1;
}

uint8_t i2c_has_byte (void)
{
    return 0;
}

uint8_t i2c_get_byte (void)
{
    return 0;
}

uint16_t i2c_get_word (void)
{
    return 0;
}

uint32_t i2c_get_long_word(void)
{
    return 0;
}


void i2c_write_byte(uint8_t address, uint8_t reg, uint8_t data)
{
    
}

void i2c_write_word(uint8_t address, uint8_t reg, uint16_t data)
{
    
}

void i2c_write_block(uint8_t address, uint8_t reg, uint8_t *data, uint8_t length)
{
    
}


void i2c_read_byte(uint8_t address, uint8_t reg)
{
    
}

void i2c_read_word(uint8_t address, uint8_t reg)
{
    
}

void i2c_read_block(uint8_t address, uint8_t reg, uint8_t length)
{
    
}

// MARK: Interupt service routines
ISR (TWI_vect)
{
    switch (TWSR & 0x03) {
        case 0x08:  // Transmition of start condition has finished
            // Transmit adress + W
            break;
        case 0x10:  // Transmition of repeated start has finished
            // Transmit adress + R
            break;
        case 0x18:  // Address + W has been sent, ACK recieved
            // Transmit Command
            break;
        case 0x20:  // Address + W has been sent, NOT ACK recieved
            // Increment error count and restart transaction or abort as appropriate
            // Restart transaction
            break;
        case 0x30:  // Data byte had been transmitted, NOT ACK recieved
            // Increment error count and restart transaction or abort as appropriate
            
            // Try to resend byte
            // Set up to resend command or decrement position as appropriate

            // Fall through to resend bytes
        case 0x28:  // Data byte had been transmitted, ACK recieved
            // Send next byte as appropriate

            break;
        case 0x38:  // Lost arbitration
            // Increment error count and restart transaction or abort as appropriate
            break;
        case 0x40:  // Address + R has been sent, ACK recieved
            // Reset possition
            // Receive bytes and send ACK or NACK as appropriate
            break;
        case 0x48:  // Address + R has been sent, NOT ACK recieved
            // Increment error count and resend address or abort as appropriate
            break;
        case 0x50:  // Data byte recieved, ACK sent
            // Get ready to receive next byte
            break;
        case 0x58:  // Data byte recieved, NOT ACK sent
            // Read finished, wrap up transaction
            break;
        default:
            // Should never be reached
            break;
    }
}
