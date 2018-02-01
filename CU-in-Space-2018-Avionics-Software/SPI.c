//
//  SPI.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "SPI.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>


#include "serial0.h"
#include <stdlib.h>

typedef struct {
    uint8_t     transaction_id;     // A unique identifier for this transaction
    
    uint8_t     bytes_out;          // The number of bytes to be  written to the peripheral
    uint8_t     *out_buffer;        // The buffer which data is sent from
    uint8_t     bytes_in;           // The number of bytes to be read from the peripheral
    uint8_t     *in_buffer;         // The buffer in which recieved data is placed
    
    uint8_t     position;           // The current position within the transaction
    
    uint8_t     cs_num:3;           // The bit number for the CS pin of the peripheral within the spi output register
    uint8_t     attn_num:3;         // The bit number for the ATTN pin of the peripheral within the spi input register
    
    uint8_t     full_duplex:1;      // This bit if set if input should be regulated via the ATTN pin
    uint8_t     active:1;           // This bit is set when the transaction is currently on the bus
    uint8_t     byte_received:1;    // This bit is set if a byte was recieved last time the SPI interupt ran
    uint8_t     done:1;             // This bit if set if the transaction is complete, regardless of success
} spi_transaction_t;

// MARK: Constants
#define SPI_TRANSACTION_VOID    0   // The ID of an invalid transaction
#define SPI_TRANSACTION_FIRST   1   // The first valid transaction ID

#define SPI_NUM_TRANSACTIONS    4   // The maximum number of transactions which can be queued

// MARK: Variable Definitions

/** The transaction queue */
static volatile spi_transaction_t spi_transactions[SPI_NUM_TRANSACTIONS];

/** The index of the head of the transaction queue */
static volatile uint8_t spi_queue_position;

/** The next transaction ID that should be used*/
static volatile uint8_t spi_next_transaction;

/** The output register for the IO port where the SPI pins are located*/
static volatile uint8_t *spi_port_reg;

// MARK: Function Definitions
void init_spi(volatile uint8_t *spi_port)
{
    spi_next_transaction = SPI_TRANSACTION_FIRST;
    
    spi_port_reg = spi_port;
    // Set up SPI as follows:
    //  SPI interupt enabled
    //  SPI master mode
    //  SPI polarity/phase mode 0
    //  SPI clock rate is F_OSC/4 (3MHz)
    // and enable it
    SPCR |= (1<<SPIE) | (1<<SPE) | (1<<MSTR);
}

/**
 *  Determine the next pending transaction and start transmitting it.
 *  Does nothing if there is already an active transaction or if there are no pending transactions.
 */
static void spi_start_next_transaction (void)
{
    if (spi_transactions[spi_queue_position].active) {
        return;
    }
    
    uint8_t i = 0;
    do {
        if ((spi_transactions[i].transaction_id != SPI_TRANSACTION_VOID) && (!spi_transactions[i].done)) {
            spi_queue_position = i;
            // Start transaction
            spi_transactions[i].active = 1;
        
            *spi_port_reg &= ~(1<<spi_transactions[i].cs_num); // Assert CS pin
            
            if (spi_transactions[i].bytes_out > 0) {
                // Send first byte
                spi_transactions[i].position = 1;
                SPDR = spi_transactions[i].out_buffer[0];
            } else {
                // Send dummy byte
                SPDR = 0;
            }
            return;
        }
        i = ((i + 1) < SPI_NUM_TRANSACTIONS) ? i + 1 : 0;
    } while (i != spi_queue_position);
}


void spi_service(void)
{
    spi_start_next_transaction();
}

/**
 *  Get a transaction with a certain ID
 *  @param transaction_id The id of the transaction which should be retrieved
 *  @return A pointer the transaction with an in matching transaction_id. Null if there is no such transaction.
 */
static volatile spi_transaction_t *get_transaction (uint8_t transaction_id)
{
    if (transaction_id != SPI_TRANSACTION_VOID) {
        for (int i = 0; i < SPI_NUM_TRANSACTIONS; i++) {
            if (spi_transactions[i].transaction_id == transaction_id) {
                return &spi_transactions[i];
            }
        }
    }
    return NULL;
}

uint8_t spi_transaction_done(uint8_t transaction_id)
{
    volatile spi_transaction_t *trans = get_transaction(transaction_id);
    if (trans != NULL) {
        return trans->done;
    }
    return 0;
}

uint8_t spi_clear_transaction(uint8_t transaction_id)
{
    volatile spi_transaction_t *trans = get_transaction(transaction_id);
    if ((trans != NULL) && !(trans->active)) {
        trans->transaction_id = SPI_TRANSACTION_VOID;
        return 0;
    }
    return 1;
}

/**
 *  Gets a pointer to the next free slot in the transaction buffer
 *  @return A pointer to the next free transaction or NULL if there are no free transactions
 */
