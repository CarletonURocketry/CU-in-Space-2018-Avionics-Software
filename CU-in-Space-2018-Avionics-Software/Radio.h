//
//  Radio.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Maxim Zakirov on 2018-04-10.
//

#ifndef Radio_h
#define Radio_h

#include "global.h"


/* TODO: Some response's payload are in ASCII format, like "NI" (Node Identifier), some in Integer format, like "SL" (lower 32 bits of the XBee address). Um....fix that too. */

extern void radio_service(void);
/*
 * This function will need to be continuously checked in the event the radio decides to send data back to the microcontroller.
 */

/* TODO: actually implement this properly */
extern uint8_t radio_clear_transaction(uint8_t transaction_id);
/*
 *
 */

extern uint8_t radio_done_transaction(uint8_t transaction_id);
/*
 *
 */


extern void at_command(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t command1, uint8_t command2, uint8_t parameter);
/*
 * Sends command to the xbee.
 */

extern void at_queue_command(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t command1, uint8_t command2, uint8_t parameter);
/*
 * It will queue a bunch of parameters in the xbee until you send an apply changes command (AC).
 */

extern void transmit_request(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t broadcast_radius, uint8_t frame_option, uint8_t *data, uint8_t data_size);
/*
 * I will probably need to talk to whoever is doing the groundstation stuff about what address they'll be using for this.
 * for now, my xbee will be the coordinator at the address 0x0000000000000000 and they can be something else.
 * for frame options, there's 0x01 = disable retries and router repair, 0x20 enabling encryption, and 0x40 = extended
 * transmission timeout. 0x00 is for nothing.
 * The 16 bit address is the network address.
 */

extern void remote_at_command_request(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t remote_options, uint8_t command1, uint8_t command2, uint8_t parameter);
/*
 * You can specify parameters of transmission and other settings to set in another device other than the coordinator.
 * Need to send an apply changes command for it to take effect.
 */

extern void explicit_zigbee_transmit_request(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t source_end, uint8_t destination_end, uint8_t cluster_id, uint8_t profile_id, uint8_t broadcast_radius, uint8_t transmit_option, uint8_t *data, uint8_t data_size);
/*
 * You can specify cluster ids and specific destinations and routes to manually send data.
 * Cluster ids and commands for this functions are shown in the datasheet.
 */

extern void create_source_route(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t address_amount, uint8_t address_1_msb, uint8_t address_1_lsb, uint8_t address_2_msb, uint8_t address_2_lsb, uint8_t address_3_msb, uint8_t address_3_lsb);
/*
 * Allows you to specify a specific route by which you can send the data. Not useful when there's only one coordinator and one
 * endpoint.
 */

extern void at_command_response(uint8_t *in_buffer);
/* Once the transmission and processing of the command has been completed, the result will be sent back to the microcontroller
 * via an 0x88 frame.
 */

void checksum(uint8_t *array, uint8_t size);
/*
 * Calculates the checksum of functions that have variable amounts of data in them.
 */

extern void modem_status(uint8_t *in_buffer);
/*
 * Sends a status value of how the network setup went after it was created.
 */

extern void zigbee_transmit_status(uint8_t *in_buffer);
/*
 * Returns a status value for the transmission of data.
 */

extern void zigbee_receive_packet(uint8_t *in_buffer);
/*
 * A mini status report of what RF data was sent and what the transmission status was
 */

extern void zigbee_explicit_rx_indicator(uint8_t *in_buffer);
/*
 * Shows clusters, endpoints, rf data sent, receive options, and all the addresses of the sent data.
 */

extern void zigbee_io_data_sample_rx_indicator(uint8_t *in_buffer);
/*
 * If sensor data is sent to the xbee from another device, it sends a sample of the data that you can view with this.
 */

//extern void xbee_sensor_read_indicator(void);
/*
 * Won't work because we don't have a 1 wire digi adapter device.
 */

extern void node_identification_indicator(uint8_t *in_buffer);
/*
 * A remote device or any other device can send this as an identifier to make the other device recognize who is the sender.
 */

extern void remote_command_response(uint8_t *in_buffer);
/*
 * Basically sends a carbon copy of the command you sent to the remote device.
 */

extern void route_record_indicator(uint8_t *in_buffer);
/*
 * This is received whenever a device sends a ZigBee route record command.
 */

extern void many_to_one_route_request_indicator(uint8_t *in_buffer);
/*
 * Sent out when a many-to-one route request is received.
 */

extern void overairfirmwarestatus(uint8_t *in_buffer);
/*
 * Provides a status indication of a firmware update transmission attempt.
 */

#endif /* Radio_h */
