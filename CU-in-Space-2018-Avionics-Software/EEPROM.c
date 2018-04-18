//
//  EEPROM.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2018-04-03.
//

#include "EEPROM.h"

#include <avr/io.h>
#include <util/atomic.h>
#include <stddef.h>

// MARK: Constants
#define QUEUE_LENGTH 4  // The number of EEPROM transactions which can be queued

#define ID_INVALID   0  // The transaction ID for an unused transaction
#define ID_FIRST     1  // The first valid transaction ID

// MARK: Structures
typedef struct {
    /** A unique identifer for this transaction */
    uint8_t id;

    /** The start address for this transaction*/
    uint16_t address;
    
    /** The buffer used for this transaction*/
    uint8_t *buffer;
    /** The number of bytes in this transaction*/
    uint8_t length;
    
    /** The number of bytes that have been read or writen*/
    uint8_t position;
    
    /** 1 if this is a write transaction */
    uint8_t write:1;
    /** 1 if this transaction is currently in progress */
    uint8_t active: 1;
    /** 1 if this transaction is complete */
    uint8_t done: 1;
} eeprom_transaction_t;

// MARK: Variables
/** The transaction queue */
static volatile eeprom_transaction_t queue[QUEUE_LENGTH];
/** The index of the head of the transaction queue */
static volatile uint8_t queue_head;

/** The transaction id that should be given to the next new transaction */
static uint8_t next_id = ID_FIRST;

/**
 *  Starts the next queued transaction if there is one and there is no currently active transaction.
 *  This function is inline so that is can be safely called from an ISR
 */
static inline void start_next_transaction (void) {
    if (queue[queue_head].active) return;
    
    uint8_t i = queue_head;
    do {
        if (((queue + i)->id != ID_INVALID) && !(queue + i)->active && !(queue + i)->done) {
            queue_head = i;
            volatile eeprom_transaction_t *t = queue + i;
            // Start transaction
            (queue + i)->active = 1;
            // Make sure that there isn't a write in progress already (there shouldn't be)
            while(EECR & (1<<EEPE));
            
            if (t->write) {
                // Write transaction, write first byte and leave the rest up to the ISR
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                    EEAR = t->address;
                    EEDR = t->buffer[t->position++];
                    EECR = (1<<EEMPE) | (1<<EERIE);
                    EECR |= (1<<EEPE);
                }
            } else {
                // Read transaction, perform read right away
                for (; t->position < t->length; t->position++) {
                    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                        EEAR = t->address + t->position;
                        EECR = (1<<EERE);
                        t->buffer[t->position] = EEDR;
                    }
                }
                t->done = 1;
                t->active = 0;
                queue_head = (queue_head + 1) % QUEUE_LENGTH;
            }
            return;
        }
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
}

void eeprom_service(void)
{
    start_next_transaction();
}

/**
 *  Get the first transaction in the queue with the given ID
 *  @param id The transaction ID to search for
 */
static volatile eeprom_transaction_t *get_transaction_with_id(uint8_t id)
{
    for (volatile eeprom_transaction_t *i = queue; i < queue + QUEUE_LENGTH; i++) {
        if (i->id == id) return i;
    }
    return NULL;
}

uint8_t eeprom_transaction_done(uint8_t transaction_id)
{
    volatile eeprom_transaction_t *t = get_transaction_with_id(transaction_id);
    return (t != NULL) ? (t->done) : 1;
}

uint8_t eeprom_clear_transaction(uint8_t transaction_id)
{
    volatile eeprom_transaction_t *t = get_transaction_with_id(transaction_id);
    if ((t != NULL) && !(t->active)) {
        t->id = ID_INVALID;
        return 0;
    }
    return 1;
}

/**
 *  Get the next transaction slot which is not currently in use
 */
static volatile eeprom_transaction_t *get_next_free_transaction(void)
{
    uint8_t i = queue_head;
    do {
        if (queue[i].id == ID_INVALID) return queue + i;
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
    return NULL;
}

uint8_t eeprom_write(uint8_t *transaction_id, uint16_t address, uint8_t *buffer, uint8_t length)
{
    volatile eeprom_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;

    t->address = address;
  
    t->buffer = buffer;
    t->length = length;
  
    t->position = 0;
  
    t->write = 1;
    t->active = 0;
    t->done = 0;
  
    eeprom_service();
    return 0;
}

uint8_t eeprom_read(uint8_t *transaction_id, uint16_t address, uint8_t *buffer, uint8_t length)
{
    volatile eeprom_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->address = address;
    
    t->buffer = buffer;
    t->length = length;
    
    t->position = 0;
    
    t->write = 0;
    t->active = 0;
    t->done = 0;
    
    eeprom_service();
    return 0;
}

uint8_t eeprom_read_byte_sync(uint16_t address)
{
    uint8_t result;
    while(EECR & (1<<EEPE)); // Wait for any ongoing write operations to finish
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        EEAR = address;
        EECR |= (1<<EERE);
        result = EEDR;
    }
    return result;
}

// MARK: Interupt service routines
ISR (EE_READY_vect)
{
    volatile eeprom_transaction_t *t = queue + queue_head;
    
    if (t->write && (t->position < t->length)) {
        // Write next byte
        EEAR = t->address + t->position;
        EEDR = t->buffer[t->position++];
        EECR = (1<<EEMPE) | (1<<EERIE);
        EECR |= (1<<EEPE);
    } else {
        // Finished transaction
        EECR = 0;   // Disable interupt
        t->done = 1;
        t->active = 0;
        queue_head = (queue_head + 1) % QUEUE_LENGTH;
    }
}
