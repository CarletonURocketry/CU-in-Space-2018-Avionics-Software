//
//  Accel-ADXL343.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//  Accelerometer
//

#ifndef Accel-ADXL343_h
#define Accel-ADXL343_h

#include "global.h"

// MARK: Constants
#define ADXL343_SAMPLE_RATE   5   // The number of times per second that the sensor should be polled

// MARK: Variables
/**
 *  The value of the global millis variable when the last sample was recieved from the sensor
 */
extern uint32_t adxl343_sample_time;

/**
 *  The acceleration in the x axis as recieved from the sensor
 */
extern int16_t adxl343_accel_x;
/**
 *  The acceleration in the y axis as recieved from the sensor
 */
extern int16_t adxl343_accel_y;
/**
 *  The acceleration in the z axis as recieved from the sensor
 */
extern int16_t adxl343_accel_z;

// MARK: Function declarations
/**
 *  Initilize the ADXL343 3-axis accelerometer
 *  @returns 0 if the function was successfull
 */
extern uint8_t init_adxl343(void);

/**
 *  Code to be run in each iteration of the main loop
 */
extern void adxl343_service(void);

#endif /* Accel-ADXL343_h */
