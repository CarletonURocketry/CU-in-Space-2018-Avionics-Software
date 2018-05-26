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

#include <inttypes.h>                   // Int types are needed everywehre

// MARK: Compile Time Settings
#define ENABLE_SENSORS_AT_RESET

#define ENABLE_DEBUG_FLASH

#define ENABLE_SPI
#define ENABLE_I2C
#define ENABLE_ADC

#define ENABLE_ALTIMETER
#define ENABLE_ACCELEROMETER
//#define ENABLE_GYROSCOPE
#define ENABLE_GPS

//#define ENABLE_XBEE

#define ENABLE_EEPROM

// FSM Settings
#define LAUNCH_ACCEL_THRESHOLD      1282    // 5g in 3.9mg per least signifigant bit
#define COASTING_ACCEL_THRESHOLD    256     // 1g in 3.9mg per least signifigant bit
#define ALTITUDE_COMPARISON_RANGE   8       // 0.5m in 1/16m per least signifigant bit

// MARK: EEPROM Addresses
#define EEPORM_ADDR_OSCCAL          0
#define EEPROM_ADDR_FSM_STATE       1

// MARK: Constants
#define TIMER_FREQUENCY     1000

// MARK: Startup cause enum
typedef enum {JTAG, WATCHDOG, BROWNOUT, EXTERNAL, POWERON} reset_reason_t;

// MARK: FSM enum
typedef enum {  STANDBY,        // Rocket is not ready for flight. Wait for rocket to be armed. Very low rate telemetry.
                PRE_FLIGHT,     // Rocket is armed and ready to launch. Low rate telemetry is being transmitted.
                POWERED_ASCENT, // Engine is burning. High rate telemetery transmitted and logged.
                COASTING_ASCENT,// Engine has burnt out.
                DESCENT,        // Rocket is descending with parachute.
                RECOVERY        // Rocket has landed, Very low rate telemetry is being transmitted.
} global_state_t;

// MARK: Global variables
/** The number of milliseconds elapsed since initilization*/
extern volatile uint32_t millis;

/** Various global boolean fields as described in pindefinitions.h*/
extern volatile uint8_t flags;

/** The reason for the last MCU reset*/
extern reset_reason_t reset_type;

/** The current state of the main FSM*/
extern global_state_t fsm_state;

#endif /* global_h */
