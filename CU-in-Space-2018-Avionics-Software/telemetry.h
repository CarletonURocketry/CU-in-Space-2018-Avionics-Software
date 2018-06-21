//
//  telemetry.h
//  CU-in-Space-2018-Avionics-Software
//
//  Prepare and transmit telemetry packets
//

#ifndef telemetry_h
#define telemetry_h

#include "global.h"


#define TELEMETRY_EEPROM_PERIOD_LOW         0
#define TELEMETRY_EEPROM_PERIOD_HIGH        250

#define TELEMETRY_RADIO_PERIOD_EXTRA_LOW    15000
#define TELEMETRY_RADIO_PERIOD_LOW          15000
#define TELEMETRY_RADIO_PERIOD_MEDIUM       5000
#define TELEMETRY_RADIO_PERIOD_HIGH         1000

extern uint32_t eeprom_telemetry_period;
extern uint32_t radio_telemetry_period;

/**
 *  Initilize the telmetry service
 */
extern void init_telemetry (void);

/**
 *  Send a telemetry packet on the next call to telemetry_service
 */
extern void telemetry_send_packet (void);

/**
 *  Code to be run in each iteration of the main loop
 */
extern void telemetry_service(void);

#endif /* telemetry_h */
