//
//  bus_tests.c
//  Index
//
//  Created by Samuel Dewan on 2018-03-27.
//

#include "bus_tests.h"

#include "serial0.h"
#include <stdlib.h>

#include "pindefinitions.h"
#include "SPI.h"
#include "I2C.h"

#include <avr/io.h>
#include <util/twi.h>

static char str[20];
static const char string_nl[] PROGMEM = "\n";

// MARK: SPI

// SPI Test
const char menu_cmd_spitest_string[] PROGMEM = "spitest";
const char menu_help_spitest[] PROGMEM = "Run a test sequence on the 25LC1024\n";

static const char spitest_string_fin[] PROGMEM = " finished. ";
static const char spitest_string_hex[] PROGMEM = ": 0x";

static void do_eeprom_cmd (uint8_t* cmd, uint8_t bytes_out, uint8_t bytes_in, char* name, char* result) {
    static uint8_t id;
    uint8_t input[bytes_in];
    
    spi_start_half_duplex(&id, EEPROM_CS_NUM, cmd, bytes_out, input, bytes_in);
    
    while (!spi_transaction_done(id));
    
    serial_0_put_string(name);
    serial_0_put_string_P(spitest_string_fin);
    if (bytes_in != 0) {
        serial_0_put_string(result);
        serial_0_put_string_P(spitest_string_hex);
        ultoa(input[0], str, 16);
        serial_0_put_string(str);
    }
    serial_0_put_string_P(string_nl);
    spi_clear_transaction(id);
    
    while (!serial_0_out_buffer_empty());
}

void menu_cmd_spitest_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_spitest);
        return;
    }
    
    uint8_t id;
    uint8_t sr;
    
    uint8_t rdid_cmd[4] = {0b10101011, 0, 0, 0};
    do_eeprom_cmd(rdid_cmd, 4, 1, "RDID", "SIG");
    
    uint8_t rdsr_cmd[1] = {0b00000101};
    do_eeprom_cmd(rdsr_cmd, 1, 1, "RDSR", "Status");
    
    uint8_t wren_cmd[1] = {0b00000110};
    do_eeprom_cmd(wren_cmd, 1, 0, "WREN", "");
    
    //    uint8_t wrdi_cmd[1] = {0b00000100};
    //    do_eeprom_cmd(wrdi_cmd, 1, 0, "WRDI", " ");
    
    //    uint8_t wrsr_cmd[2] = {0b00000001, 0};
    //    do_eeprom_cmd(wrsr_cmd, 2, 0, "WRSR", " ");
    
    do_eeprom_cmd(rdsr_cmd, 1, 1, "RDSR", "Status");
    
    uint8_t write_cmd[5] = {0b00000010, 0, 0, 1, 0x12};
    do_eeprom_cmd(write_cmd, 5, 0, "WRITE", "");
    
    while (sr & 1) {
        spi_start_half_duplex(&id, EEPROM_CS_NUM, rdsr_cmd, 1, &sr, 1);
        while (!spi_transaction_done(id)) spi_service();
        spi_clear_transaction(id);
    }
    serial_0_put_string("Write done\n");
    
    uint8_t read_cmd[4] = {0b00000011, 0, 0, 1};
    do_eeprom_cmd(read_cmd, 4, 1, "READ", "Data");
}

// SPI Raw
const char menu_cmd_spiraw_string[] PROGMEM = "spiraw";
const char menu_help_spiraw[] PROGMEM = "Run a test sequence on the 25LC1024 without using the SPI buffer\n";

#ifdef ENABLE_SPI
static const char spiraw_string_enabled[] PROGMEM = "Cannot use spiraw, SPI queue is enabled.\n";
#else
static const char spiraw_string_sig[] PROGMEM = "Signature: 0x";
#endif

