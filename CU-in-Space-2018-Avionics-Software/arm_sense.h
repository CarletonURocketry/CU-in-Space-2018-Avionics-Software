//
//  arm_sense.h
//  Index
//
//  Created by Samuel Dewan on 2018-03-14.
//

#ifndef arm_sense_h
#define arm_sense_h

#include "global.h"

/**
 *  Debounces the armed switch input
 */
extern void arm_sense_db_service (void);

/**
 *  Determine what the current position of the armed switch is
 *  @return 1 if the switch is in the armed position, 0 otherwise
 */
extern uint8_t is_armed (void);

/**
 *  Arm the ejection system
 */
extern void arm(void);

/**
 * Disarm the ejection system
 */
extern void disarm(void);

#endif /* arm_sense_h */
