//
//  I2C.c
//  CU-in-Space-2018-Avionics-Software
//
//

#include "I2C.h"

#include <stddef.h> //NULL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

#include "pindefinitions.h"

// MARK: Constants
#define I2C_MAX_ERRORS  3   // Number of errors before an I2C transaction is aborted

#define QUEUE_LENGTH    10  // The number of SPI transactions which can be queued

#define ID_INVALID      0   // The transaction ID for an unused transaction
#define ID_FIRST        1   // The first valid transaction ID

// MARK: Type Definitions
typedef struct {
    /** A unique identifer for this transaction */
    uint8_t id;
    
    /** The address of the peripheral to be accessed*/
    uint8_t address;
    /** The address of the register within the peripheral to be accessed*/
    uint8_t reg;
    
    /** The number of bytes to be read or writen*/
    uint8_t length;
    /** The memory where bytes to be written come from or bytes read are stored*/
    uint8_t *buffer;
    
    /** The number of bytes that have been read or writen*/
    uint8_t position;
    /** The number of retries which have occured*/
    uint8_t errors;
    
    /** 1 if this is a write transaction*/
    uint8_t write:1;
    /** 1 if this transaction is currently in progress*/
    uint8_t active:1;
    /** 1 if address and register have been sent succesfully*/
    uint8_t done_reg:1;
    /** 1 if this transaction is complete */
    uint8_t done: 1;
} i2c_transaction_t;

// MARK: Variable Definitions
/** The transaction queue */
static volatile i2c_transaction_t queue[QUEUE_LENGTH];
/** The index of the head of the transaction queue */
static volatile uint8_t queue_head;

/** The transaction id that should be given to the next new transaction */
static uint8_t next_id = ID_FIRST;

// MARK: Function Definitions
void init_i2c(void)
{
    TWBR = 7;           // Set the TWI baud rate to 400kHz
    TWDR = 0xFF;        // Default content (SDA released)
    TWSR = (1<<TWEN);   // Enable TWI and take control of the SDA and SCL pins
}

/**
 *  Starts the next queued transaction if there is one and there is no currently active transaction.
 *  This function is inline so that is can be safely called from an ISR
 */
static inline void start_next_transaction (void) {
    if (queue[queue_head].active) return;
    
    uint8_t i = queue_head;
    do {
        if ((queue[i].id != ID_INVALID) && !queue[i].active && !queue[i].done) {
            queue_head = i;
            // Start transaction
            queue[i].active = 1;
            // Send a start condition
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWSTA); // Enable TWI with interupt, clear interupt and send start
            return;
        }
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
}

void i2c_service(void)
{
    start_next_transaction();
}

/**
 *  Get the first transaction in the queue with the given ID
 *  @param id The transaction ID to search for
 */
static volatile i2c_transaction_t *get_transaction_with_id(uint8_t id)
{
    for (volatile i2c_transaction_t *i = queue; i < queue + QUEUE_LENGTH; i++) {
        if (i->id == id) return i;
    }
    return NULL;
}

uint8_t i2c_transaction_done(uint8_t transaction_id)
{
    volatile i2c_transaction_t *t = get_transaction_with_id(transaction_id);
    return (t != NULL) ? (t->done) : 1;
}

uint8_t i2c_transaction_successful(uint8_t transaction_id)
{
    volatile i2c_transaction_t *t = get_transaction_with_id(transaction_id);
    return (t != NULL) ? (t->errors < I2C_MAX_ERRORS) : 1;
}

uint8_t i2c_clear_transaction(uint8_t transaction_id)
{
    volatile i2c_transaction_t *t = get_transaction_with_id(transaction_id);
    if ((t != NULL) && !(t->active)) {
        t->id = ID_INVALID;
        return 0;
    }
    return 1;
}

/**
 *  Get the next transaction slot which is not currently in use
 */
