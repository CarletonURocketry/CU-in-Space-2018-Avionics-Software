//
//  global.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#ifndef global_h
#define global_h

#include <stdint.h>                     // Int types are needed everywehre

// MARK: Definitions
#define TIMER_FREQUENCY     1000

// MARK: Global variables
/** The number of milliseconds elapsed since initilization*/
extern volatile uint32_t millis;

/** Various global boolean fields as described in pindefinitions.h*/
extern volatile uint8_t flags;

#endif /* global_h */
