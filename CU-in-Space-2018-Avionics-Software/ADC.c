 //
//  ADC.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-31.
//

#include "ADC.h"

#include <avr/io.h>
#include <avr/interrupt.h>

// MARK: Constants
#define ADC_FLAG_AUTO_ENABLED   2   // When this flag is set to one the ADC will take readings at a set interval
#define ADC_FLAG_VALUE_CURRENT  1   // When this flag is set the values in adc_avg_data are up to date
#define ADC_FLAG_IN_PROGRESS    0   // This flag is set to one when there is a set of conversions taking place

// MARK: Variable Definitions
static uint16_t adc_raw_data[ADC_NUM_CHANNELS][ADC_NUM_SAMPLES];
uint16_t adc_avg_data[ADC_NUM_CHANNELS];

/** Various boolean fields used by the ADC*/
static volatile uint8_t adc_flags;

/** The time in milliseconds since startup that the last read of the ADC was started*/
static uint32_t adc_last_sample_time;

/** The next channel to be read from the ADC*/
static volatile uint8_t adc_current_chan;
/** The current sample number*/
static volatile uint8_t adc_sample_num;

// MARK: Function Definitions

void init_adc(void)
{
    ADCSRA |= (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1);  // Enable ADC interupt and set prescaler to 64 (187.5kHz)
    // Each ADC clock cycles is 64 system clock cycles. (5.333 microseconds)
    // Conversions take 13 ADC clock cycles (69.333 microseconds), except for the first which takes 25 (133.333 microseconds).
    // This means that in general reading all 8 ADCs would take about 104 ADC cycles (554.667 microseconds)
    
    ADMUX = (1<<REFS0);                             // Set refrence to AVCC with external capacitor (3.3v)
}

void adc_service(void)
{
    uint8_t adc_active = adc_flags & (1<<ADC_FLAG_IN_PROGRESS);
    
    if (!(adc_flags & (1<<ADC_FLAG_VALUE_CURRENT)) && !adc_active) {
        // Find average value for each channel
        for (int i = 0; i < ADC_NUM_CHANNELS; i++) {
            uint16_t sample_sum = 0;
            for (int j =0; j < ADC_NUM_SAMPLES; i++) {
                sample_sum += (uint16_t) adc_raw_data[i][j];
            }
            adc_avg_data[i] = (uint8_t)(sample_sum / ADC_NUM_SAMPLES);
        }
        adc_flags |= (1<<ADC_FLAG_VALUE_CURRENT);
    }
    
    if (!adc_active && (adc_flags & (1<<ADC_FLAG_AUTO_ENABLED)) && (millis - adc_last_sample_time) >= ADC_AUTO_PERIOD) {
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
    adc_flags &= ~(1<<ADC_FLAG_VALUE_CURRENT);
    
    // Select next enabled channel
    uint8_t adc_chan_selected = 0;
    for (uint8_t i = 0; i < ADC_NUM_CHANNELS; i++) {
        if (ADC_ENABLE_MASK & (1<<i)) {
            adc_chan_selected = 1;
            adc_current_chan = i;
            ADMUX |= adc_current_chan;  // Set adc channel
            break;
        }
    }
    
    if (adc_chan_selected) {            // We don't want to bother turning on the ADC if no channels are enabled
        adc_sample_num = 0;
        
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
    
    adc_raw_data[adc_current_chan][adc_sample_num] = value;
    
    adc_sample_num++;
    if (adc_sample_num < ADC_NUM_SAMPLES) {
        // Take next sample
        ADCSRA |= (1<<ADSC);                // Start the next ADC conversion
    } else {
        // Switch to next channel
        adc_sample_num = 0;
        
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
            // finished conversion
            adc_flags &= ~(1<<ADC_FLAG_IN_PROGRESS);
            ADCSRA &= ~(1<<ADEN);           // Disable the ADC
            PRR0 |= (1<<PRADC);             // Shut down ADC to save power
        }
    }
}
