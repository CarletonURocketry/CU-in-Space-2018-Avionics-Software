//
//  I2C.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "I2C.h"

#include <stddef.h> //NULL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

// MARK: Type Definitions
typedef struct {
    uint8_t     transaction_id;     // A unique identifier for this transaction
    
    uint8_t     slave_address;      // The 7-bit address for the peripheral
    uint8_t     slave_register;     // The register address to be read or written
    uint8_t     num_bytes;          // The number of bytes to be read or written from the peripheral register
    uint8_t     *buffer;            // The buffer which data is sent from or recieved data is placed in
    
    uint8_t     position;           // The current position within the transaction
    uint8_t     error_count;        // The number of errors which have occured during the current transaction
    
    uint8_t     write:1;            // If this bit is set this transaction is a write operation, otherwise a read operation
    uint8_t     active:1;           // This bit is set when the transaction is currently on the bus
    uint8_t     successful:1;       // This bit is set if the transaction is complete and did not fail
    uint8_t     done:1;             // This bit if set if the transaction is complete, regardless of success
} i2c_transaction_t;

// MARK: Constants
#define I2C_MAX_ERRORS          3   // Number of errors before an I2C interaction is aborted

#define I2C_TRANSACTION_VOID    0   // The ID of an invalid transaction
#define I2C_TRANSACTION_FIRST   1   // The first valid transaction ID

#define I2C_NUM_TRANSACTIONS    8   // The maximum number of transactions which can be buffered

// MARK: Variable Definitions

/** The buffer of i2c transactions*/
i2c_transaction_t i2c_transactions[I2C_NUM_TRANSACTIONS];
/** The address to be given to the next transaction on the i2c bus*/
uint8_t i2c_next_transaction;

uint8_t i2c_buffer_position;

// MARK: Function Definitions
void init_i2c(void)
{
    i2c_next_transaction = I2C_TRANSACTION_FIRST;
}

/**
 *  Determine the next pending transaction and start transmitting it.
 *  Does nothing if there is already an active transaction or if there are no pending transactions.
 */
void i2c_start_next_transaction (void)
{
    if (i2c_transactions[i2c_buffer_position].active) {
        return;
    }
    
    uint8_t i = 0;
    do {
        if ((i2c_transactions[i].transaction_id != I2C_TRANSACTION_VOID) && (!i2c_transactions[i].done)) {
            i2c_buffer_position = i;
            // start i2c_transactions[i]
			i2c_transactions[i2c_buffer_position].active = 1; // set i2c_transactions[i] to active
			TWCR = _BV(TWEN)|_BV(TWSTA)|_BV(TWIE); // send START condition and enable interrupt
			TWCR |= _BV(TWINT);
            return;
        }
        i = ((i + 1) < I2C_NUM_TRANSACTIONS) ? i + 1 : 0;
    } while (i != i2c_buffer_position);
}

void i2c_service(void)
{
    i2c_start_next_transaction();
}

/**
 *  Get a transaction with a certain ID
 *  @param transaction_id The id of the transaction which should be retrieved
 *  @return A pointer the transaction with an in matching transaction_id. Null if there is no such transaction.
 */
i2c_transaction_t *get_transaction (uint8_t transaction_id)
{
    if (transaction_id != I2C_TRANSACTION_VOID) {
        for (int i = 0; i < I2C_NUM_TRANSACTIONS; i++) {
            if (i2c_transactions[i].transaction_id == transaction_id) {
                return &i2c_transactions[i];
            }
        }
    }
    return NULL;
}

uint8_t i2c_transaction_done(uint8_t transaction_id)
{
    i2c_transaction_t *trans = get_transaction(transaction_id);
    if (trans != NULL) {
        return trans->done;
    }
    return 0;
}

uint8_t i2c_transaction_successful(uint8_t transaction_id)
{
    i2c_transaction_t *trans = get_transaction(transaction_id);
    if (trans != NULL) {
        return trans->successful;
    }
    return 0;
}

uint8_t i2c_clear_transaction(uint8_t transaction_id)
{
    i2c_transaction_t *trans = get_transaction(transaction_id);
    if ((trans != NULL) && !(trans->active)) {
        trans->transaction_id = I2C_TRANSACTION_VOID;
        return 0;
    }
    return 1;
}

