//
//  XBee.h
//  CU-in-Space-2018-Avionics-Software
//
//  Abstract over the XBee API
//

#ifndef XBee_h
#define XBee_h


extern void init_xbee(void);
/*
 * xbee radio initialization sequence.
 */

extern void create_network(void);
/*
 * Creates the point to multipoint network with the groundstation through this.
 */







#endif /* XBee_h */
