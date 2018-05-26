//
//  XBee.h
//  CU-in-Space-2018-Avionics-Software
//
//  Abstract over the XBee API
//

#ifndef XBee_h
#define XBee_h
#include "global.h"

struct Transmit_Options {
    
    uint8_t disable_ack:1;
    uint8_t disable_route_discovery:1;
    uint8_t enable_unicast_NACK_messages:1;
    uint8_t enable_unicast_trace_route_messages:1;
    uint8_t unused:2;
    uint8_t sending_mode:2;
};

extern void init_xbee(void);
/*
 * xbee radio initialization sequence.
 */

extern void create_network(void);
/*
 * Creates the point to multipoint network with the groundstation through this.
 */

extern void io_setup(void);
/*
 * sets the i/o and other things to input or an output.
 */
extern void xbee_router_setup(void);



#endif /* XBee_h */