/**
 *  Gets a pointer to the next free slot in the transaction buffer
 *  @return A pointer to the next free transaction or NULL if there are no free transactions
 */
i2c_transaction_t *get_next_free_transaction(void) {
    uint8_t i = i2c_buffer_position;
    
    do {
        if (i2c_transactions[i].transaction_id == I2C_TRANSACTION_VOID) {
            return &i2c_transactions[i];
        }
        i = ((i + 1) < I2C_NUM_TRANSACTIONS) ? i + 1 : 0;
    } while (i != i2c_buffer_position);
    
    return NULL;
}

uint8_t i2c_write(uint8_t *transaction_id, uint8_t address, uint8_t reg, uint8_t *data, uint8_t length)
{
    i2c_transaction_t *trans = get_next_free_transaction();
    if (trans == NULL) {
        return 1;
    }
    
    uint8_t next = ((i2c_next_transaction + 1) == I2C_TRANSACTION_VOID) ? I2C_TRANSACTION_FIRST : i2c_next_transaction + 1;
    
    trans->transaction_id = i2c_next_transaction;
    *transaction_id = i2c_next_transaction;
    i2c_next_transaction = next;
    
    trans->slave_address = address;
    trans->slave_register = reg;
    trans->buffer = data;
    trans->num_bytes = length;
    
    trans->position = 0;
    trans->error_count = 0;
    
    trans->write = 1;
    trans->active = 0;
    trans->successful = 0;
    trans->done = 0;
    
    i2c_start_next_transaction();
    
    return 0;
}

uint8_t i2c_read(uint8_t *transaction_id, uint8_t address, uint8_t reg, uint8_t *data, uint8_t length)
{
    i2c_transaction_t *trans = get_next_free_transaction();
    if (trans == NULL) {
        return 1;
    }
    
    uint8_t next = ((i2c_next_transaction + 1) == I2C_TRANSACTION_VOID) ? I2C_TRANSACTION_FIRST : i2c_next_transaction + 1;
    
    trans->transaction_id = i2c_next_transaction;
    *transaction_id = i2c_next_transaction;
    i2c_next_transaction = next;
    
    trans->slave_address = address;
    trans->slave_register = reg;
    trans->buffer = data;
    trans->num_bytes = length;
    
    trans->position = 0;
    trans->error_count = 0;
    
    trans->write = 0;
    trans->active = 0;
    trans->successful = 0;
    trans->done = 0;
    
    i2c_start_next_transaction();
    
    return 0;
}

