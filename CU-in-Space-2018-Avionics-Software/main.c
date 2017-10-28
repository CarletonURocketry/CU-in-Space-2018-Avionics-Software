#include <avr/io.h>
#include <avr/interrupt.h>

//MARK: Constants

// MARK: Function prototypes
static void main_loop(void);

// MARK: Variable Definitions

// MARK: Function Definitions
void initIO(void)
{
	
}

void init_timers(void)
{
    
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
