//
//  arm_sense.h
//  Index
//
//  Created by Samuel Dewan on 2018-03-14.
//  Copyright © 2018 Samuel Dewan. All rights reserved.
//

#ifndef arm_sense_h
#define arm_sense_h

/**
 *  Debounces the armed switch input
 */
extern void arm_sense_db_service (void);

/**
 *  Arm the ejection system
 */
extern void arm(void);

/**
 * Disarm the ejection system
 */
extern void disarm(void);

#endif /* arm_sense_h */
