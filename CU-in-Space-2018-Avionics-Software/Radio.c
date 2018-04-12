//
//  Radio.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Maxim Zakirov
//

//Different ways to interface with the radio. Very basic start

#include "Radio.h"
#include "Radio_commands.h"
#include "SPI.h"
#include "pindefinitions.h"

#include <avr/io.h>
#include <string.h>

void at_command(uint8_t frame_id, uint8_t length_msb, uint8_t length_lsb, uint8_t command1, uint8_t command2, uint8_t parameter) {
    
    uint8_t checksum = 0xff - AT_COMMAND + frame_id + length_msb + length_lsb + command1 + command2 + parameter;
    uint8_t transaction_id;
    uint8_t api_packet[9] = {0x78, length_msb, length_lsb, frame_id, AT_COMMAND, command1, command2, parameter, checksum};
    uint8_t in_buffer[256];
    
    spi_start_half_duplex(&transaction_id, RADIO_CS_NUM, api_packet,9 , in_buffer, RADIO_ATTN_NUM);
    
    //while (!spi_transaction_done(transaction_id))
}

void at_queue_command(uint8_t frame_id, uint8_t length_msb, uint8_t length_lsb, uint8_t command1, uint8_t command2, uint8_t parameter) {
    
    uint8_t checksum = 0xff - QUEUE_PARAMETER + frame_id + length_msb + length_lsb + command1 + command2 + parameter;
    uint8_t transaction_id;
    uint8_t api_queue_packet[9] = {0x78, length_msb, length_lsb, QUEUE_PARAMETER, frame_id, command1, command2, parameter, checksum};
    uint8_t in_buffer[256];
    
    spi_start_half_duplex(&transaction_id, RADIO_CS_NUM, api_queue_packet,9, in_buffer, RADIO_ATTN_NUM);
    
}

void transmit_request(uint8_t frame_id, uint8_t length_msb, uint8_t length_lsb, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t broadcast_radius, uint8_t frame_option, uint8_t rfdata1, uint8_t rfdata2, uint8_t rfdata3, uint8_t rfdata4, uint8_t rfdata5, uint8_t rfdata6, uint8_t rfdata7, uint8_t rfdata8) {
    
    uint8_t checksum = 0xff - TRANSMIT_REQUEST + frame_id + length_msb + length_lsb + address_16_1 + address_16_2 + address_64_1 + address_16_2 + address_64_3 + address_64_4 + address_64_5 + address_64_6 + address_64_7 + address_64_8 + broadcast_radius + frame_option + rfdata1 + rfdata2 + rfdata3 + rfdata4 + rfdata5 + rfdata6 + rfdata7 + rfdata8;
    uint8_t transaction_id;
    uint8_t api_transmit_packet[26] = {0x78, length_msb, length_lsb, TRANSMIT_REQUEST, frame_id, address_64_1, address_64_2, address_64_3, address_64_4, address_64_5, address_64_6, address_64_7, address_64_8, address_16_1, address_16_2, broadcast_radius, frame_option, rfdata1, rfdata2, rfdata3, rfdata4, rfdata5, rfdata6, rfdata7, rfdata8, checksum};
    uint8_t in_buffer[256];
    
    spi_start_half_duplex(&transaction_id, RADIO_CS_NUM, api_transmit_packet,26, in_buffer, RADIO_ATTN_NUM);
    
}

void xbee_service(void) {
    
    uint8_t transaction_id;
    uint8_t in_buffer[256];
    
    if(RADIO_ATTN_PORT & (1 << RADIO_ATTN_NUM)) {
        spi_start_full_duplex(&transaction_id, RADIO_CS_NUM, NULL,0, in_buffer, RADIO_ATTN_NUM);
        
    }
}

