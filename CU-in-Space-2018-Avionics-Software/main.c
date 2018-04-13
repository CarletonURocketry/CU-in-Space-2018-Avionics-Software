//
//  main.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include "pindefinitions.h"

#include "ematch_detect.h"
#include "menu.h"
#include "telemetry.h"
#include "SPI.h"
#include "I2C.h"
#include "ADC.h"

#include "Accel-ADXL343.h"
#include "Barometer-MPL3115A2.h"
#include "Gyro-FXAS21002C.h"
#include "GPS-FGPMMOPA6H.h"
#include "25LC1024.h"

//MARK: Constants

// MARK: Function prototypes
/**
 *  The code which runs endlessly forever
 */
static void main_loop(void);

/**
 *  Runs during the init3 section to fetch MCUSR and disable the watchdog timer
 */
void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));

// MARK: Variable Definitions
volatile uint32_t millis;
volatile uint8_t flags;
reset_reason reset_type;

volatile static uint32_t last_led;

// Mirror of MCUSR created during init process before watchdog is reset
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

// MARK: Function Definitions
void get_mcusr(void)
{
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
}

void initIO(void)
{
    // Set LED pin as an output
    LED_DDR |= (1<<LED_NUM);
    // Set reset jumper pin as an input with pullup
    RESET_JUMPER_DDR &= ~(1<<RESET_JUMPER_NUM);
    RESET_JUMPER_PORT |= (1<<RESET_JUMPER_NUM);
    
    // Set cap discharge pin as an output and drive low
    CAP_DISCHARGE_DDR |= (1<<CAP_DISCHARGE_NUM);
    CAP_DISCHARGE_PORT &= ~(1<<CAP_DISCHARGE_NUM);
    // Set main triggger pin as an output and drive low
    MAIN_TRIGGER_DDR |= (1<<MAIN_TRIGGER_NUM);
    MAIN_TRIGGER_PORT &= ~(1<<MAIN_TRIGGER_NUM);
    // Set e-match sense 1 pin as an input without pullup
    EMATCH_SENSE_1_DDR &= ~(1 << EMATCH_SENSE_1_NUM);
    EMATCH_SENSE_1_PORT &= ~(1 << EMATCH_SENSE_1_NUM);
    // Set e-match sense 2 pin as an input without pullup
    EMATCH_SENSE_2_DDR &= ~(1 << EMATCH_SENSE_2_NUM);
    EMATCH_SENSE_2_PORT &= ~(1 << EMATCH_SENSE_2_NUM);
    // Set 12v enable pin as an output and drive high
    ENABLE_12V_DDR |= (1 << ENABLE_12V_NUM);
    ENABLE_12V_PORT |= (1 << ENABLE_12V_NUM);
    
    // Set altimeter interupt pin as an input without pullup
    ALT_INT_DDR &= ~(1 << ALT_INT_NUM);
    ALT_INT_PORT &= ~(1 << ALT_INT_NUM);
    // Set accelerometer interupt pin as an input without pullup
    ACCEL_INT_DDR &= ~(1 << ACCEL_INT_NUM);
    ACCEL_INT_PORT &= ~(1 << ACCEL_INT_NUM);
    // Set gyroscope interupt pin as an input without pullup
    GYRO_INT_DDR &= ~(1 << GYRO_INT_NUM);
    GYRO_INT_PORT &= ~(1 << GYRO_INT_NUM);
    
    // Set eeprom 2 cs pin as ouput and drive high
    EEPROM2_CS_DDR |= (1 << EEPROM2_CS_NUM);
    EEPROM2_CS_PORT |= (1 << EEPROM2_CS_NUM);
    // Set radio attn pin as input without pullup
    RADIO_ATTN_DDR &= ~(1 << RADIO_ATTN_NUM);
    RADIO_ATTN_PORT &= ~(1 << RADIO_ATTN_NUM);
    // Set radio cs pin as ouput and drive high
    RADIO_CS_DDR |= (1 << RADIO_CS_NUM);
    RADIO_CS_PORT |= (1 << RADIO_CS_NUM);
    // Set eeprom cs pin as ouput and drive high
    EEPROM_CS_DDR |= (1 << EEPROM_CS_NUM);
    EEPROM_CS_PORT |= (1 << EEPROM_CS_NUM);
    // Set spi MOSI pin as an output
    SPI_MOSI_DDR |= (1<<SPI_MOSI_NUM);
    // Set spi MISO pin as an input
    SPI_MISO_DDR &= ~(1<<SPI_MISO_NUM);
    // Set spi SCK pin as an output
    SPI_SCK_DDR |= (1<<SPI_SCK_NUM);
}