// MARK: Interupt service routines
ISR (TWI_vect)
{
    switch (TW_STATUS) {
        case TW_START:  // Transmition of start condition has finished
            // Transmit adress + W
			TWDR = (i2c_transactions[i2c_buffer_position].slave_address << 1); // write bit is low at SDA
			TWCR &= ~_BV(TWSTA); //TWSTA must be cleared by software 
            break;
        case TW_REP_START:  // Transmition of repeated start has finished
            // Transmit adress + R
			TWDR = (i2c_transactions[i2c_buffer_position].slave_address << 1); // write bit is low at SDA
			if (i2c_transactions[i2c_buffer_position].write == 0) TWDR |= TW_READ; // if this is a reading operation set the last bit to 1
			TWCR &= ~_BV(TWSTA); //TWSTA must be cleared by software
            break;
        case TW_MT_SLA_ACK:  // Address + W has been sent, ACK recieved
            // Transmit Command
			TWDR = i2c_transactions[i2c_buffer_position].slave_register;
			// increment position indicator
			i2c_transactions[i2c_buffer_position].position++;
            break;
        case TW_MT_SLA_NACK:  // Address + W has been sent, NOT ACK recieved
            // Increment error count and restart transaction or abort as appropriate
            // Restart transaction
	    TWCR |= _BV(TWSTO);
			if (++i2c_transactions[i2c_buffer_position].error_count <= I2C_MAX_ERRORS)
			{
				i2c_transactions[i2c_buffer_position].position = 0;
				TWCR |= _BV(TWSTA);
			}
			else // max error number reached, abort this operation
			{
				i2c_transactions[i2c_buffer_position].successful = 0;
				i2c_transactions[i2c_buffer_position].active = 0;
				i2c_transactions[i2c_buffer_position].done = 1;
			}
            break;
        case TW_MT_DATA_NACK:  // Data byte had been transmitted, NOT ACK recieved
            // Increment error count and restart transaction or abort as appropriate
            if (++i2c_transactions[i2c_buffer_position].error_count > I2C_MAX_ERRORS)
            {
				i2c_transactions[i2c_buffer_position].successful = 0;
				i2c_transactions[i2c_buffer_position].active = 0;
				i2c_transactions[i2c_buffer_position].done = 1;
				TWCR |= _BV(TWSTO);
				goto OPERATION_CONTINUE;
            }
            // Try to resend byte
            // Set up to resend command or decrement position as appropriate
			i2c_transactions[i2c_buffer_position].position--;
            // Fall through to resend bytes
        case TW_MT_DATA_ACK:  // Data byte had been transmitted, ACK recieved
            // Send next byte as appropriate
			// if we are writing to a register, use writing routine
			if (i2c_transactions[i2c_buffer_position].write) {
				if (++i2c_transactions[i2c_buffer_position].position > i2c_transactions[i2c_buffer_position].num_bytes)
				{
					// data write complete, release the i2c transaction
					TWCR = _BV(TWSTO);
					i2c_transactions[i2c_buffer_position].successful = 1;
					i2c_transactions[i2c_buffer_position].active = 0;
					i2c_transactions[i2c_buffer_position].done = 1;
				}
				else TWDR = (i2c_transactions[i2c_buffer_position].position > 0) ? *(i2c_transactions[i2c_buffer_position].buffer + i2c_transactions[i2c_buffer_position].position - 1) : i2c_transactions[i2c_buffer_position].slave_register;
			}
			else {
				// that means the register address is successfully sent, send repeated start signal
				TWCR |= _BV(TWSTA);
			}
            break;
        case TW_MT_ARB_LOST:  // Lost arbitration
            // Increment error count and restart transaction or abort as appropriate
			// resend START condition when the bus becomes free
			TWCR |= _BV(TWSTA);
			i2c_transactions[i2c_buffer_position].position = 0;
            break;
        case TW_MR_SLA_ACK:  // Address + R has been sent, ACK recieved
            // Reset possition
            // Receive bytes and send ACK or NACK as appropriate
			i2c_transactions[i2c_buffer_position].position = 0;
			if (i2c_transactions[i2c_buffer_position].num_bytes > 1)
			{
				TWCR |= _BV(TWEA);
			} 
			else
			{
				TWCR &= ~_BV(TWEA);
			}
            break;
        case TW_MR_SLA_NACK:  // Address + R has been sent, NOT ACK recieved
            // Increment error count and resend address or abort as appropriate
			if (++i2c_transactions[i2c_buffer_position].error_count <= I2C_MAX_ERRORS)
			{
				TWDR = (i2c_transactions[i2c_buffer_position].slave_address << 1) | TW_READ;
			} 
			else
			{
				TWCR |= _BV(TWSTO);
				i2c_transactions[i2c_buffer_position].successful = 0;
				i2c_transactions[i2c_buffer_position].active = 0;
				i2c_transactions[i2c_buffer_position].done = 1;
			}
            break;
        case TW_MR_DATA_ACK:  // Data byte recieved, ACK sent
            // Get ready to receive next byte
			++(i2c_transactions[i2c_buffer_position].position);
			if ((i2c_transactions[i2c_buffer_position].position) + 1 < i2c_transactions[i2c_buffer_position].num_bytes)
			{
				TWCR |= _BV(TWEA);
			} 
			else
			{
				TWCR &= ~_BV(TWEA);
			}
            break;
        case TW_MR_DATA_NACK:  // Data byte recieved, NOT ACK sent
            // Read finished, wrap up transaction
			++(i2c_transactions[i2c_buffer_position].position);
			TWCR |= _BV(TWSTO);
			i2c_transactions[i2c_buffer_position].successful = 1;
			i2c_transactions[i2c_buffer_position].active = 0;
			i2c_transactions[i2c_buffer_position].done = 1;
            break;
        default:
            // Should never be reached
            break;
    }
OPERATION_CONTINUE:
	TWCR |= _BV(TWINT); // continue operation
}
