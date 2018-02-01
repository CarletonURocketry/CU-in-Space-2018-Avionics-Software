//
//  menu_data.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "menu_data.h"

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "pindefinitions.h"
#include "menu_data.h"
#include "serial0.h"

#include "25LC1024.h"
#include "SPI.h"
#include "ADC.h"

// MARK: Strings
const char welcome_string[] PROGMEM = "CU In Space Avionics - 2018\tv1.0\n";
const char prompt_string[] PROGMEM = "> ";
const char menu_unkown_cmd_prt1[] PROGMEM = "Unkown command: \"";
const char menu_unkown_cmd_prt2[] PROGMEM = "\"\nUse \"help --list\" to get a list of avaliable commands.\n";

// MARK: Commands

// Help
static const char menu_cmd_help_string[] PROGMEM = "help";
static const char menu_help_help[] PROGMEM = "Get information about how to use commands.\nValid Usage: help <command name>\nUse help --list to list all valid commands.\n";

static const char help_string_unknown_one[] PROGMEM = "Unkown Command \"";
static const char help_string_unknown_two[] PROGMEM = "\"\n";

static const char help_list_all_string[] PROGMEM = "--list";

void menu_cmd_help_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 2) {
        serial_0_put_string_P(menu_help_help);
        return;
    }
    
    if (!strcasecmp_P(args[1], help_list_all_string)) {
        for (int i = 0; i < menu_num_items; i++) {
            serial_0_put_string_P((char*)pgm_read_word(&menu_items[i].string));
            serial_0_put_byte('\n');
        }
        return;
    }
    
    for (int i = 0; i < menu_num_items; i++) {
        if (!strcasecmp_P(args[1], (char*)pgm_read_word(&menu_items[i].string))) {
            serial_0_put_string_P((char*)pgm_read_word(&menu_items[i].help_string));
            return;
        }
    }
    
    serial_0_put_string_P(help_string_unknown_one);
    serial_0_put_string(args[1]);
    serial_0_put_string_P(help_string_unknown_two);
}

// Clear
static const char menu_cmd_clear_string[] PROGMEM = "clear";
static const char menu_help_clear[] PROGMEM = "Clear the terminal screen\n";

void menu_cmd_clear_handler(uint8_t arg_len, char** args)
{
    // Clear screen
    serial_0_put_byte(0x1B);
    serial_0_put_string("[2J");
    // Bring cursor home
    serial_0_put_byte(0x1B);
    serial_0_put_string("[H");
}

// Time
static const char menu_cmd_time_string[] PROGMEM = "time";
static const char menu_help_time[] PROGMEM = "Get current mission time.\n";

void menu_cmd_time_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_time);
        return;
    }
    
    char str[11];
    
    ultoa(millis, str, 10);
    serial_0_put_string(str);
    serial_0_put_byte('\n');
}

// EEPROM
static const char menu_cmd_eeprom_string[] PROGMEM = "eeprom";
static const char menu_help_eeprom[] PROGMEM = "Test external 25LC1024 EEPROM.\nValid Usage:\n\tRead: eeprom read <address>\n\tWrite: eprom write <address> <data>\n";

static const char eeprom_string_read[] PROGMEM = "read";
static const char eeprom_string_write[] PROGMEM = "write";

void menu_cmd_epprom_handler(uint8_t arg_len, char** args)
{
    if (arg_len < 3) {
        goto invalid_args;
    }
    
    if (!strcasecmp_P(args[1], eeprom_string_read)) {
        char* end;
        uint32_t addr = strtoul(args[2], &end, 0);
        if (*end != '\0') {
            goto invalid_args;
        }
        
        uint8_t id;
        uint32_t buffer;
        eeprom_25lc1024_read(&id, addr, 4, (uint8_t*)&buffer);
        
        while (!eeprom_25lc1024_transaction_done(id)) {
            eeprom_25lc1024_service();
        }
        
        char str[10];
        ultoa(buffer, str, 10);
        serial_0_put_string(str);
        serial_0_put_byte('\n');
    } else if (!strcasecmp_P(args[1], eeprom_string_write)) {
        if (arg_len < 4) {
            goto invalid_args;
        }
        
        char* end;
        uint32_t addr = strtoul(args[2], &end, 0);
        if (*end != '\0') {
            goto invalid_args;
        }

        uint32_t data = strtoul(args[3], &end, 0);
        if (*end != '\0') {
            goto invalid_args;
        }
        
        uint8_t id;
        eeprom_25lc1024_read(&id, addr, 4, (uint8_t*)&data);
        
        while (!eeprom_25lc1024_transaction_done(id)) {
            eeprom_25lc1024_service();
        }
    } else {
        goto invalid_args;
    }
    
    return;
    
invalid_args:
    serial_0_put_string_P(menu_help_eeprom);
}

