//
//  XBee.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "XBee.h"

#include "Radio_commands.h"
#include "pindefinitions.h"
#include "SPI.h"

#include <avr/io.h>
#include <string.h>

//The address configuring, initializing, and also the routing will be done in this.

#define QUEUE_LENGTH    4  // The number of SPI transactions which can be queued

#define ID_INVALID      0   // The transaction ID for an unused transaction
#define ID_FIRST        1   // The first valid transaction ID

typedef struct {
    /** A unique identifer for this transaction */
    uint8_t id;
    
    /** data that will be sent */
    uint8_t buffer[128];
    
    uint8_t length;
    uint8_t spi_transaction_id;
    
    /** The type for this transaction */
    uint8_t type:3; //3 bits long
    
    /** 1 if this transaction is currently in progress*/
    uint8_t active:1; //fancy bitcompact stuff
    
    uint8_t read:1;
    /** 1 if this transaction is complete */
    uint8_t done:1;
    
} xbee_transaction_t;

/** The transaction queue */
static xbee_transaction_t queue[QUEUE_LENGTH];
/** The index of the head of the transaction queue */
static uint8_t queue_head;

/** The transaction id that should be given to the next new transaction */
static uint8_t next_id = ID_FIRST;

/** The buffer used to received data from the module */
static uint8_t in_buffer[256];


void init_xbee(void)
{
    
}

