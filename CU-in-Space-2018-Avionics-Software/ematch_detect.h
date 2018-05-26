//
//  ematch_detect.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2018-04-03.
//

#ifndef ematch_detect_h
#define ematch_detect_h

#include "global.h"

/**
 *  Debounces the ematch present inputs
 */
extern void ematch_detect_service (void);

/**
 *  Determine if the first ematch is ready to fire
 *  @return 1 if the ematch is present and armed, 0 otherwise
 *  @note If the ematch has already fired it will not show as present.
 */
extern uint8_t ematch_1_is_ready (void);

/**
 *  Determine if the first ematch is ready to fire
 *  @return 1 if the ematch is present and armed, 0 otherwise
 *  @note If the ematch has already fired it will not show as present.
 */
extern uint8_t ematch_2_is_ready (void);

#endif /* ematch_detect_h */
