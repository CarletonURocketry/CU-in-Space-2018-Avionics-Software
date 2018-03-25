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

// MARK: Constants
#define QUEUE_LENGTH 8
#define MAX_ERRORS 5

#define ID_INVALID 0
#define ID_FIRST 1

// MARK: Structures
typedef struct {
    uint8_t id;
    
    uint8_t cs_num;
    uint8_t attn_num;
    
    uint8_t out_length;
    uint8_t *out_buffer;
    uint8_t in_length;
    uint8_t *in_buffer;
    
    uint8_t bytes_out;
    uint8_t bytes_in;

    uint8_t full_duplex: 1;
    uint8_t last_attn: 1;
    uint8_t active: 1;
    uint8_t done_out: 1;
    uint8_t done: 1;
} spi_transaction_t;

// MARK: Variables
static volatile uint8_t *port;

static spi_transaction_t queue[QUEUE_LENGTH];
static uint8_t queue_head;

static uint8_t next_id = ID_FIRST;

// MARK: Functions
void init_spi(volatile uint8_t *spi_port)
{
    port = spi_port;
    SPCR |= (1<<SPIE)|(1<<SPE)|(1<<MSTR);  // Enable SPI interface in master mode with interupts
}

void spi_service(void)
{
    if (queue[queue_head].active) return;
        
    uint8_t i = queue_head;
    do {
        if ((queue[i].id != ID_INVALID) && !queue[i].active && !queue[i].done) {
            queue_head = i;
            // Start transaction
            queue[i].active = 1;
            
            *port &= ~(1<<queue[i].cs_num); // Assert CS pin
            if (queue[i].out_length > 0) {
                // Send first byte
                queue[i].bytes_out = 1;
                SPDR = queue[i].out_buffer[0];
            } else {
                // Send dummy byte
                SPDR = 0;
            }
            return;
        }
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
}

static spi_transaction_t *get_transaction_with_id(uint8_t id)
{
    for (spi_transaction_t *i = queue; i < queue + QUEUE_LENGTH; i++) {
        if (i->id == id) return i;
    }
    return NULL;
}

uint8_t spi_transaction_done(uint8_t transaction_id)
{
    spi_transaction_t *t = get_transaction_with_id(transaction_id);
    return (t != NULL) ? (t->done) : 0;
}

uint8_t spi_clear_transaction(uint8_t transaction_id)
{
    spi_transaction_t *t = get_transaction_with_id(transaction_id);
    if (t != NULL) {
        t->id = ID_INVALID;
        return 0;
    }
    return 1;
}

static spi_transaction_t *get_next_free_transaction(void)
{
    uint8_t i = queue_head;
    do {
        if (queue[i].id == ID_INVALID) return queue + i;
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
    return NULL;
}

uint8_t spi_start_half_duplex(uint8_t *transaction_id, uint8_t cs_num, uint8_t *out_buffer, uint8_t out_length,
                              uint8_t * in_buffer, uint8_t in_length)
{
    spi_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->bytes_out = 0;
    t->bytes_in = 0;
    t->full_duplex = 0;
    t->last_attn = 0;
    t->active = 0;
    t->done_out = 0;
    t->done = 0;
    
    t->cs_num = cs_num;
    t->attn_num = 0;
    t->out_buffer = out_buffer;
    t->out_length = out_length;
    t->in_buffer = in_buffer;
    t->in_length = in_length;
    
    spi_service();
    return 0;
}

uint8_t spi_start_full_duplex(uint8_t *transaction_id, uint8_t cs_num, uint8_t *out_buffer, uint8_t out_length,
                              uint8_t * in_buffer, uint8_t attn_num)
{
    return 1;
}

// MARK: Interupt service routines
ISR (SPI_STC_vect)
{
    spi_transaction_t *t = queue + queue_head;
    
    // Read
    if (!t->full_duplex && (t->done_out) && (t->bytes_in < t->in_length)) {
        // A byte should be recieved (half duplex)
        t->in_buffer[t->bytes_in++] = SPDR;
    } else if (t->full_duplex && (t->last_attn || (*port & (1 << t->attn_num))) && (t->bytes_in < t->in_length)) {
        // A byte should be recieved (full duplex)
        t->in_buffer[t->bytes_in++] = SPDR;
    }
    t->last_attn = (*port & (1 << t->attn_num));
    
    // Write
    if (t->bytes_out < t->out_length) {
        // A byte should be sent
        SPDR = t->out_buffer[t->bytes_out++];
    } else if ((t->full_duplex && t->last_attn) || (!t->full_duplex && (t->bytes_in < t->in_length))) {
        // Send dummy byte
        SPDR = 0;
        t->done_out = 1;
    } else {
        // Transaction is done
        *port |= (1<<t->cs_num); // De-assert CS pin
        t->done = 1;
        t->active = 0;
        queue_head = (queue_head + 1) % QUEUE_LENGTH;
        
        spi_service();
    }
}
