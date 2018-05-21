//
//  Radio.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Maxim Zakirov
//


#include "Radio.h"

#include "Radio_commands.h"
#include "SPI.h"
#include "pindefinitions.h"

#include <avr/io.h>


static uint8_t checksum_value; //literally the only global variable I need.

//TODO: Use the transaction ids that are in each of these.

void radio_service(void) {
    
    uint8_t transaction_id; //Dunno how these really work.
    uint8_t in_buffer[256];
    
    if(RADIO_ATTN_PORT & (1 << RADIO_ATTN_NUM)) {
        spi_start_full_duplex(&transaction_id, RADIO_CS_NUM, 0, 0, in_buffer, RADIO_ATTN_NUM);
        
        uint8_t *in_buffer_pointer;
        in_buffer_pointer = &in_buffer[0];
        
        if(in_buffer[3] == AT_COMMAND_RESPONSE) {
            at_command_response(in_buffer_pointer);
        } else if (in_buffer[3] == ZIGBEE_RECEIVE_PACKET) {
            zigbee_receive_packet(in_buffer_pointer);
        } else if (in_buffer[3] == MODEM_STATUS) {
            modem_status(in_buffer_pointer);
        } else if (in_buffer[3] == ZIGBEE_TRANSMIT_STATUS) {
            zigbee_transmit_status(in_buffer_pointer);
        } else if (in_buffer[3] == ZIGBEE_EXPLICIT_RX_INDICATOR) {
            zigbee_explicit_rx_indicator(in_buffer_pointer);
        } else if (in_buffer[3] == ZIGBEE_IO_DATA_SAMPLE_RX_INDICATOR) {
            zigbee_explicit_rx_indicator(in_buffer_pointer);
        } else if (in_buffer[3] == NODE_IDENTIFICATION_INDICATOR) {
            node_identification_indicator(in_buffer_pointer);
        } else if (in_buffer[3] == REMOTE_COMMAND_RESPONSE) {
            remote_command_response(in_buffer_pointer);
        } else if (in_buffer[3] == OVER_AIR_FIRMWARE_UPDATE_STATUS) {
            overairfirmwarestatus(in_buffer_pointer);
        } else if (in_buffer[3] == ROUTE_RECORD_INDICATOR) {
            route_record_indicator(in_buffer_pointer);
        } else if (in_buffer[3] == MANY_TO_ONE_ROUTE_REQUEST_INDICATOR) {
            many_to_one_route_request_indicator(in_buffer_pointer);
        }
    }
}

void at_command(uint8_t frame_id, uint8_t length_msb, uint8_t length_lsb, uint8_t command1, uint8_t command2, uint8_t parameter) {
    
    uint8_t transaction_id; //Dunno how these really work.
    uint8_t in_buffer[256];
    
    uint8_t api_packet[9] = {0x78, length_msb, length_lsb, AT_COMMAND, frame_id, command1, command2, parameter,0};
    uint8_t *api_pointer = &api_packet[0];
    checksum(api_pointer,9);
    api_packet[8] = checksum_value;
    
   // uint8_t checksum = 0xff - AT_COMMAND + frame_id + length_msb + length_lsb + command1 + command2 + parameter; old code
   //uint8_t api_packet[9] = {0x78, length_msb, length_lsb, AT_COMMAND, frame_id, command1, command2, parameter, checksum_value};
    
    spi_start_full_duplex(&transaction_id, RADIO_CS_NUM, api_packet,9, in_buffer, RADIO_ATTN_NUM);
    
    //while (!spi_transaction_done(transaction_id))
}


