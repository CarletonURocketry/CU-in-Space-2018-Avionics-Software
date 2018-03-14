//
//  arm_sense.c
//  Index
//
//  Created by Samuel Dewan on 2018-03-14.
//  Copyright © 2018 Samuel Dewan. All rights reserved.
//

#include "arm_sense.h"

#include "global.h"
#include "pindefinitions.h"

#include <avr/io.h>

#define ARMED_SENSE_SAMPLE_PERIOD   1

/**
 * The current state of the armed switch
 */
static uint8_t armed_sense_state;

/**
 * A history of states of the armed switch
 */
static uint16_t armed_sense_shift;

/**
 * The mission time when the armed sense pin was last sampled
 */
static uint32_t last_shift_time;


void arm_sense_db_service (void)
{
    if ((millis - last_shift_time) >= ARMED_SENSE_SAMPLE_PERIOD) {
        // Shift the current state into the shift variable
        armed_sense_shift = (armed_sense_shift << 1);
        armed_sense_shift |= !!(ARMED_SENSE_PIN & (1<<ARMED_SENSE_NUM));
        
        if ((armed_sense_shift == 0x00) && (armed_sense_state == 0)) {
            // Now on, was off -> Arm
            armed_sense_state = 1;
            arm();
        } else if ((armed_sense_shift == UINT16_MAX) && (armed_sense_state == 1)) {
            // Now off, was on -> Disarm
            armed_sense_state = 0;
            disarm();
        }
    }
}

void arm(void)
{
    // Disable capacitor discharge circuit
    CAP_DISCHARGE_PORT &= ~(1<<CAP_DISCHARGE_NUM);
    // Enable 12v rail
    DEPLOY_12V_EN_PORT |= (1<<DEPLOY_12V_EN_NUM);
}

void disarm(void)
{
    // Disable 12v rail
    DEPLOY_12V_EN_PORT &= ~(1<<DEPLOY_12V_EN_NUM);
    // Enable capacitor discharge circuit
    CAP_DISCHARGE_PORT |= (1<<CAP_DISCHARGE_NUM);
}
