//
//  I2C-Example.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2018-01-18.
//
//  An Example which demonstrates the usage of the I2C abstraction
//

//#define INCLUDE_I2C_TEST
#ifdef INCLUDE_I2C_TEST

#include "I2C.h"

typedef enum {SENT_INIT, READING_DATA, IDLE} sensor_state;

#define DEVICE_ADDRESS  0x08
#define INIT_REGISTER   0x01
#define DATA_REGISTER   0xa5

static uint8_t buffer[10];
static uint8_t init_data;
static uint8_t transaction_id;

static sensor_state state;

void init (void)
{
    // Generally there is some initilization work which must be done including config register which must be written
    // Ex. for the altimiter we will need to read the presure at ground level and then write it back to a register
    // The best way to do this is an FSM, you would start the first transaction in this function and then have the rest of the
    // FSM in the service
    
    init_data |= (1<<7) | (1<<3);
    
    i2c_write(&transaction_id,  // The ID we will use to get this transaction back later
              DEVICE_ADDRESS,   // The I2C address of the device with which we want to communicate
              INIT_REGISTER,    // The register that we are writing to
              &init_data,       // A pointer to the data we want to send
              1                 // The number of bytes we want to send
              );
    
    state = SENT_INIT;
}

void service (void)
{
    // This is where most of the code will be
    
    switch (state) {
    case SENT_INIT:
        // init sent, check if it's done and move on
        if (i2c_transaction_done(transaction_id)) {
            // Move on, in this example we are going to read a register
            
            // Always clear finished transaction as the queue has a fixed size and would fill up otherwise
            i2c_clear_transaction(transaction_id);
            
            
            i2c_read(&transaction_id,   // The ID we will use to get this transaction back later
                     DEVICE_ADDRESS,    // The I2C address of the device with which we want to communicate
                     DATA_REGISTER,     // The register that we are writing to
                     buffer,            // A pointer where the data we receive will be placed
                     10                 // The number of bytes we want to read
                     );
            
            state = READING_DATA;
        } else {
            // Try again next loop (no need to actually have an else block)
        }
        break;
    case READING_DATA:
        if (i2c_transaction_done(transaction_id)) {
            // All done (for now at least)
            
            // Always clear finished transaction as the queue has a fixed size and would fill up otherwise
            i2c_clear_transaction(transaction_id);
            
            // Our data is now in the array buffer, we would process it here
            
            state = IDLE;
        }  else {
            // Try again next loop (no need to actually have an else block)
        }
        break;
    default:
            break;
    }
}


#endif /* INCLUDE_I2C_TEST */