static volatile i2c_transaction_t *get_next_free_transaction(void)
{
    uint8_t i = queue_head;
    do {
        if (queue[i].id == ID_INVALID) return queue + i;
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
    return NULL;
}

uint8_t i2c_write(uint8_t *transaction_id, uint8_t address, uint8_t reg, uint8_t *data, uint8_t length)
{
    volatile i2c_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->address = address << 1;
    t->reg = reg;
    
    t->length = length;
    t->buffer = data;
    
    t->position = 0;
    t->errors = 0;
    
    t->write = 1;
    t->active = 0;
    t->done_reg = 0;
    t->done = 0;
    
    i2c_service();
    return 0;
}

uint8_t i2c_read(uint8_t *transaction_id, uint8_t address, uint8_t reg, uint8_t *data, uint8_t length)
{
    volatile i2c_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->address = address << 1;
    t->reg = reg;
    
    t->length = length;
    t->buffer = data;
    
    t->position = 0;
    t->errors = 0;
    
    t->write = 0;
    t->active = 0;
    t->done_reg = 0;
    t->done = 0;
    
    i2c_service();
    return 0;
}

// MARK: Interupt service routines
ISR (TWI_vect)
{
    volatile i2c_transaction_t *t = queue + queue_head;
    
    switch (TW_STATUS) {
        case TW_NO_INFO:        // 0xF8 -> No state information avaliable
            // This should not happen
            break;
        case TW_START:          // 0x08 -> START condition has been transmitted
            // Need to send SLA+W (handled same as repeated START)
        case TW_REP_START:      // 0x10 -> Repeated START condition has been transmitted
            // Need to send SLA+R if address and register have been sent, otherwise send SLA+W
            TWDR = (t->done_reg) ? (t->address | TW_READ) : (t->address | TW_WRITE);
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TW_MT_SLA_ACK:     // 0x18 -> SLA+W has been transmitted, ACK recieved
            // Need to send register address
            TWDR = t->reg;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TW_MT_SLA_NACK:    // 0x20 -> SLA+W has been transmitted, NOT ACK recieved
            goto error;
        case TW_MT_DATA_ACK:    // 0x28 -> Data byte has been transmitted, ACK recieved
            // Send repeated start if this is a read, continue sending data if this is a write and there is data to be sent
            t->done_reg = 1;
            if (t->write && t->position < t->length) {
                // Write, need to send more bytes
                TWDR = t->buffer[t->position++];
                TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            } else if (t->write) {
                // Write, finished
                goto wrap_up;
            } else {
                // Read, need to send repeated start
                TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
            }
            break;
        case TW_MT_DATA_NACK:   // 0x30 -> Data byte has been transmitted, NOT ACK recieved
            goto error;
        case TW_MT_ARB_LOST:    // 0x38 -> Arbitration lost
            // Increment error count and try again by sending START condition when bus is avaliable or bail out as appropriate
            // This should not happen as there is only one master on the bus
            goto error;
        case TW_MR_SLA_ACK:     // 0x40 -> SLA+R has been transmitted, ACK recieved
            // Select whether next response will be ACK or NAT ACK
            TWCR = (1<<TWINT) | ((t->length != 1) ? (1<<TWEA) : 0) | (1<<TWEN) | (1<<TWIE);
            break;
        case TW_MR_SLA_NACK:    // 0x48 -> SLA+R has been transmitted, NOT ACK recieved
            goto error;
        case TW_MR_DATA_ACK:    // 0x50 -> Data byte recieved, ACK returned
            // Read data byte and select whether next response will be ACK or NAT ACK
            t->buffer[t->position++] = TWDR;
            TWCR = (1<<TWINT) | ((t->position < (t->length - 1)) ? (1<<TWEA) : 0) | (1<<TWEN) | (1<<TWIE);
            break;
        case TW_MR_DATA_NACK:   // 0x58 -> Data byte recieved, NOT ACK returned
            // Read last data byte, send stop condition, wrap-up transaction and start next transaction if there is one
            t->buffer[t->position++] = TWDR;
            goto wrap_up;
        default:
            // This should not happen
            break;
    }
    return;
error:
    // Increment error count and try again by sending STOP followed by START condition or bail out as appropriate
    if (++t->errors < I2C_MAX_ERRORS) {
        t->done_reg = 0;
        t->position = 0;
        TWCR = (1<<TWINT) | (1<<TWSTA) | ((TW_STATUS == TW_MT_ARB_LOST) ? (1<<TWSTO) : 0) | (1<<TWEN) | (1<<TWIE);
        return;
    }
wrap_up:
    // Send stop condition
    if (TW_STATUS != TW_MR_ARB_LOST) TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
    // Set flags and start next transaction if applicable
    t->done = 1;
    t->active = 0;
    queue_head = (queue_head + 1) % QUEUE_LENGTH;
    start_next_transaction();
}
