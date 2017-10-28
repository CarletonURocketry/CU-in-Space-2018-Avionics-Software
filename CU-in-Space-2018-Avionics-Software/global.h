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
extern volatile uint32_t millis;        // Tracks the number of milliseconds elapsed since initilization

extern volatile uint8_t flags;          // Stores some global boolean flags

#endif /* global_h */
