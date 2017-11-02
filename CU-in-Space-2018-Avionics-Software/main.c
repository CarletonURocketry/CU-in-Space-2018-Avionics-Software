//
//  main.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include <avr/io.h>
#include <avr/interrupt.h>

#include "pindefinitions.h"

//MARK: Constants

// MARK: Function prototypes
/**
 *  The code which runs endlessly forever
 */
static void main_loop(void);

// MARK: Variable Definitions

volatile uint32_t millis;
volatile uint8_t flags;

// MARK: Function Definitions
void initIO(void)
{
	
}

void init_timers(void)
{
    // Timer 1 (clock)
    TCCR1B |= (1<<WGM12);                           // Set the Timer Mode to CTC
    TIMSK1 |= (1<<OCIE1A);                          // Set the ISR COMPA vector (enables COMP interupt)
    OCR1AL = 220;                                   // 1000 Hz
    OCR1AH = 5;                                     // OCR1A = 1500
    TCCR1B |= (1<<CS11);                            // set prescaler to 8 and start timer 1
}

int main(void)
{
    cli();
    
	initIO();
    init_timers();
    
    sei();

    for (;;) {
        main_loop();
	}
	return 0; // never reached
}

static void main_loop ()
{
    
}

// MARK: Interupt Service Routines
ISR (TIMER1_COMPA_vect)                             // Timer 0, called every millisecond
{
    millis++;
}
