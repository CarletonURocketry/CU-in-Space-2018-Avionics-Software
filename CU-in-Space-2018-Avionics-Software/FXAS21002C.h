//
//  FXAS21002C.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//  Gyroscope
//

#ifndef FXAS21002C_h
#define FXAS21002C_h

#include "global.h"

// MARK: Constants
#define FXAS21002C_SAMPLE_RATE   5   // The number of times per second that the sensor should be polled

// MARK: Variables
/**
 *  The value of the global millis variable when the last sample was recieved from the sensor
 */
extern uint32_t fxas21002c_sample_time;

/**
 *  The pitch rate as recieved from the sensor
 */
extern int16_t fxas21002c_pitch_rate;
/**
 *  The roll rate as recieved from the sensor
 */
extern int16_t fxas21002c_roll_rate;
/**
 *  The yaw rate as recieved from the sensor
 */
extern int16_t fxas21002c_yaw_rate;

/**
 *  The temperature as recieved from the sensor
 */
extern int8_t fxas21002c_temp;

// MARK: Function declarations
/**
 *  Initilize the FXAS21002C 3-axis gyroscope
 *  @returns 0 if the function was successfull
 */
extern uint8_t init_fxas21002c(void);

/**
 *  Code to be run in each iteration of the main loop
 */
extern void fxas21002c_service(void);

#endif /* FXAS21002C_h */