static inline void start_next_transaction (void) {
    if (queue[queue_head].active) return;
    
    uint8_t i = queue_head;
    do {
        if ((queue[i].id != ID_INVALID) && !queue[i].active && !queue[i].done) {
            queue_head = i;
            // Start transaction
            
            in_buffer[0] = 0;
            queue[i].active = 1;
            spi_start_full_duplex(&queue[i].spi_transaction_id, RADIO_CS_NUM, queue[i].buffer, queue[i].length, in_buffer, RADIO_ATTN_NUM);
            return;
        }
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
}

static uint8_t has_queued_transaction (void)
{
    
    uint8_t i = queue_head;
    do {
        if ((queue[i].id != ID_INVALID) ) {
            
            return 1;
        }
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
    return 0;
}

/**
 *  Get the first transaction in the queue with the given ID
 *  @param id The transaction ID to search for
 */
static xbee_transaction_t *get_transaction_with_id(uint8_t id)
{
    for (xbee_transaction_t *i = queue; i < queue + QUEUE_LENGTH; i++) {
        if (i->id == id) return i;
    }
    return NULL;
}

uint8_t xbee_transaction_done(uint8_t transaction_id)
{
    xbee_transaction_t *t = get_transaction_with_id(transaction_id);
    return (t != NULL) ? (t->done) : 1;
}

uint8_t xbee_clear_transaction(uint8_t transaction_id)
{
    xbee_transaction_t *t = get_transaction_with_id(transaction_id);
    if ((t != NULL) && !(t->active)) {
        t->id = ID_INVALID;
        return 0;
    }
    return 1;
}

/**
 *  Get the next transaction slot which is not currently in use
 */
static xbee_transaction_t *get_next_free_transaction(void)
{
    uint8_t i = queue_head;
    do {
        if (queue[i].id == ID_INVALID) return queue + i;
        i = (i + 1) % QUEUE_LENGTH;
    } while (i != queue_head);
    return NULL;
}

static uint8_t radio_receive(uint8_t *transaction_id) { //endians
    xbee_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->read = 1;
    t->length = 0;
    t->active = 0;
    t->done = 0;
    
    start_next_transaction();
    
    
    return 0;
}

void xbee_service(void) {
    
    if(RADIO_ATTN_PORT & (1 << RADIO_ATTN_NUM) && !(has_queued_transaction()) ) {
        uint8_t id;
        radio_receive(&id);
    }
    
    xbee_transaction_t *t = queue + queue_head; //pointer to transaction
    if( (t->id == 0) || !spi_transaction_done(t->spi_transaction_id)) return;
    spi_clear_transaction(t->spi_transaction_id);
    
    t->done = 1; //transaction is done
    t->active = 0;
    
    if (t->read) {
        // This is an internally created read transaction, it should be freed
        t->id = ID_INVALID;
    }
    
    //TODO: Panic when you receive something!
    
    queue_head = (queue_head + 1) % QUEUE_LENGTH; //advanced queue
    
    start_next_transaction();
}

static uint8_t calculate_checksum (uint8_t *array, uint8_t size) {
    uint8_t checksum_value = 0;
    for(uint8_t i = 0; i < size; i++) {
        checksum_value += array[i];
    }
    return 0xff - checksum_value;
}

uint8_t xbee_send_at_command(uint8_t *transaction_id, uint8_t get_response, uint8_t *command, uint8_t has_parameter, uint8_t parameter) {
    
    xbee_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->buffer[0] = 0x7E;
    t->buffer[1] = 0;
    t->buffer[2] = (has_parameter) ? 5 : 4;
    t->buffer[3] = AT_COMMAND;
    t->buffer[4] = (get_response) ? 1 : 0;
    t->buffer[5] = command[0];
    t->buffer[6] = command[1];
    t->buffer[7] = parameter;
    
    t->buffer[8] = calculate_checksum(t->buffer + 3, 8);
    
    t->read = 0;
    t->length = 9;
    t->active = 0;
    t->done = 0;
    
    start_next_transaction();
    
    
    return 0;
}

uint8_t xbee_send_at_queue_command(uint8_t *transaction_id, uint8_t get_response, uint8_t *command, uint8_t has_parameter, uint8_t parameter) {
    
    xbee_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;
    
    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    t->buffer[0] = 0x7E;
    t->buffer[1] = 0;
    t->buffer[2] = (has_parameter) ? 5 : 4;
    t->buffer[3] = QUEUE_PARAMETER;
    t->buffer[4] = (get_response) ? 1 : 0;
    t->buffer[5] = command[0];
    t->buffer[6] = command[1];
    t->buffer[7] = parameter;
    
    t->buffer[8] = calculate_checksum(t->buffer + 3, 8);
    
    t->read = 0;
    t->length = 9;
    t->active = 0;
    t->done = 0;
    
    start_next_transaction();
    
    
    return 0;
}


uint8_t xbee_transmit_command(uint8_t *transaction_id, uint8_t get_response, uint64_t address_64, uint16_t address_16, uint8_t broadcast_radius, uint8_t transmit_options, uint8_t *data, uint8_t data_size) {
    
    xbee_transaction_t *t = get_next_free_transaction();
    if (t == NULL) return 1;

    t->id = next_id;
    *transaction_id = next_id++;
    if (next_id == ID_INVALID) next_id = ID_FIRST;
    
    uint8_t data_length = (data_size < 110) ? data_size : 110;
    t->buffer[0] = 0x7E;
    t->buffer[1] = 0;
    t->buffer[2] = 14 + data_length;
    t->buffer[3] = TRANSMIT_REQUEST;
    t->buffer[4] = (get_response) ? 1 : 0;
    uint8_t *addr_64_bytes = (uint8_t*)(&address_64);
    t->buffer[5] = addr_64_bytes[7];
    t->buffer[6] = addr_64_bytes[6];
    t->buffer[7] = addr_64_bytes[5];
    t->buffer[8] = addr_64_bytes[4];
    t->buffer[9] = addr_64_bytes[3];
    t->buffer[10] = addr_64_bytes[2];
    t->buffer[11] = addr_64_bytes[1];
    t->buffer[12] = addr_64_bytes[0];
    uint8_t *addr_16_bytes = (uint8_t*)(&address_16);
    t->buffer[13] = addr_16_bytes[1];
    t->buffer[14] = addr_16_bytes[0];
    t->buffer[15] = broadcast_radius;
    t->buffer[16] = transmit_options;
    memcpy(t->buffer + 17, data, data_length);
    t->buffer[17 + data_length] = calculate_checksum(t->buffer + 3, data_length + 14);
    
    t->read = 0;
    t->length = 18 + data_length;
    t->active = 0;
    t->done = 0;
    
    start_next_transaction();
    
    
    return 0;
}