void init_timers(void)
{
    // Timer 1 (clock)
    TCCR1B |= (1<<WGM12);                           // Set the Timer Mode to CTC
    TIMSK1 |= (1<<OCIE1A);                          // Set the ISR COMPA vector (enables COMP interupt)
    OCR1AH = 5;                                     // OCR1A = 1500 - Note: The datasheet is wrong, MSB must be written first
    OCR1AL = 220;                                   // 1000 Hz
    
    TCCR1B |= (1<<CS11);                            // set prescaler to 8 and start timer 1
}

int main(void)
{
    // Get reset reason
    if (mcusr_mirror & (1<<PORF )) reset_type = POWERON;
    else if (mcusr_mirror & (1<<EXTRF)) reset_type = EXTERNAL;
    else if (mcusr_mirror & (1<<BORF )) reset_type = BROWNOUT;
    else if (mcusr_mirror & (1<<WDRF )) reset_type = WATCHDOG;
    else if (mcusr_mirror & (1<<JTRF )) reset_type = JTAG;
    
    cli();
    PRR0 |= (1<<PRTIM2) |  (1<<PRTIM0);             // Shutdown timers 0 and 2
    PRR1 |= (1<<PRTIM3);                            // Shutdown timer 3

    initIO();
    init_timers();

    // Initilize IO peripherals
#ifdef ENABLE_ADC
    ADC_ENABLE_MASK = 0xff;                         // Enable all ADC channels
    init_adc();
#endif
#ifdef ENABLE_SPI
    init_spi(&SPI_PORT);
#endif
#ifdef ENABLE_I2C
    init_i2c();
#endif

    // Initilize software modules
    init_menu();

    // Enable the watchdog timer for a 2 second timeout
    wdt_enable(WDTO_2S);
    sei();

    // Initilize external peripherals
#ifdef ENABLE_ALTIMETER
    init_mpl3115a2(); // Barometric Altimeter
#endif
#ifdef ENABLE_ACCELEROMETER
    init_adxl343(); // Accelerometer
#endif
#ifdef ENABLE_GYROSCOPE
    init_fxas21002c(); // Gyroscope
#endif
#ifdef ENABLE_GPS
    fgpmmopa6h_service(); // GPS
#endif
#ifdef ENABLE_EEPROM
    init_25lc1024(EEPROM_CS_NUM, EEPROM2_CS_NUM); // EEPROM
#endif

    for (;;) {
        main_loop();
	}
}

static void main_loop ()
{
    wdt_reset(); // Pat the dog
    
#ifdef ENABLE_DEBUG_FLASH
    // Flash LED
    if ((millis - last_led) > 250) {
        last_led = millis;
        LED_PORT ^= (1<<LED_NUM);
    }
#endif
    
    // Run IO Services
#ifdef ENABLE_ADC
    adc_service();
#endif
#ifdef ENABLE_SPI
    spi_service();
#endif
#ifdef ENABLE_I2C
    i2c_service();
#endif
    
    // Run Peripheral Services
#ifdef ENABLE_ALTIMETER
    mpl3115a2_service(); // Barometric Altimeter
#endif
#ifdef ENABLE_ACCELEROMETER
    adxl343_service();  // Accelerometer
#endif
#ifdef ENABLE_GYROSCOPE
    fxas21002c_service();  // Gyroscope
#endif
#ifdef ENABLE_GPS
    fgpmmopa6h_service();  // GPS
#endif
#ifdef ENABLE_EEPROM
    eeprom_25lc1024_service(); // EEPROM
#endif
    
    // Run Software Module Servies
    ematch_detect_service();
    telemetry_service();
    menu_service();
}

// MARK: Interupt Service Routines
ISR (TIMER1_COMPA_vect)                             // Timer 0, called every millisecond
{
    millis++;
}

ISR(BADISR_vect)
{
    for (;;) {
        for (uint32_t i = 0; i < 750000; i++);
        LED_PORT ^= (LED_NUM);
    }
}