static volatile spi_transaction_t *get_next_free_transaction(void)
{
    uint8_t i = spi_queue_position;
    
    do {
        if (spi_transactions[i].transaction_id == SPI_TRANSACTION_VOID) {
            return &spi_transactions[i];
        }
        i = ((i + 1) < SPI_NUM_TRANSACTIONS) ? i + 1 : 0;
    } while (i != spi_queue_position);
    
    return NULL;
}

uint8_t spi_start_half_duplex(uint8_t *transaction_id, uint8_t cs_num, uint8_t *out_buffer, uint8_t out_length,
                              uint8_t * in_buffer, uint8_t in_length)
{
    serial_0_put_string("Started spi transaction with id: ");
    
    volatile spi_transaction_t *trans = get_next_free_transaction();
    if (trans == NULL) {
        return 1;
    }
    
    uint8_t next = ((spi_next_transaction + 1) == SPI_TRANSACTION_VOID) ? SPI_TRANSACTION_FIRST : spi_next_transaction + 1;
    
    trans->transaction_id = spi_next_transaction;
    *transaction_id = spi_next_transaction;
    spi_next_transaction = next;
    
    trans->cs_num = cs_num;
    trans->out_buffer = out_buffer;
    trans->bytes_out = out_length;
    trans->in_buffer = in_buffer;
    trans->bytes_in = in_length;
    
    trans->position = 0;
    
    trans->full_duplex = 0;
    trans->active = 0;
    trans->byte_received = 0;
    trans->done = 0;
    
    spi_start_next_transaction();
    
    char str[10];
    ultoa(trans->transaction_id, str, 16);
    serial_0_put_string(str);
    serial_0_put_byte('\n');
    
    return 0;
}

uint8_t spi_start_full_duplex(uint8_t *transaction_id, uint8_t cs_num, uint8_t *out_buffer, uint8_t out_length,
                              uint8_t * in_buffer, uint8_t attn_num)
{
    volatile spi_transaction_t *trans = get_next_free_transaction();
    if (trans == NULL) {
        return 1;
    }
    
    uint8_t next = ((spi_next_transaction + 1) == SPI_TRANSACTION_VOID) ? SPI_TRANSACTION_FIRST : spi_next_transaction + 1;
    
    trans->transaction_id = spi_next_transaction;
    *transaction_id = spi_next_transaction;
    spi_next_transaction = next;
    
    trans->cs_num = cs_num;
    trans->attn_num = attn_num;
    trans->out_buffer = out_buffer;
    trans->bytes_out = out_length;
    trans->in_buffer = in_buffer;
    trans->bytes_in = 0;
    
    trans->position = 0;
    
    trans->full_duplex = 1;
    trans->active = 0;
    trans->byte_received = 0;
    trans->done = 0;
    
    spi_start_next_transaction();
    
    return 0;
}

// MARK: Interupt service routines
ISR (SPI_STC_vect)
{
    volatile spi_transaction_t *trans = &(spi_transactions[spi_queue_position]);
    
    // Read
    if (trans->full_duplex && ((*spi_port_reg & (1 << trans->attn_num)) || trans->byte_received)) {
        // A byte should be recieved (full duplex)
        trans->in_buffer[trans->bytes_in] = SPDR;
        trans->bytes_in++;
        trans->byte_received = 1;
    } else if (!trans->full_duplex && (trans->bytes_out <= 0)) {
        // A byte should be recieved (half duplex)
//        uint8_t data = SPDR;
//        char str[10];
//        ultoa(data, str, 16);
//        serial_0_put_string("SPI Recieved Byte: 0x");
//        serial_0_put_string(str);
//        serial_0_put_byte('\n');
        trans->in_buffer[trans->position] = SPDR;
        trans->position++;
    } else {
        trans->byte_received = 0;
    }
    
    // Write
    if (trans->bytes_out > 0) {
        // A byte should be sent
        SPDR = trans->out_buffer[trans->position];
        trans->position++;
        trans->bytes_out--;
        if (trans->bytes_out <= 0) {
            // Finished transmitting, reset position
            trans->position = 0;
        }
    } else if (trans->full_duplex && ((*spi_port_reg & (1 << trans->attn_num)))) {
        // A dummy byte should be sent (full duplex)
        SPDR = 0;
    } else if (!trans->full_duplex && (trans->position < trans->bytes_in)) {
        // A dummy byte should be sent (half duplex)
        SPDR = 0;
    } else {
        // Transaction is finished
        *spi_port_reg ^= (1<< trans-> cs_num);  // De-assert CS pin
        trans->active = 0;
        trans->done = 1;
        
        serial_0_put_string("Finished spi transaction\n");
        
        spi_start_next_transaction();
    }
}
