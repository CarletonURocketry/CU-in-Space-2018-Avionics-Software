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
#define ADC_NUM_CHANNELS    8       // Number of ADC channels aviable, this includeds channels which are not being used
#define ADC_NUM_SAMPLES     10      // The number of samples to average for each reading
#define ADC_AUTO_PERIOD     1000    // The period at which readings will be taken from all enabled channels in milliseconds

// Setting a bit in DIDR0 will enable reading on the corresponding analog input and disable the corresponding digital pin
#define ADC_ENABLE_MASK     DIDR0

// MARK: Variable Declarations
/** The most resently read data from each ADC*/
extern uint16_t adc_avg_data[ADC_NUM_CHANNELS];

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
