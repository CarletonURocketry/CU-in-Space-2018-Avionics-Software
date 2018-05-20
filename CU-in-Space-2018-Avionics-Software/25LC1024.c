//
//  25LC1024.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-11-01.
//

#include "25LC1024.h"
#include "25LC1024-Commands.h"

#include <string.h>

#include "SPI.h"

typedef enum {QUEUED, WAKE, WRITE_EN, ACTION, CHECK_STAT, SLEEP, DONE} eeprom_state_t;

typedef struct {
    /** A unique identifer for this transaction */
    uint8_t id;
    /** The current state of this transaction */
    eeprom_state_t state;
    
    /** The ID of the SPI transaction used by this eeprom transaction */
    uint8_t spi_id;

    /** The pin number of the CS pin used in this transaction */
    uint8_t cs_num;
    /** The EEPROM address used in this transaction */
    uint32_t address;
    /** The data buffer used in this transaction */
    uint8_t *data;
    /** The number of bytes to be read or written in this transaction */
    uint8_t length;
    
    /** Whether this is a write transaction */
    uint8_t write: 1;
    /** Whether this is an erase transaction */
    uint8_t erase: 1;
} eeprom_transaction_t;

// MARK: Constants
#define QUEUE_LENGTH    5   // The number of SPI transactions which can be queued
#define BUFFER_LENGTH   260 // The size of the output buffer

#define ID_INVALID   0  // The transaction ID for an unused transaction
#define ID_FIRST     1  // The first valid transaction ID

// MARK: Variable Definitions
/** The chip select pin for the first eeprom */
static volatile uint8_t cs_num_0;
/** The chip select pin for the second eeprom*/
static volatile uint8_t cs_num_1;

/** The transaction queue */
static eeprom_transaction_t queue[QUEUE_LENGTH];
/** The index of the head of the transaction queue */
static uint8_t queue_head;

/** The transaction id that should be given to the next new transaction */
static uint8_t next_id = ID_FIRST;

/** The buffer used in communications with the EEPROM*/
static uint8_t buffer[BUFFER_LENGTH];

// MARK: Function Definitions
void init_25lc1024(uint8_t eeprom_cs_num_0, uint8_t eeprom_cs_num_1)
{
    cs_num_0 = eeprom_cs_num_0;
    cs_num_1 = eeprom_cs_num_1;
}

/**
 *  Determine the next pending transaction and start it.
 *  Does nothing if there is already an active transaction or if there are no pending transactions.
 */
