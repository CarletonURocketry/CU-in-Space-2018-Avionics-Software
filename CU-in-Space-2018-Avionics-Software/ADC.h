//
//  ADC.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-31.
//
//  Asynchronously read data from analog inputs
//

#ifndef ADC_h
#define ADC_h

#include "global.h"

// MARK: Constants
#define ADC_NUM_CHANNELS        8       // Number of ADC channels aviable. This includeds channels which are not being used
#define ADC_AUTO_PERIOD         4000    // The period at which readings will be taken from all enabled channels

#define ADC_FLAG_AUTO_ENABLED   0   // When this flag is set to one the ADC will take readings at a set interval
#define ADC_FLAG_IN_PROGRESS    1   // This flag is set to one when there is a set of conversions taking place

// Setting a bit in DIDR0 will enable reading on the corresponding analog input and disable the corresponding digital pin
#define ADC_ENABLE_MASK         DIDR0

// MARK: Variable Declarations
/** The most resently read data from each ADC*/
extern uint16_t adc_data[ADC_NUM_CHANNELS];

/** Various boolean fields used by the ADC*/
extern volatile uint8_t adc_flags;
/** The time in milliseconds since startup that the last read of the ADC was started*/
extern uint32_t adc_last_sample_time;

// MARK: Function Prototypes

/**
 *  Initialize the ADC hardware
 */
extern void init_adc(void);

/**
 *  Service to be run in each iteration of the main loop
 */
extern void adc_service(void);

/**
 *  Service to be run in each iteration of the main loop
 */
extern void adc_start_conversion(void);


#endif /* ADC_h */
