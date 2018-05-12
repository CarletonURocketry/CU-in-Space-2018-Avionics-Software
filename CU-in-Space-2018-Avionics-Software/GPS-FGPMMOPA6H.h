//
//  GPS-FGPMMOPA6H.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//  GPS
//

#ifndef GPS_FGPMMOPA6H_h
#define GPS_FGPMMOPA6H_h

#include "global.h"

// MARK: Variables
/**
 *  The value of the global millis variable when the last sample was recieved from the sensor
 */
extern uint32_t fgpmmopa6h_sample_time;

/**
 *  The UTC time according to the GPS module in milliseconds since midnight
 */
extern uint32_t fgpmmopa6h_utc_time;

/**
 *  The latitude according to the GPS module in 100 micro-minutes, positive if in norhern hemisphere, negtive if in southern
 */
extern int32_t fgpmmopa6h_latitude;
/**
 *  The longitude according to the GPS module in 100 micro-minutes, positive if in eastern hemisphere, negative if in western
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

/**
 *  The number of sattelites which the GPS module is recieving singals from
 */
extern uint8_t fgpmmopa6h_satellites_in_view;

/**
 *  For each location packet recieved, a 1 is shifted into this byte if the packet contains valid data.
 */
extern uint8_t fgpmmopa6h_data_valid;

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

#endif /* GPS_FGPMMOPA6H_h */
