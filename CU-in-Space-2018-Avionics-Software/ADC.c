//
//  ADC.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-31.
//

#include "ADC.h"

#include <avr/io.h>
#include <avr/interrupt.h>

// MARK: Variable Definitions
uint16_t adc_data[ADC_NUM_CHANNELS];

volatile uint8_t adc_enable_mask;
volatile uint8_t adc_flags;

uint32_t adc_last_sample_time;
static volatile uint8_t adc_current_chan;

// MARK: Function Definitions

void init_adc(void)
{
    ADCSRA |= (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1);  // Enable ADC interupt and set prescaler to 64 (187.5kHz)
    // Each ADC clock cycles is 64 system clock cycles. (5.334 milliseconds)
    // Conversions take 13 ADC clock cycles, except for the first which takes 25.
    // This means that in general reading all 8 ADCs would take about 104 ADC cycles (554.667 milliseconds)
    
    ADMUX = (1<<REFS0);                             // Set refrence to AVCC with external capacitor (3.3v)
}

void adc_service(void)
{
    if ((adc_flags & (1<<ADC_FLAG_AUTO_ENABLED)) && (millis - adc_last_sample_time) >= ADC_AUTO_PERIOD) {
        // Start a new set of conversions
        adc_last_sample_time = millis;
        adc_start_conversion();
    }
}

void adc_start_conversion(void)
{
    if (adc_flags & (1<<ADC_FLAG_IN_PROGRESS)) {
        return;
    }
    adc_flags |= (1<<ADC_FLAG_IN_PROGRESS);
    
    // Select next enabled channel
    uint8_t adc_chan_selected = 0;
    for (uint8_t i = 0; i < ADC_NUM_CHANNELS; i++) {
        if (ADC_ENABLE_MASK & (1<<i)) {
            adc_chan_selected = 1;
            adc_current_chan = i;
            ADMUX |= adc_current_chan;  // Set adc channel
            ADCSRA |= (1<<ADSC);        // Start the next ADC conversion
            break;
        }
    }
    
    if (adc_chan_selected) {            // We don't want to bother turning on the ADC if no channels are enabled
        PRR0 &= ~(1<<PRADC);            // Turn on power to the ADC
        ADCSRA |= (1<<ADEN);            // Enable the ADC
        ADMUX |= adc_current_chan;      // Set adc channel
        ADCSRA |= (1<<ADSC);            // Start the next ADC conversion
    }
}

// MARK: Interupt Service Routines
ISR(ADC_vect)   // ADC Conversion Complete
{
    uint16_t value = ADCL;
    value |= (((uint16_t)ADCH) << 8);
    
    adc_data[adc_current_chan] = value;
    
    // Select the next enabled channel
    uint8_t adc_chan_selected = 0;
    for (uint8_t i = adc_current_chan + 1; i < ADC_NUM_CHANNELS; i++) {
        if (ADC_ENABLE_MASK & (1<<i)) {
            adc_chan_selected = 1;
            adc_current_chan = i;
            ADMUX |= adc_current_chan;  // Set adc channel
            ADCSRA |= (1<<ADSC);        // Start the next ADC conversion
            break;
        }
    }
    
    if (!adc_chan_selected) {
        adc_flags &= ~(1<<ADC_FLAG_IN_PROGRESS);
        ADCSRA &= ~(1<<ADEN);           // Disable the ADC
        PRR0 |= (1<<PRADC);             // Shut down ADC to save power
    }
}