void menu_cmd_spiraw_handler(uint8_t arg_len, char** args)
{
#ifdef ENABLE_SPI
    serial_0_put_string_P(spiraw_string_enabled);
#else
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_spiraw);
        return;
    }
    
    SPCR |= (1<<SPE) | (1<<MSTR);
    
    // Read sig
    EEPROM_CS_PORT &= ~(1<<EEPROM_CS_NUM);
    SPDR = 0b10101011;
    while (!(SPSR & (1<<SPIF)));
    SPDR = 0b0;
    while (!(SPSR & (1<<SPIF)));
    SPDR = 0b0;
    while (!(SPSR & (1<<SPIF)));
    SPDR = 0b0;
    while (!(SPSR & (1<<SPIF)));
    SPDR = 0b0;
    while (!(SPSR & (1<<SPIF)));
    uint8_t result = SPDR;
    EEPROM_CS_PORT |= (1<<EEPROM_CS_NUM);
    
    serial_0_put_string_P(spiraw_string_sig);
    ultoa(result, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P(string_nl);
#endif
}

// SPI Conc
const char menu_cmd_spiconc_string[] PROGMEM = "spiconc";
const char menu_help_spiconc[] PROGMEM = "Run a test sequence on the 25LC1024, issuing multiple transactions to the spi queue concurrently.\n";

static const char spiconc_string_rdid[] PROGMEM = "Signature: 0x";
static const char spiconc_string_rdsr[] PROGMEM = "Status Register: 0b";
static const char spiconc_string_read[] PROGMEM = "Read: 0x";

void menu_cmd_spiconc_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_spiconc);
        return;
    }
    
    uint8_t rdid_id, rdsr_id, read_id, wren_id, rdsr2_id, write_id;
    uint8_t rdid_in, rdsr_in, read_in, rdsr2_in;
    
    uint8_t rdid_cmd[4] = {0b10101011, 0, 0, 0};
    uint8_t rdsr_cmd[1] = {0b00000101};
    uint8_t read_cmd[4] = {0b00000011, 0, 0, 1};
    uint8_t wren_cmd[1] = {0b00000110};
    uint8_t write_cmd[5] = {0b00000010, 0, 0, 1, 0xcc};
    
    spi_start_half_duplex(&rdid_id, EEPROM_CS_NUM, rdid_cmd, 4, &rdid_in, 1);
    spi_start_half_duplex(&rdsr_id, EEPROM_CS_NUM, rdsr_cmd, 1, &rdsr_in, 1);
    spi_start_half_duplex(&read_id, EEPROM_CS_NUM, read_cmd, 4, &read_in, 1);
    spi_start_half_duplex(&wren_id, EEPROM_CS_NUM, wren_cmd, 1, NULL, 0);
    spi_start_half_duplex(&rdsr2_id, EEPROM_CS_NUM, rdsr_cmd, 1, &rdsr2_in, 1);
    spi_start_half_duplex(&write_id, EEPROM_CS_NUM, write_cmd, 5, NULL, 0);
    
    while (!spi_transaction_done(rdid_id));
    serial_0_put_string_P(spiconc_string_rdid);
    ultoa(rdid_in, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P(string_nl);
    spi_clear_transaction(rdid_id);
    
    while (!spi_transaction_done(rdsr_id));
    serial_0_put_string_P(spiconc_string_rdsr);
    ultoa(rdsr_in, str, 2);
    serial_0_put_string(str);
    serial_0_put_string_P(string_nl);
    spi_clear_transaction(rdsr_id);
    
    while (!spi_transaction_done(read_id));
    serial_0_put_string_P(spiconc_string_read);
    ultoa(read_in, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P(string_nl);
    spi_clear_transaction(read_id);
    
    while (!spi_transaction_done(rdsr2_id));
    serial_0_put_string_P(spiconc_string_rdsr);
    ultoa(rdsr2_in, str, 2);
    serial_0_put_string(str);
    serial_0_put_string_P(string_nl);
    
    spi_clear_transaction(wren_id);
    spi_clear_transaction(rdsr2_id);
    spi_clear_transaction(write_id);
}


// MARK: I2C

// IIC Raw
const char menu_cmd_iicraw_string[] PROGMEM = "iicraw";
const char menu_help_iicraw[] PROGMEM = "Run a test sequence on the MPL3115A2 without using the I2C Buffer\n";

#ifdef ENABLE_I2C
static const char iicraw_string_enabled[] PROGMEM = "Cannot use iicraw, I2C queue is enabled.\n";
#else
static const char iicraw_string_sig[] PROGMEM = "WHO AM I: 0x";
static const char iicraw_string_error1[] PROGMEM = "An error occured while sending the start condition\n";
static const char iicraw_string_error2[] PROGMEM = "An error occured.\n";
#endif

void menu_cmd_iicraw_handler(uint8_t arg_len, char** args)
{
#ifdef ENABLE_I2C
    serial_0_put_string_P(iicraw_string_enabled);
#else
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_iicraw);
        return;
    }
    uint8_t result = 0;
    
    TWSR = (1<<TWEN);
    TWBR = 7;
    TWDR = 0xFF;
    
