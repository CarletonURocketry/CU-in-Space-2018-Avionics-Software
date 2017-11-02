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
    // Set radio attn pin as input without pullup
    RADIO_ATTN_DDR &= ~(1 << RADIO_ATTN_NUM);
    RADIO_ATTN_PORT &= ~(1 << RADIO_ATTN_NUM);
    // Set radio cs pin as ouput and drive high
    RADIO_CS_DDR |= (1 << RADIO_CS_NUM);
    RADIO_CS_PORT |= (1 << RADIO_ATTN_NUM);
    // Set eeprom cs pin as ouput and drive high
    EEPROM_CS_DDR |= (1 << EEPROM_CS_NUM);
    EEPROM_CS_PORT |= (1 << EEPROM_CS_NUM);
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
    PRRR0 |= (1<<PRTIM2) |  (1<<PRTIM0) | (1<<PRADC);// Shutdown timers 0 and 2 and the ADC
    PRRR1 |= (1<<PRTIM3);                           // Shutdown timer 3
    
	initIO();
    init_timers();
    
    sei();

    for (;;) {
        main_loop();
	}
}

static void main_loop ()
{
    
}

// MARK: Interupt Service Routines
ISR (TIMER1_COMPA_vect)                             // Timer 0, called every millisecond
{
    millis++;
}