void at_queue_command(uint8_t frame_id, uint8_t length_msb, uint8_t length_lsb, uint8_t command1, uint8_t command2, uint8_t parameter) {
    
    uint8_t transaction_id; //Dunno how these really work.
    uint8_t in_buffer[256];
    
    uint8_t api_queue_packet[9] = {0x78, length_msb, length_lsb, AT_COMMAND, frame_id, command1, command2, parameter,0};
    uint8_t *api_pointer = &api_queue_packet[0];
    checksum(api_pointer,9);
    api_queue_packet[8] = checksum_value;
    
    // uint8_t checksum = 0xff - QUEUE_PARAMETER + frame_id + length_msb + length_lsb + command1 + command2 + parameter; old code
    //uint8_t api_queue_packet[9] = {0x78, length_msb, length_lsb, QUEUE_PARAMETER, frame_id, command1, command2, parameter, checksum};
    
    spi_start_full_duplex(&transaction_id, RADIO_CS_NUM, api_queue_packet,9, in_buffer, RADIO_ATTN_NUM);
    
}

void transmit_request(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t broadcast_radius, uint8_t frame_option, uint8_t *data, uint8_t data_size) {
    
    uint8_t transaction_id; //Dunno how these really work.
    uint8_t in_buffer[256];
    
    uint8_t api_prelim_transmit_packet[17] = {0x78, length_msb, length_lsb, TRANSMIT_REQUEST, frame_id, address_64_1, address_64_2, address_64_3, address_64_4, address_64_5, address_64_6, address_64_7, address_64_8, address_16_1, address_16_2, broadcast_radius, frame_option};
    uint8_t new_size = 18 + data_size; //extra one for checksum.
    uint8_t api_transmit_packet[new_size];
    
    for(int i = 0; i < 17; i++) {
        api_transmit_packet[i] = api_prelim_transmit_packet[i];
    }
    
    for(int i = 17; i < new_size; i++) {
        api_transmit_packet[i] = data[i-new_size];
    }
    
    uint8_t *api_transmit_packet_pointer = &api_transmit_packet[0]; //preliminary packet.
    checksum(api_transmit_packet_pointer, new_size);
    api_transmit_packet[new_size - 1] = checksum_value;

    /* take the api preliminary packet
       concatecate the given buffer of data by the user into the prelimnary packet
       add on the checksum
       send out the packet
     */
    
    spi_start_full_duplex(&transaction_id, RADIO_CS_NUM, api_transmit_packet, new_size, in_buffer, RADIO_ATTN_NUM);
    
}

void explicit_zigbee_transmit_request(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t source_end, uint8_t destination_end, uint8_t cluster_id, uint8_t profile_id, uint8_t broadcast_radius, uint8_t transmit_option, uint8_t *data, uint8_t data_size) {
    
    //uint8_t checksum = 0xff - ZIGBEE_EXPLICIT_RX_INDICATOR + frame_id + length_msb + length_lsb + address_16_1 + address_16_2 + address_64_1 + address_64_2 + address_64_3 + address_64_4 + address_64_5 + address_64_6 + address_64_7 + address_64_8 + + source_end + destination_end + cluster_id + profile_id + broadcast_radius + transmit_option;
    
    uint8_t transaction_id; //Dunno how these really work.
    uint8_t in_buffer[256];
    
    uint8_t api_prelim_explicit_transmit_packet[21] = {0x78, length_msb, length_lsb, ZIGBEE_EXPLICIT_RX_INDICATOR, frame_id, address_64_1, address_64_2, address_64_3, address_64_4, address_64_5, address_64_6, address_64_7, address_64_8, address_16_1, address_16_2, source_end, destination_end, cluster_id, profile_id, broadcast_radius, transmit_option};
    uint8_t new_size = 22 + data_size; //extra one for checksum.
    uint8_t api_explicit_transmit_packet[new_size];
    
    for(int i = 0; i < 21; i++) {
        api_explicit_transmit_packet[i] = api_prelim_explicit_transmit_packet[i];
    }
    
    for(int i = 21; i < new_size; i++) {
        api_explicit_transmit_packet[i] = data[i-new_size];
    }
    
    uint8_t *api_explicit_transmit_packet_pointer = &api_explicit_transmit_packet[0]; //preliminary packet.
    checksum(api_explicit_transmit_packet_pointer, new_size);
    api_explicit_transmit_packet[new_size - 1] = checksum_value;
    
    spi_start_full_duplex(&transaction_id, RADIO_CS_NUM, api_explicit_transmit_packet, new_size, in_buffer, RADIO_ATTN_NUM);

}