begin:
    serial_0_put_string("\tSending start condition\n");
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // Send start condition
    while (!(TWCR & (1<<TWINT)));
    switch (TW_STATUS) {
        case TW_REP_START:      // Shouldn't happen
        case TW_START:
            break;
        case TW_MT_ARB_LOST:    // Try to get arbitration again
            goto begin;
        default:
            serial_0_put_string_P(iicraw_string_error1);
            return;             // ?? bail out
    }
    
    serial_0_put_string("\tSending address + w\n");
    TWDR = 0b11000000 | TW_WRITE;   // Send address + w
    TWCR = (1<<TWINT) | (1<<TWEN);  // Clear interupt to start transmition
    while (!(TWCR & (1<<TWINT)));
    switch (TW_STATUS) {
        case TW_MT_SLA_ACK:
            break;
        case TW_MT_SLA_NACK:    // Device is busy, try again
            goto begin;
        case TW_MT_ARB_LOST:    // Try to get arbitration again
            goto begin;
        default:
            goto error;         // Send stop condition and exit gracefully
    }
    
    serial_0_put_string("\tSending register address\n");
    TWDR = 0x0c;                    // Register address (Who Am I)
    TWCR = (1<<TWINT) | (1<<TWEN);  // Clear interupt to start transmition
    while (!(TWCR & (1<<TWINT)));
    switch (TW_STATUS) {
        case TW_MT_DATA_ACK:
            break;
        case TW_MT_DATA_NACK:
            goto quit;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            goto error;         // Send stop condition and exit gracefully
    }
    serial_0_put_string("\tSending rep start\n");
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // Send repeated start condition to get into master recieve mode
    while (!(TWCR & (1<<TWINT)));
    switch (TW_STATUS) {
        case TW_START:
        case TW_REP_START:
            break;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            goto error;
    }
    
    serial_0_put_string("\tSending address + r\n");
    TWDR = 0b11000000 | TW_READ;    // Send address + r
    TWCR = (1<<TWINT) | (1<<TWEN);  // Clear interupt to start transmition
    while (!(TWCR & (1<<TWINT)));
    switch (TW_STATUS) {
        case TW_MR_SLA_ACK:
            break;
        case TW_MR_SLA_NACK:
            goto quit;
        case TW_MR_ARB_LOST:
            goto begin;
        default:
            goto error;
    }
    
    serial_0_put_string("\tRecieving byte\n");
    TWCR = _BV(TWINT) | _BV(TWEN);  // Send nac to stop getting data after first byte
    while (!(TWCR & (1<<TWINT)));
    switch (TW_STATUS) {
        case TW_MR_DATA_NACK:
        case TW_MR_DATA_ACK:
            result = TWDR;
            break;
        default:
            goto error;
    }
    
    serial_0_put_string_P(iicraw_string_sig);
    ultoa(result, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P(string_nl);
    
quit:
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); // send stop condition
    return;
error:
    serial_0_put_string_P(iicraw_string_error2);
    goto quit;
#endif
}
