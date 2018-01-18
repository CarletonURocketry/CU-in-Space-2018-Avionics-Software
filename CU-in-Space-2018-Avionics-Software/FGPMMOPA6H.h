//
//  FGPMMOPA6H.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//  GPS
//

#ifndef FGPMMOPA6H_h
#define FGPMMOPA6H_h

#include "global.h"

// MARK: Constants
#define FGPMMOPA6H_SAMPLE_RATE   5   // The number of times per second that the sensor should be polled

// MARK: Variables
/**
 *  The value of the global millis variable when the last sample was recieved from the sensor
 */
extern uint32_t fgpmmopa6h_sample_time;

/**
 *  The UTC time according to the GPS module. See Telemetery Format document for more information.
 */
extern uint32_t fgpmmopa6h_utc_time;

/**
 *  The lattitude according to the GPS module. See Telemetery Format document for more information.
 */
extern int32_t fgpmmopa6h_lattitude;
/**
 *  The longitude according to the GPS module. See Telemetery Format document for more information.
 */
extern int32_t fgpmmopa6h_longitude;

/**
 *  The land speed according to the GPS module in hundredths of a knot
 */
extern int16_t fgpmmopa6h_speed;
/**
 *  The course according to the GPS module in hundredths of a degree
 */
extern int16_t fgpmmopa6h_course;

// MARK: Function declarations
/**
 *  Initilize the FGPMMOPA6H GPS
 *  @returns 0 if the function was successfull
 */
extern uint8_t init_fgpmmopa6h(void);

/**
 *  Code to be run in each iteration of the main loop
 */
extern void fgpmmopa6h_service(void);

#endif /* FGPMMOPA6H_h */