void create_source_route(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t address_amount, uint8_t address_1_msb, uint8_t address_1_lsb, uint8_t address_2_msb, uint8_t address_2_lsb, uint8_t address_3_msb, uint8_t address_3_lsb) {

    uint8_t transaction_id; //Dunno how these really work.
    uint8_t in_buffer[256];
    
    uint8_t checksum = 0xff - CREATE_SOURCE_ROUTE + frame_id + length_msb + length_lsb + address_16_1 + address_16_2 + address_64_1 + address_64_2 + address_64_3 + address_64_4 + address_64_5 + address_64_6 + address_64_7 + address_64_8 + address_amount + address_1_msb + address_1_lsb + address_2_msb + address_2_lsb + address_3_msb + address_3_lsb;
    uint8_t source_route_packet[23] = {0x78, length_msb, length_lsb, CREATE_SOURCE_ROUTE, frame_id, address_64_1, address_64_2, address_64_3, address_64_4, address_64_5, address_64_6, address_64_7, address_64_8, address_16_1, address_16_2, address_amount, address_1_msb, address_1_lsb, address_2_msb, address_2_lsb, address_3_msb, address_3_lsb, checksum};
    
    spi_start_full_duplex(&transaction_id, RADIO_CS_NUM, source_route_packet, 23, in_buffer, RADIO_ATTN_NUM);
}


void remote_at_command_request(uint8_t length_msb, uint8_t length_lsb, uint8_t frame_id, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t remote_options, uint8_t command1, uint8_t command2, uint8_t parameter) {

    uint8_t transaction_id; //Dunno how these really work.
    uint8_t in_buffer[256];
    
    uint8_t checksum = 0xff - REMOTE_COMMAND_REQEUEST + frame_id + address_64_1 + address_64_2 + address_64_3 + address_64_4 + address_64_5 + address_64_6 + address_64_7 + address_64_8 + address_16_1 + address_16_2 + remote_options + command1 + command2 + parameter;
    
    uint8_t remote_at_command_packet[21] = {0x78, length_msb, length_lsb, REMOTE_COMMAND_REQEUEST, frame_id, address_64_1, address_64_2, address_64_3, address_64_4, address_64_5, address_64_6, address_64_7, address_64_8, address_16_1, address_16_2, remote_options, command1, command2, parameter, checksum};
    
    spi_start_full_duplex(&transaction_id, RADIO_CS_NUM, remote_at_command_packet, 21, in_buffer, RADIO_ATTN_NUM);

}


void checksum(uint8_t *array, uint8_t size) {
    checksum_value = 0;
    for(int i = 3; i < size; i++) {
        checksum_value = checksum_value + array[i];
    }
    checksum_value = 0xff - checksum_value;
}



void at_command_response(uint8_t *in_buffer) {
    //2 ASCII CHARACTERS COME BACK.
}

void modem_status(uint8_t *in_buffer) {
}

void zigbee_transmit_status(uint8_t *in_buffer) {
}

void zigbee_receive_packet(uint8_t *in_buffer) {
}

void zigbee_explicit_rx_indicator(uint8_t *in_buffer) {
}

void zigbee_io_data_sample_rx_indicator(uint8_t *in_buffer) {
}

void node_identification_indicator(uint8_t *in_buffer) {
}

void remote_command_response(uint8_t *in_buffer) {
}

void route_record_indicator(uint8_t *in_buffer) {
}

void many_to_one_route_request_indicator(uint8_t *in_buffer) {
}

void overairfirmwarestatus(uint8_t *in_buffer) {
}