// SPI Test
static const char menu_cmd_spitest_string[] PROGMEM = "spitest";
static const char menu_help_spitest[] PROGMEM = "Run a test sequence on the 25LC1024\n";


void menu_cmd_spitest_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_spitest);
        return;
    }
    
    uint8_t id;
    uint8_t rdid_cmd[4] = {0b10101011, 0, 0, 0};
    uint8_t input[1];

    spi_start_half_duplex(&id, EEPROM_CS_NUM, rdid_cmd, 4, input, 1);

    while (!spi_transaction_done(id)) {
        spi_service();
    }

    serial_0_put_string("RDID finished. SIG: 0x");
    char str[10];
    ultoa(input[0], str, 16);
    serial_0_put_string(str);
    serial_0_put_byte('\n');
    
    spi_clear_transaction(id);
    
    uint8_t wren_cmd[1] = {0b00000110};
    spi_start_half_duplex(&id, EEPROM_CS_NUM, wren_cmd, 1, NULL, 0);
    
    while (!spi_transaction_done(id)) {
        spi_service();
    }
    
    serial_0_put_string("WREN finished.\n");
    
    uint8_t write_cmd[5] = {0b00000110, 0, 0, 0, 0xff};
    spi_start_half_duplex(&id, EEPROM_CS_NUM, write_cmd, 1, NULL, 0);
    
    while (!spi_transaction_done(id)) {
        spi_service();
    }
    
    serial_0_put_string("WRITE finished.\n");
}

// Analog
static const char menu_cmd_analog_string[] PROGMEM = "analog";
static const char menu_help_analog[] PROGMEM = "Read analog inputs\n";

static const char analog_string_one[] PROGMEM = ": ";
static const char analog_string_two[] PROGMEM = " -> ";
static const char analog_string_three[] PROGMEM = " ºC\n";

void menu_cmd_analog_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_analog);
        return;
    }
    
    char str[8];
    for (int i = 0; i < ADC_NUM_CHANNELS; i++) {
        ultoa(i, str, 10);
        serial_0_put_string(str);
        serial_0_put_string_P(analog_string_one);
        ultoa(adc_avg_data[i], str, 10);
        serial_0_put_string(str);
        serial_0_put_string_P(analog_string_two);
        // t = (vout - v0) / tc
        // (((3.3/1024)*1024)-.5)/0.01
        dtostrf(((0.00322265625*(double)adc_avg_data[i]) - 0.5) / 0.01, 7, 2, str);
        serial_0_put_string(str);
        serial_0_put_string_P(analog_string_three);
    }
}

const uint8_t menu_num_items = 6;
const menu_item_t menu_items[] PROGMEM = {
    {.string = menu_cmd_help_string, .handler = menu_cmd_help_handler, .help_string = menu_help_help},
    {.string = menu_cmd_clear_string, .handler = menu_cmd_clear_handler, .help_string = menu_help_clear},
    {.string = menu_cmd_time_string, .handler = menu_cmd_time_handler, .help_string = menu_help_time},
    {.string = menu_cmd_eeprom_string, .handler = menu_cmd_epprom_handler, .help_string = menu_help_eeprom},
    {.string = menu_cmd_spitest_string, .handler = menu_cmd_spitest_handler, .help_string = menu_help_spitest},
    {.string = menu_cmd_analog_string, .handler = menu_cmd_analog_handler, .help_string = menu_help_analog}
};
