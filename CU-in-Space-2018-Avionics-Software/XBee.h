//
//  XBee.h
//  CU-in-Space-2018-Avionics-Software
//
//  Abstract over the XBee API
//

#ifndef XBee_h
#define XBee_h
#include "global.h"

#define XBEE_ADDRESS_64_BROADCAST   0x000000000000FFFF
#define XBEE_ADDRESS_16_UNKOWN      0xFFFE

struct Transmit_Options {
    
    uint8_t disable_ack:1;
    uint8_t disable_route_discovery:1;
    uint8_t enable_unicast_NACK_messages:1;
    uint8_t enable_unicast_trace_route_messages:1;
    uint8_t unused:2;
    uint8_t sending_mode:2;
};

/**
 *  Initilize the XBee radio
 */
extern void init_xbee(void);

/**
 *  The XBee code to be run in each iteration of the main loop
 */
extern void xbee_service(void);

extern uint8_t xbee_transaction_done(uint8_t transaction_id);
extern uint8_t xbee_clear_transaction(uint8_t transaction_id);

/**
 *  Querry or set module parameters
 *  @param transaction_id Memory where the unique identifier for this transaction should be stored
 *  @param get_response Whether or not the module should be asked for a response
 *  @param command A two char array containing the mnemonic for the command
 *  @param has_parameter Whether or not a parameter should be sent with the command
 *  @param parameter The paramater to send with the command
 *  @return A non-zero value if the command was successfully queued
 */
extern uint8_t xbee_send_at_command(uint8_t *transaction_id, uint8_t get_response, uint8_t *command, uint8_t has_parameter, uint8_t parameter);

/**
 *  Querry or set module parameters without applying immediatly
 *  @param transaction_id Memory where the unique identifier for this transaction should be stored
 *  @param get_response Whether or not the module should be asked for a response
 *  @param command A two char array containing the mnemonic for the command
 *  @param has_parameter Whether or not a parameter should be sent with the command
 *  @param parameter The paramater to send with the command
 *  @return A non-zero value if the command was successfully queued
 */
extern uint8_t xbee_send_at_queue_command(uint8_t *transaction_id, uint8_t get_response, uint8_t *command, uint8_t has_parameter, uint8_t parameter);

/**
 *  Transmit data
 *  @param transaction_id Memory where the unique identifier for this transaction should be stored
 *  @param get_response Whether or not the module should be asked for a response
 *  @param address_64 The 64 bit destination address
 *  @param address_16 The 16 bit destination address
 *  @param broadcast_radius The maximum number of hops for broadcast transmitions
 *  @param transmit_options The options for this transmition
 *  @param data The data to be transmitted
 *  @param data_size The number of bytes to be transmitted
 *  
 */
extern uint8_t xbee_transmit_command(uint8_t *transaction_id, uint8_t get_response, uint64_t address_64, uint16_t address_16, uint8_t broadcast_radius, uint8_t transmit_options, uint8_t *data, uint8_t data_size);


#endif /* XBee_h */
