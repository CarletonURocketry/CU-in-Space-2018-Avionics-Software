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

typedef enum {QUEUED, READ_SIG, CHECK_WIP, ENABLE_WRITE, ACTION, FINISHED} eeprom_state_t;

typedef struct {
    uint8_t         transaction_id;     // A unique identifier for this transaction
    
    uint8_t         spi_transaction;    // The identifier of the SPI transaction associated with this EEPROM transaction
    
    eeprom_state_t  state;              // The current state of this transaction
    
    uint8_t         command;            // The command to be used in this transaction
    uint8_t         *address;           // The memory address for this transaction (3 bytes)
    uint8_t         *data;              // The buffer to be used for sending or receiveing data in this transaction
    uint8_t         length;             // The number of bytes to be sent or recieved
    
    uint8_t         write:1;            // This bit is set if this is a write transaction
    uint8_t         send_addr:1;        // This bit is set if the address should be transmitted
    uint8_t         active:1;           // This bit is set when the transaction is currently in progress
    uint8_t         done:1;             // This bit if set if the transaction is complete, regardless of success
    uint8_t         successfull:1;      // This bit is set if the transaction was successfull
    
} eeprom_transaction_t;

// MARK: Constants
#define EEPROM_NUM_TRANSACTIONS     4   // The maximum number of transactions which can be queued

#define EEPROM_TRANSACTION_VOID     0   // The transaction ID for a void transaction
#define EEPROM_TRANSACTION_FIRST    1   // The first valid transaction ID

// MARK: Variable Definitions
static volatile uint8_t cs_port_num;

/** The transaction queue */
static eeprom_transaction_t eeprom_transactions[EEPROM_NUM_TRANSACTIONS];

/** The index of the head of the transaction queue */
static uint8_t eeprom_queue_position;

/** The next transaction ID that should be used*/
static uint8_t eeprom_next_transaction;

static uint8_t eeprom_out_buffer[260];

/** The data which should be written to the status register */
static uint8_t init_data[] = {0b00000000};

/** A buffer in which various responses can be stored*/
static uint8_t response_buffer[1];

// MARK: Function Definitions
uint8_t init_25lc1024(uint8_t cs_num)
{
    cs_port_num = cs_num;
    eeprom_next_transaction = EEPROM_TRANSACTION_FIRST + 1;
    
    eeprom_transaction_t *trans = &eeprom_transactions[0];
    
    eeprom_queue_position = EEPROM_TRANSACTION_FIRST;
    
    trans->transaction_id = EEPROM_TRANSACTION_FIRST;
    trans->state = READ_SIG;
    
    trans->write = 1;
    trans->send_addr = 0;
    trans->active = 1;
    trans->done = 0;
    trans->successfull = 1;
    
    trans->command = WRSR;
    trans->data = init_data;
    trans->length = 1;
    
    eeprom_out_buffer[0] = RDID;
    eeprom_out_buffer[1] = 0;
    eeprom_out_buffer[2] = 0;
    eeprom_out_buffer[3] = 0;
    
    spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, 4, response_buffer, 1);
    
    return 0;
}

/**
 *  Determine the next pending transaction and start it.
 *  Does nothing if there is already an active transaction or if there are no pending transactions.
 */
static void eeprom_start_next_transaction (void)
{
    if (eeprom_transactions[eeprom_queue_position].active) {
        return;
    }
    
    uint8_t i = 0;
    do {
        if ((eeprom_transactions[i].transaction_id != EEPROM_TRANSACTION_VOID) && (!eeprom_transactions[i].done)) {
            eeprom_queue_position = i;
            // Start transaction
            eeprom_transactions[i].active = 1;
            
            eeprom_out_buffer[0] = RDID;
            eeprom_out_buffer[1] = 0;
            eeprom_out_buffer[2] = 0;
            eeprom_out_buffer[3] = 0;
            
            spi_start_half_duplex(&(eeprom_transactions[i].spi_transaction), cs_port_num, eeprom_out_buffer, 4, response_buffer, 1);
            
            return;
        }
        i = ((i + 1) < EEPROM_NUM_TRANSACTIONS) ? i + 1 : 0;
    } while (i != eeprom_queue_position);
}

