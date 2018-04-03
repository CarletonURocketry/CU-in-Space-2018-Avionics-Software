//
//  global.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

// Fuses: -U lfuse:w:0xff:m -U hfuse:w:0xd7:m -U efuse:w:0xfd:m
//        -U lfuse:w:0xbf:m -U hfuse:w:0xd7:m -U efuse:w:0xfd:m (with clock output on PB1)

#ifndef global_h
#define global_h

#include <stdint.h>                     // Int types are needed everywehre

// MARK: Compile Time Settings
#define ENABLE_DEBUG_FLASH

#define ENABLE_SPI
#define ENABLE_I2C
#define ENABLE_ADC

//#define ENABLE_ALTIMETER
//#define ENABLE_ACCELEROMETER
//#define ENABLE_GYROSCOPE
//#define ENABLE_GPS

//#define ENABLE_XBEE

#define ENABLE_EEPROM

// MARK: Constants
#define TIMER_FREQUENCY     1000

// MARK: Startup cause enum
typedef enum {JTAG, WATCHDOG, BROWNOUT, EXTERNAL, POWERON} reset_reason;

// MARK: Global variables
/** The number of milliseconds elapsed since initilization*/
extern volatile uint32_t millis;

/** Various global boolean fields as described in pindefinitions.h*/
extern volatile uint8_t flags;

/** The reason for the last MCU reset*/
extern reset_reason reset_type;

#endif /* global_h */
