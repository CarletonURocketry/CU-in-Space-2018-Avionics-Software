//
//  Radio.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Maxim Zakirov on 2018-04-10.
//

#ifndef Radio_h
#define Radio_h

#include "global.h"


extern void at_command(uint8_t frame_id, uint8_t length_msb, uint8_t length_lsb, uint8_t command1, uint8_t command2, uint8_t parameter);
/*
 * Sends command to the xbee.
 */

extern void at_queue_command(uint8_t frame_id, uint8_t length_msb, uint8_t length_lsb, uint8_t command1, uint8_t command2, uint8_t parameter);
/*
 * it will queue a bunch of parameters in the xbee until you send an apply changes command (AC).
 */


extern void transmit_request(uint8_t frame_id, uint8_t length_msb, uint8_t length_lsb, uint8_t address_64_1, uint8_t address_64_2, uint8_t address_64_3, uint8_t address_64_4, uint8_t address_64_5, uint8_t address_64_6, uint8_t address_64_7, uint8_t address_64_8, uint8_t address_16_1, uint8_t address_16_2, uint8_t broadcast_radius, uint8_t frame_option, uint8_t rfdata1, uint8_t rfdata2, uint8_t rfdata3, uint8_t rfdata4, uint8_t rfdata5, uint8_t rfdata6, uint8_t rfdata7, uint8_t rfdata8);
/*
 * I will probably need to talk to whoever is doing the groundstation stuff about what address they'll be using for this.
 * for now, my xbee will be the coordinator at the address 0x0000000000000000 and they can be something else.
 * for frame options, there's 0x01 = disable retries and router repair, 0x20 enabling encryption, and 0x40 = extended
 * transmission timeout. 0x00 is for nothing.
 */




#endif /* Radio_h */