void eeprom_25lc1024_service(void)
{
    eeprom_transaction_t *trans = &(eeprom_transactions[eeprom_queue_position]);
    
    if (!trans->active) {
        eeprom_start_next_transaction();
        return;
    }
    // QUEUED, READ_SIG, CHECK_WIP, ENABLE_WRITE, WRITE, DPD, FINISHED
    switch (trans->state) {
        case QUEUED:
            // This shouldn't happen
            break;
        case READ_SIG:
            // Result of SPI transaction should be the 8bit signature of the EEPROM (0x29)
            if (spi_transaction_done(trans->spi_transaction)) {
                trans->state = CHECK_WIP;
                eeprom_out_buffer[0] = RDSR;
                spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, 1, response_buffer, 1);
            }
            break;
        case CHECK_WIP:
            // Result of SPI transaction should be the STATUS regsiter, check that WIP bit is cleared and send WREN if writing
            if (spi_transaction_done(trans->spi_transaction)) {
                if (response_buffer[0] & (1<<SR_WIP)) {
                    // Write is in progress, check WIP again
                    spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, 1, response_buffer, 1);
                    break;
                } else if (trans->write) {
                    // No write in progress, need to send WREN
                    trans->state = ENABLE_WRITE;
                    eeprom_out_buffer[0] = WREN;
                    spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, 1, NULL, 0);
                    break;
                } else {
                    // No write in progress, don't need to send WREN
                }
            } else {
                break;
            }
        case ENABLE_WRITE:
            // WREN sent, Write action can now take place or Read action can now take place
            if (spi_transaction_done(trans->spi_transaction)) {
                trans->state = ACTION;
                eeprom_out_buffer[0] = trans->command;
                if (trans->write && trans->send_addr) {
                    memcpy(eeprom_out_buffer + 1, trans->address, 3);
                    memcpy(eeprom_out_buffer + 4, trans->data, trans->length);
                    spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, trans->length + 4, NULL, 0);
                } else if (trans->write) {
                    memcpy(eeprom_out_buffer + 1, trans->data, trans->length);
                    spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, trans->length + 1, NULL, 0);
                } else if (trans->send_addr) {
                    memcpy(eeprom_out_buffer + 1, trans->address, 3);
                    spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, 4, trans->data, trans->length);
                } else {
                    spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, 1, trans->data, trans->length);
                }
            }
            break;
        case ACTION:
            // Action preformed, handle result and return to DPD
            if (spi_transaction_done(trans->spi_transaction)) {
                trans->state = FINISHED;
                eeprom_out_buffer[0] = DPD;
                spi_start_half_duplex(&(trans->spi_transaction), cs_port_num, eeprom_out_buffer, 1, NULL, 0);
            }
            break;
        case FINISHED:
            // DPD entered, transaction done
            if (spi_transaction_done(trans->spi_transaction)) {
                trans->done = 1;
                trans->active = 0;
                eeprom_start_next_transaction();
            }
            break;
    }
}


/**
 *  Get a transaction with a certain ID
 *  @param transaction_id The id of the transaction which should be retrieved
 *  @return A pointer the transaction with an in matching transaction_id. Null if there is no such transaction.
 */
static volatile eeprom_transaction_t *get_transaction (uint8_t transaction_id)
{
    if (transaction_id != EEPROM_TRANSACTION_VOID) {
        for (int i = 0; i < EEPROM_NUM_TRANSACTIONS; i++) {
            if (eeprom_transactions[i].transaction_id == transaction_id) {
                return &eeprom_transactions[i];
            }
        }
    }
    return NULL;
}

uint8_t eeprom_25lc1024_transaction_done(uint8_t transaction_id)
{
    volatile eeprom_transaction_t *trans = get_transaction(transaction_id);
    if (trans != NULL) {
        return trans->done;
    }
    return 0;
}

uint8_t eeprom_25lc1024_clear_transaction(uint8_t transaction_id)
{
    volatile eeprom_transaction_t *trans = get_transaction(transaction_id);
    if ((trans != NULL) && !(trans->active)) {
        trans->transaction_id = EEPROM_TRANSACTION_VOID;
        return 0;
    }
    return 1;
}


/**
 *  Gets a pointer to the next free slot in the transaction buffer
 *  @return A pointer to the next free transaction or NULL if there are no free transactions
 */
static volatile eeprom_transaction_t *get_next_free_transaction(void)
{
    uint8_t i = eeprom_queue_position;
    
    do {
        if (eeprom_transactions[i].transaction_id == EEPROM_TRANSACTION_VOID) {
            return &eeprom_transactions[i];
        }
        i = ((i + 1) < EEPROM_NUM_TRANSACTIONS) ? i + 1 : 0;
    } while (i != eeprom_queue_position);
    
    return NULL;
}


uint8_t eeprom_25lc1024_read(uint8_t *transaction_id, uint32_t address, uint8_t length, uint8_t *buffer)
{
    volatile eeprom_transaction_t *trans = get_next_free_transaction();
    if (trans == NULL) {
        return 1;
    }
    
    uint8_t next = ((eeprom_next_transaction + 1) == EEPROM_TRANSACTION_VOID) ? EEPROM_TRANSACTION_FIRST : eeprom_next_transaction + 1;
    
    trans->transaction_id = eeprom_next_transaction;
    *transaction_id = eeprom_next_transaction;
    eeprom_next_transaction = next;
    
    trans->state = QUEUED;
    
    trans->command = READ;
    trans->address = ((uint8_t*)&address) + 1;
    trans->data = buffer;
    trans->length = length;
    
    trans->write = 0;
    trans->send_addr = 1;
    trans->active = 0;
    trans->done = 0;
    trans->successfull = 1;
    
    eeprom_start_next_transaction();
    
    return 0;
}

uint8_t eeprom_25lc1024_write(uint8_t *transaction_id, uint32_t address, uint8_t length,  uint8_t *data)
{
    volatile eeprom_transaction_t *trans = get_next_free_transaction();
    if (trans == NULL) {
        return 1;
    }
    
    uint8_t next = ((eeprom_next_transaction + 1) == EEPROM_TRANSACTION_VOID) ? EEPROM_TRANSACTION_FIRST : eeprom_next_transaction + 1;
    
    trans->transaction_id = eeprom_next_transaction;
    *transaction_id = eeprom_next_transaction;
    eeprom_next_transaction = next;
    
    trans->state = QUEUED;
    
    trans->command = WRITE;
    trans->address = ((uint8_t*)&address) + 1;
    trans->data = data;
    trans->length = length;
    
    trans->write = 1;
    trans->send_addr = 1;
    trans->active = 0;
    trans->done = 0;
    trans->successfull = 1;
    
    eeprom_start_next_transaction();
    
    return 0;
}