static void eeprom_start_next_transaction (void)
{
    if ((queue[queue_head].state != QUEUED) && (queue[queue_head].state != DONE)) return;
    
    uint8_t i = queue_head;
    do {
        if ((queue[i].id != ID_INVALID) && (queue[i].state == QUEUED)) {
            queue_head = i;
            // Start transaction by waking the eeprom
            buffer[0] = RDID;
            spi_start_half_duplex(&queue[i].spi_id, queue[i].cs_num, buffer, 4,  buffer + 4, 1);
            queue[i].state = WAKE;
            return;
        }
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
}

void eeprom_25lc1024_service(void)
{
    eeprom_transaction_t *t = queue + queue_head;
    if (!spi_transaction_done(t->spi_id)) return;
    spi_clear_transaction(t->spi_id);
    
    switch (t->state) {
        case QUEUED:
            // Shouldn't happen
            break;
        case WAKE:
            // Finished reading SIG, move on to read action or right enable
            if (t->write || t->erase) {
                // WIP cleared, we need to enable writes now
                buffer[0] = WREN;
                spi_start_half_duplex(&t->spi_id, t->cs_num, buffer, 1, NULL, 0);
                t->state = WRITE_EN;
                break;
            }
            // If we make it here we are ready to start a read transaction by falling through to write_en done
        case WRITE_EN:
            // Write EN done. Perform action
            if (!t->erase) {
                buffer[0] = (t->write) ? WRITE : READ;
                buffer[1] = ((uint8_t*)(&t->address))[2];
                buffer[2] = ((uint8_t*)(&t->address))[1];
                buffer[3] = ((uint8_t*)(&t->address))[0];
                if (t->write) memcpy(buffer + 4, t->data, t->length);
                spi_start_half_duplex(&t->spi_id, t->cs_num, buffer, (t->write) ? t->length + 4 : 4,  buffer + 4,
                                      (t->write) ? 0 : t->length);
            } else {
                buffer[0] = CE;
                spi_start_half_duplex(&t->spi_id, t->cs_num, buffer, 1, NULL, 0);
            }
            t->state = ACTION;
            break;
        case ACTION:
            // Action finished. Check stat
            if (t->write) {
                buffer[0] = RDSR;
                spi_start_half_duplex(&t->spi_id, t->cs_num, buffer, 1,  buffer + 1, 1);
                t->state = CHECK_STAT;
                break;
            } else {
                memcpy(t->data, buffer + 4, t->length);
                buffer[1] = 0;
            }
        case CHECK_STAT:
            // Finished reading STAT, check if WIP is set, if it is check stat again, if it isn't put eeprom to sleep
            if (buffer[1] & (1<<SR_WIP)) {
                // WIP still set
                spi_start_half_duplex(&t->spi_id, t->cs_num, buffer, 1,  buffer + 1, 1);
            } else {
                // WIP cleared, enter sleep
                buffer[0] = DPD;
                spi_start_half_duplex(&t->spi_id, t->cs_num, buffer, 1,  NULL, 0);
                t->state = SLEEP;
            }
            break;
        case SLEEP:
            // Sleep mode has been entered, clean up and start the next transaction
            if (t->erase && (t->cs_num == cs_num_0)) {
                // If this is an erase transaction we need to restart from the begining to erase the second chip
                t->cs_num = cs_num_1;
                buffer[0] = RDID;
                spi_start_half_duplex(&t->spi_id, t->cs_num, buffer, 4, buffer + 4, 1);
                t->state = WAKE;
            } else {
                t->state = DONE;
                queue_head = (queue_head + 1) % QUEUE_LENGTH;
                eeprom_start_next_transaction();
            }
            break;
        case DONE:
            // Shouldn't happen
            break;
    }
}


/**
 *  Get a transaction with a certain ID
 *  @param transaction_id The id of the transaction which should be retrieved
 *  @return A pointer the transaction with an in matching transaction_id. Null if there is no such transaction.
 */
static volatile eeprom_transaction_t *get_transaction_with_id (uint8_t id)
{
    for (eeprom_transaction_t *i = queue; i < queue + QUEUE_LENGTH; i++) {
        if (i->id == id) return i;
    }
    return NULL;
}

uint8_t eeprom_25lc1024_transaction_done(uint8_t transaction_id)
{
    volatile eeprom_transaction_t *t = get_transaction_with_id(transaction_id);
    return (t != NULL) ? (t->state == DONE) : 0;
}

uint8_t eeprom_25lc1024_clear_transaction(uint8_t transaction_id)
{
    volatile eeprom_transaction_t *t = get_transaction_with_id(transaction_id);
    if ((t != NULL) && (t->state == DONE)) {
        t->id = ID_INVALID;
        return 0;
    }
    return 1;
}

/**
 *  Gets a pointer to the next free slot in the transaction buffer
 *  @return A pointer to the next free transaction or NULL if there are no free transactions
 */
static eeprom_transaction_t *get_next_free_transaction(void)
{
    uint8_t i = queue_head;
    do {
        if (queue[i].id == ID_INVALID) return queue + i;
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
    return NULL;
}


uint8_t eeprom_25lc1024_read(uint8_t *transaction_id, uint32_t address, uint8_t length, uint8_t *data)
{
    eeprom_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->state = QUEUED;
    t->spi_id = 0;
    
    t->cs_num = (address <= MAX_ADDRESS) ? cs_num_0 : cs_num_1;
    t->address = (address <= MAX_ADDRESS) ? address : address - MAX_ADDRESS - 1;
    t->data = data;
    t->length = length;
    
    t->write = 0;
    t->erase = 0;
    
    eeprom_start_next_transaction();
    return 0;
}

uint8_t eeprom_25lc1024_write(uint8_t *transaction_id, uint32_t address, uint8_t length,  uint8_t *data)
{
    eeprom_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->state = QUEUED;
    t->spi_id = 0;
    
    t->cs_num = (address <= MAX_ADDRESS) ? cs_num_0 : cs_num_1;
    t->address = (address <= MAX_ADDRESS) ? address : address - MAX_ADDRESS - 1;
    t->data = data;
    t->length = length;
    
    t->write = 1;
    t->erase = 0;
    
    eeprom_start_next_transaction();
    return 0;
}

uint8_t eeprom_25lc1024_chip_erase(uint8_t *transaction_id)
{
    eeprom_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->state = QUEUED;
    t->spi_id = 0;
    
    t->cs_num = cs_num_0;
    t->address = 0;
    t->data = NULL;
    t->length = 0;
    
    t->write = 0;
    t->erase = 1;
    
    eeprom_start_next_transaction();
    return 0;
}
