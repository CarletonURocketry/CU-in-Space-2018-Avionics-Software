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

// MARK: Deployment
#define MAIN_TRIGGER_DDR    DDRD
#define MAIN_TRIGGER_PORT   PORTD
#define MAIN_TRIGGER_NUM    PIND7

#define DROGUE_TRIGGER_DDR  DDRD
#define DROGUE_TRIGGER_PORT PORTD
#define DROGUE_TRIGGER_NUM  PIND6

#define ENGINE_VALVE_DDR    DDRD
#define ENGINE_VALVE_PORT   PORTD
#define ENGINE_VALVE_NUM    PIND5

// MARK: SPI Pins
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

#define SPI_MOSI_DDR        DDRB
#define SPI_MOSI_NUM        PINB5

#define SPI_MISO_DDR        DDRB
#define SPI_MISO_NUM        PINB6

#define SPI_SCK_DDR         DDRB
#define SPI_SCK_NUM         PINB7

// MARK: Analog Inputs
#define TEMP_1_ANALOG_PIN       0
#define BAT_VOLTAGE_ANALOG_PIN  7

#endif /* pindefinitions_h */
