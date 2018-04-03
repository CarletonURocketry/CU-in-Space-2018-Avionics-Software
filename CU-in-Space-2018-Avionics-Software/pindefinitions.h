//
//  pindefinitions.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#ifndef pindefinitions_h
#define pindefinitions_h

// MARK: Flags
#define FLAG_SERIAL_1_LOOPBACK  4
#define FLAG_SERIAL_1_TX_LOCK   5
#define FLAG_SERIAL_0_LOOPBACK  6
#define FLAG_SERIAL_0_TX_LOCK   7

// MARK: General
#define LED_DDR             DDRB
#define LED_PORT            PORTB
#define LED_NUM             PINB0

#define RESET_JUMPER_DDR    DDRC
#define RESET_JUMPER_PORT   PORTC
#define RESET_JUMPER_PIN    PINC
#define RESET_JUMPER_NUM    PINC6

// MARK: Deployment
#define CAP_DISCHARGE_DDR   DDRD
#define CAP_DISCHARGE_PORT  PORTD
#define CAP_DISCHARGE_NUM   PIND4

#define MAIN_TRIGGER_DDR    DDRD
#define MAIN_TRIGGER_PORT   PORTD
#define MAIN_TRIGGER_NUM    PIND5

#define EMATCH_SENSE_1_DDR  DDRD
#define EMATCH_SENSE_1_PORT PORTD
#define EMATCH_SENSE_1_PIN  PIND
#define EMATCH_SENSE_1_NUM  PIND6

#define EMATCH_SENSE_2_DDR  DDRD
#define EMATCH_SENSE_2_PORT PORTD
#define EMATCH_SENSE_2_PIN  PIND
#define EMATCH_SENSE_2_NUM  PIND7

#define ENABLE_12V_DDR      DDRC
#define ENABLE_12V_PORT     PORTC
#define ENABLE_12V_NUM      PINC5

// MARK: Sensor Interupts
#define ALT_INT_DDR         DDRC
#define ALT_INT_PORT        PORTC
#define ALT_INT_PIN         PINC
#define ALT_INT_NUM         PINC2

#define ACCEL_INT_DDR       DDRC
#define ACCEL_INT_PORT      PORTC
#define ACCEL_INT_PIN       PINC
#define ACCEL_INT_NUM       PINC3

#define GYRO_INT_DDR        DDRC
#define GYRO_INT_PORT       PORTC
#define GYRO_INT_PIN        PINC
#define GYRO_INT_NUM        PINC4

// MARK: SPI Pins
#define EEPROM2_CS_DDR      DDRB
#define EEPROM2_CS_PORT     PORTB
#define EEPROM2_CS_NUM      PINB1

#define RADIO_ATTN_DDR      DDRB
#define RADIO_ATTN_PORT     PORTB
#define RADIO_ATTN_PIN      PINB
#define RADIO_ATTN_NUM      PINB2

#define EEPROM_CS_DDR       DDRB
#define EEPROM_CS_PORT      PORTB
#define EEPROM_CS_NUM       PINB3

#define RADIO_CS_DDR        DDRB
#define RADIO_CS_PORT       PORTB
#define RADIO_CS_NUM        PINB4

#define SPI_PORT            PORTB

#define SPI_MOSI_DDR        DDRB
#define SPI_MOSI_NUM        PINB5

#define SPI_MISO_DDR        DDRB
#define SPI_MISO_NUM        PINB6

#define SPI_SCK_DDR         DDRB
#define SPI_SCK_NUM         PINB7

// MARK: Analog Inputs
#define CAP_REF_ANALOG_PIN      0
#define TEMP_1_ANALOG_PIN       1
#define TEMP_2_ANALOG_PIN       2
#define BAT_REF_ANALOG_PIN      7

#endif /* pindefinitions_h */
