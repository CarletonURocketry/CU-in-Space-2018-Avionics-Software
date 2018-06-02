//
//  ematch_detect.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2018-04-03.
//

#include "ematch_detect.h"

#include "pindefinitions.h"
#include <avr/io.h>

#define EMATCH_SENSE_SAMPLE_PERIOD   1

typedef struct {
    uint32_t ematch_1_last_shift_time;
    uint32_t ematch_2_last_shift_time;
    
    uint16_t ematch_1_sense_shift;
    uint16_t ematch_2_sense_shift;
    
    uint8_t ematch_1_sense_state:1;
    uint8_t ematch_2_sense_state:1;
} debounce_data_t;

static debounce_data_t debounce_data;

void ematch_detect_service (void)
{
    if ((millis - debounce_data.ematch_1_last_shift_time) >= EMATCH_SENSE_SAMPLE_PERIOD) {
        // Shift the current state into the shift variable
        debounce_data.ematch_1_sense_shift = (debounce_data.ematch_1_sense_shift << 1);
        debounce_data.ematch_1_sense_shift |= !(EMATCH_SENSE_1_PIN & (1<<EMATCH_SENSE_1_NUM));
        
        if ((debounce_data.ematch_1_sense_shift == 0x00) && (debounce_data.ematch_1_sense_state == 0)) {
            // Now on, was off -> Arm
            debounce_data.ematch_1_sense_state = 1;
        } else if ((debounce_data.ematch_1_sense_shift == UINT16_MAX) && (debounce_data.ematch_1_sense_state == 1)) {
            // Now off, was on -> Disarm
            debounce_data.ematch_1_sense_state = 0;
        }
    }
    
    if ((millis - debounce_data.ematch_2_last_shift_time) >= EMATCH_SENSE_SAMPLE_PERIOD) {
        // Shift the current state into the shift variable
        debounce_data.ematch_2_sense_shift = (debounce_data.ematch_2_sense_shift << 1);
        debounce_data.ematch_2_sense_shift |= !(EMATCH_SENSE_2_PIN & (1<<EMATCH_SENSE_2_NUM));
        
        if ((debounce_data.ematch_2_sense_shift == 0x00) && (debounce_data.ematch_2_sense_state == 0)) {
            // Now on, was off -> Arm
            debounce_data.ematch_2_sense_state = 1;
        } else if ((debounce_data.ematch_2_sense_shift == UINT16_MAX) && (debounce_data.ematch_2_sense_state == 1)) {
            // Now off, was on -> Disarm
            debounce_data.ematch_2_sense_state = 0;
        }
    }
}

uint8_t ematch_1_is_ready (void)
{
    return debounce_data.ematch_1_sense_state;
}

uint8_t ematch_2_is_ready (void)
{
    return debounce_data.ematch_2_sense_state;
}

#include "ematch_detect.h"
