//
//  Barometer-MPL3115A2.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//  Barometric Pressure Sensor
//

#ifndef Barometer-MPL3115A2_h
#define Barometer-MPL3115A2_h

#include "global.h"

// MARK: Constants
#define MPL3115A2_SAMPLE_RATE   5   // The number of times per second that the sensor should be polled

// MARK: Variables
/**
 *  The value of the global millis variable when the last sample was recieved from the sensor
 */
extern uint32_t mpl3115a2_sample_time;

/*
 *  The current altitude in sixteenths of a meter
 */
extern uint32_t mpl3115a2_alt;

/*
 *  The most signifigant 8 bytes in the altidude value as recieved from the sensor
 */
extern uint8_t mpl3115a2_alt_msb;
/*
 *  The least signifigant 8 bytes in the whole part of altidude value as recieved from the sensor
 */
extern uint8_t mpl3115a2_alt_csb;
/*
 *  The fraction part of the altitude value as recieved from the sensor
 */
extern uint8_t mpl3115a2_alt_lsb;

/*
 *  The whole part of the temeperature as recieved from the sensor
 */
extern uint8_t mpl3115a2_temp_msb;
/*
 *  The fraction part of the temperature value as recieved from the sensor
 */
extern uint8_t mpl3115a2_temp_lsb;


// MARK: Function declarations
/**
 *  Initilize the MOL3115A2 barometric pressure altimiter
 *  @returns 0 if the function was successfull
 */
extern uint8_t init_mpl3115a2(void);

/**
 *  Code to be run in each iteration of the main loop
 */
extern void mpl3115a2_service(void);

#endif /* Barometer-MPL3115A2_h */
