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
    // Timer 0 (clock)
    TCCR0B |= (1<<WGM01);                           // Set the Timer Mode to CTC
    TIMSK0 |= (1<<OCIE0A);                          // Set the ISR COMPA vector (enables COMP interupt)
    OCR0A = 125;                                    // 1000 Hz
    TCCR0B |= (1<<CS01)|(1<<CS00);                  // set prescaler to 64 and start timer 0
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
ISR (TIMER0_COMPA_vect)                             // Timer 0, called every millisecond
{
    millis++;
}
