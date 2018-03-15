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

#include "arm_sense.h"

#include "25LC1024.h"
#include "SPI.h"
#include "I2C.h"
#include "ADC.h"

#include "Accel-ADXL343.h"
#include "Barometer-MPL3115A2.h"
#include "Gyro-FXAS21002C.h"
#include "GPS-FGPMMOPA6H.h"

static char str[25];

// MARK: Strings
const char welcome_string[] PROGMEM = "CU In Space Avionics - 2018\tv1.0\n";
const char prompt_string[] PROGMEM = "> ";
const char menu_unkown_cmd_prt1[] PROGMEM = "Unkown command: \"";
const char menu_unkown_cmd_prt2[] PROGMEM = "\"\nUse \"help --list\" to get a list of avaliable commands.\n";
static const char string_nl[] PROGMEM = "\n";

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
            serial_0_put_string_P(string_nl);
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

// Stat
static const char menu_cmd_stat_string[] PROGMEM = "stat";
static const char menu_help_stat[] PROGMEM = "Get status information\n";

static const char stat_str_time_units[] PROGMEM = " ms\n";

static const char stat_str_state_title[] PROGMEM = "Current State\n";
static const char stat_str_state_time[] PROGMEM = "\tMission Time: ";
static const char stat_str_state_state[] PROGMEM = "\tMain FSM State: ";
static const char stat_str_state_armed[] PROGMEM = "\tDeployment State: ";
static const char stat_str_state_armed_t[] PROGMEM = "ARMED\n";
static const char stat_str_state_armed_f[] PROGMEM = "DISARMED\n";

static const char stat_str_volt_title[] PROGMEM = "Voltages\n";
static const char stat_str_volt_bat[] PROGMEM = "\tBattery: ";
static const char stat_str_volt_cap[] PROGMEM = "\tCapacitor Bank: ";
static const char stat_str_volt_units[] PROGMEM = " V\n";

static const char stat_str_times_title[] PROGMEM = "Last Sample Times\n";
static const char stat_str_times_alt[] PROGMEM = "\tBarometric Altimiter: ";
static const char stat_str_times_accel[] PROGMEM = "\tAccelerometer: ";
static const char stat_str_times_gyro[] PROGMEM = "\tGyroscope: ";
static const char stat_str_times_gps[] PROGMEM = "\tGPS: ";

void menu_cmd_stat_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_stat);
        return;
    }
    
    // State
    serial_0_put_string_P(stat_str_state_title);
    // Time
    serial_0_put_string_P(stat_str_state_time);
    ultoa(millis, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_time_units);
    // FSM State
    serial_0_put_string_P(stat_str_state_state);
    serial_0_put_string_P(string_nl);
    // Is armed
    serial_0_put_string_P(stat_str_state_armed);
    if (is_armed()) {
        serial_0_put_string_P(stat_str_state_armed_t);
    } else {
        serial_0_put_string_P(stat_str_state_armed_f);
    }
    while (!serial_0_out_buffer_empty());
    
    // Voltages
    serial_0_put_string_P(stat_str_volt_title);
    // Battery Voltage
    serial_0_put_string_P(stat_str_volt_bat);
    dtostrf(0.01501651249 * (double)adc_avg_data[ADC_NUM_CHANNELS - 1], 7, 3, str);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_volt_units);
    // Capacitor Voltage
    serial_0_put_string_P(stat_str_volt_cap);
    dtostrf(0.02560460069 * (double)adc_avg_data[0], 7, 3, str);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_volt_units);
    while (!serial_0_out_buffer_empty());
    
    // Last Sample Times
    serial_0_put_string_P(stat_str_times_title);
    // Altimiter
    serial_0_put_string_P(stat_str_times_alt);
    ultoa(mpl3115a2_sample_time, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_time_units);
    // Accelerometer
    serial_0_put_string_P(stat_str_times_accel);
    ultoa(adxl343_sample_time, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_time_units);
    // Gyroscope
    serial_0_put_string_P(stat_str_times_gyro);
    ultoa(fxas21002c_sample_time, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_time_units);
    // GPS
    serial_0_put_string_P(stat_str_times_gps);
    ultoa(fgpmmopa6h_sample_time, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_time_units);
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
        
        ultoa(buffer, str, 10);
        serial_0_put_string(str);
        serial_0_put_string_P(string_nl);
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

static const char spitest_string_fin[] PROGMEM = " finished. ";
static const char spitest_string_hex[] PROGMEM = ": 0x";

static void do_eeprom_cmd (uint8_t* cmd, uint8_t bytes_out, uint8_t bytes_in, char* name, char* result) {
    static uint8_t id;
    uint8_t input[bytes_in];
    
    spi_start_half_duplex(&id, EEPROM_CS_NUM, cmd, bytes_out, input, bytes_in);
    
    while (!spi_transaction_done(id)) {
        spi_service();
    }
    
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

    uint8_t rdid_cmd[4] = {0b10101011, 0, 0, 0};
    do_eeprom_cmd(rdid_cmd, 4, 1, "RDID", "SIG");

//    uint8_t rdsr_cmd[1] = {0b00000101};
//    do_eeprom_cmd(rdsr_cmd, 1, 1, "RDSR", "Status");
    
//    uint8_t wren_cmd[1] = {0b00000110};
//    do_eeprom_cmd(wren_cmd, 1, 0, "WREN", "");
    
//    uint8_t wrdi_cmd[1] = {0b00000100};
//    do_eeprom_cmd(wrdi_cmd, 1, 0, "WRDI", "");
//
//    uint8_t wrsr_cmd[2] = {0b00000001, 0};
//    do_eeprom_cmd(wrsr_cmd, 2, 0, "WRSR", "");

    
//    do_eeprom_cmd(rdsr_cmd, 1, 1, "RDSR", "Status");

//    uint8_t read_cmd[4] = {0b00000011, 0, 0, 1};
//    do_eeprom_cmd(read_cmd, 4, 1, "READ", "Data");

//    uint8_t write_cmd[5] = {0b00000110, 0, 0, 0, 0x67};
//    do_eeprom_cmd(write_cmd, 5, 0, "WRITE", "");
}

// SPI Raw
static const char menu_cmd_spiraw_string[] PROGMEM = "spiraw";
static const char menu_help_spiraw[] PROGMEM = "Run a test sequence on the 25LC1024 without using the SPI buffer\n";

static const char spiraw_string_sig[] PROGMEM = "Signature: 0x";

void menu_cmd_spiraw_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_spiraw);
        return;
    }
    
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
    uint8_t sig = SPDR;
    EEPROM_CS_PORT |= (1<<EEPROM_CS_NUM);
    
    serial_0_put_string_P(spiraw_string_sig);
    ultoa(sig, str, 16);
    serial_0_put_string(str);
}

// Analog
static const char menu_cmd_analog_string[] PROGMEM = "analog";
static const char menu_help_analog[] PROGMEM = "Read analog inputs\n";

static const char analog_string_one[] PROGMEM = ": ";
static const char analog_string_two[] PROGMEM = " -> ";
static const char analog_string_three[] PROGMEM = " ºC\n";
static const char analog_string_four[] PROGMEM = " Vbat\n";
static const char analog_string_five[] PROGMEM = " Vcap\n";

void menu_cmd_analog_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_analog);
        return;
    }
    
    for (int i = 0; i < ADC_NUM_CHANNELS; i++) {
        while (!serial_0_out_buffer_empty());
        
        ultoa(i, str, 10);
        serial_0_put_string(str);
        serial_0_put_string_P(analog_string_one);
        ultoa(adc_avg_data[i], str, 10);
        serial_0_put_string(str);
        
        if (i == 0) {
            // Capacitor Voltage
            serial_0_put_string_P(analog_string_two);
            // Vcap = ((3.3/1024)*n)/(6.75/(6.75+46.88))
            dtostrf(0.02560460069 * (double)adc_avg_data[i], 7, 3, str);
            serial_0_put_string(str);
            serial_0_put_string_P(analog_string_five);
        } else if (i == 1 || i == 2) {
            // Temp
            serial_0_put_string_P(analog_string_two);
            // t = (vout - v0) / tc
            // (((3.3/1024)*1024)-.5)/0.01
            dtostrf(((0.00322265625*(double)adc_avg_data[i]) - 0.5) * 100.0, 7, 2, str);
            serial_0_put_string(str);
            serial_0_put_string_P(analog_string_three);
        } else if (i == (ADC_NUM_CHANNELS - 1)) {
            // Battery Voltage
            serial_0_put_string_P(analog_string_two);
            // Vbat = ((3.3/1024)*n)/(5.383/(5.383+19.7))
            dtostrf(0.01501651249 * (double)adc_avg_data[i], 7, 3, str);
            serial_0_put_string(str);
            serial_0_put_string_P(analog_string_four);
        } else {
            serial_0_put_string_P(string_nl);
        }
    }
}

// Sensors
static const char menu_cmd_sensors_string[] PROGMEM = "sensors";
static const char menu_help_sensors[] PROGMEM = "Read from sensors\n";

static const char sensors_str_baro_title[] PROGMEM = "Barometric Altimiter (MPL3115A2)\n";
static const char sensors_str_baro_alt[] PROGMEM = "\tAltitude: ";
static const char sensors_str_temp[] PROGMEM = "\tTempurature: ";
static const char sensors_str_accel_title[] PROGMEM = "Accelerometer (ADXL343)\n";
static const char sensors_str_accel_x[] PROGMEM = "\tX: ";
static const char sensors_str_accel_y[] PROGMEM = "\tY: ";
static const char sensors_str_accel_z[] PROGMEM = "\tZ: ";
static const char sensors_str_gyro_title[] PROGMEM = "Gyroscope (FXAS21002)\n";
static const char sensors_str_gyro_pitch[] PROGMEM = "\tPitch: ";
static const char sensors_str_gyro_roll[] PROGMEM = "\tRoll: ";
static const char sensors_str_gyro_yaw[] PROGMEM = "\tYaw: ";

static const char sensors_str_temp_units[] PROGMEM = " ºC\n";
static const char sensors_str_alt_units[] PROGMEM = " m\n";
static const char sensors_str_accel_units[] PROGMEM = " g\n";
static const char sensors_str_gyro_units[] PROGMEM = " º/s\n";

void menu_cmd_sensors_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_sensors);
        return;
    }
    
    double val = 0;

    serial_0_put_string_P(sensors_str_baro_title);
    serial_0_put_string_P(sensors_str_baro_alt);
    val = ((double)(mpl3115a2_alt_csb + (((uint16_t)mpl3115a2_alt_msb) << 8))) +
                    (((double)(mpl3115a2_alt_lsb >> 4)) / 16);
    dtostrf(val, 12, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_alt_units);
    serial_0_put_string_P(sensors_str_temp);
    val = ((double)(mpl3115a2_temp_msb + (((double)(mpl3115a2_temp_lsb >> 4)) / 16)));
    dtostrf(val, 9, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_temp_units);
    while (!serial_0_out_buffer_empty());
    
    serial_0_put_string_P(sensors_str_accel_title);
    serial_0_put_string_P(sensors_str_accel_x);
    val = (double)adxl343_accel_x * 0.0039; // 3.9 milli-g per LSB
    dtostrf(val, 8, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_accel_units);
    serial_0_put_string_P(sensors_str_accel_y);
    val = (double)adxl343_accel_y * 0.0039; // 3.9 milli-g per LSB
    dtostrf(val, 8, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_accel_units);
    serial_0_put_string_P(sensors_str_accel_z);
    val = (double)adxl343_accel_z * 0.0039; // 3.9 milli-g per LSB
    dtostrf(val, 8, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_accel_units);
    while (!serial_0_out_buffer_empty());
    
    serial_0_put_string_P(sensors_str_gyro_title);
    serial_0_put_string_P(sensors_str_gyro_pitch);
    val = (double)fxas21002c_pitch_rate * 0.0625;
    dtostrf(val, 9, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_gyro_units);
    serial_0_put_string_P(sensors_str_gyro_roll);
    val = (double)fxas21002c_roll_rate * 0.0625;
    dtostrf(val, 9, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_gyro_units);
    serial_0_put_string_P(sensors_str_gyro_yaw);
    val = (double)fxas21002c_yaw_rate * 0.0625;
    dtostrf(val, 9, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_gyro_units);
    serial_0_put_string_P(sensors_str_temp);
    itoa(fxas21002c_temp, str , 10);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_temp_units);
}

// actest
static const char menu_cmd_actest_string[] PROGMEM = "actest";
static const char menu_help_actest[] PROGMEM = "Test ADXL343 Accelerometer\n";

static const char actest_devid[] PROGMEM = "Device ID: 0x";

void menu_cmd_actest_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_actest);
        return;
    }
    
    uint8_t transaction_id;
    uint8_t buffer;
    
    i2c_read(&transaction_id, 0x53, 0x0, &buffer, 1);
    while (!i2c_transaction_done(transaction_id)) i2c_service();
    serial_0_put_string_P(actest_devid);
    ultoa(buffer, str, 16);
    serial_0_put_string(str);
    i2c_clear_transaction(transaction_id);
    serial_0_put_string_P(string_nl);
}

// altest
static const char menu_cmd_altest_string[] PROGMEM = "altest";
static const char menu_help_altest[] PROGMEM = "Test MPL3115A2 Barometric Altimiter\n";

static const char altest_devid[] PROGMEM = "Who Am I: 0x";

void menu_cmd_altest_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_altest);
        return;
    }
    
    uint8_t transaction_id;
    uint8_t buffer;
    
    i2c_read(&transaction_id, 0x60, 0x0c, &buffer, 1);
    while (!i2c_transaction_done(transaction_id)) i2c_service();
    serial_0_put_string_P(altest_devid);
    ultoa(buffer, str, 16);
    serial_0_put_string(str);
    i2c_clear_transaction(transaction_id);
    serial_0_put_string_P(string_nl);
}


const uint8_t menu_num_items = 10;
const menu_item_t menu_items[] PROGMEM = {
    {.string = menu_cmd_help_string, .handler = menu_cmd_help_handler, .help_string = menu_help_help},
    {.string = menu_cmd_clear_string, .handler = menu_cmd_clear_handler, .help_string = menu_help_clear},
    {.string = menu_cmd_stat_string, .handler = menu_cmd_stat_handler, .help_string = menu_help_stat},
    {.string = menu_cmd_eeprom_string, .handler = menu_cmd_epprom_handler, .help_string = menu_help_eeprom},
    {.string = menu_cmd_spitest_string, .handler = menu_cmd_spitest_handler, .help_string = menu_help_spitest},
    {.string = menu_cmd_spiraw_string, .handler = menu_cmd_spiraw_handler, .help_string = menu_help_spiraw},
    {.string = menu_cmd_analog_string, .handler = menu_cmd_analog_handler, .help_string = menu_help_analog},
    {.string = menu_cmd_sensors_string, .handler = menu_cmd_sensors_handler, .help_string = menu_help_sensors},
    {.string = menu_cmd_actest_string, .handler = menu_cmd_actest_handler, .help_string = menu_help_actest},
    {.string = menu_cmd_altest_string, .handler = menu_cmd_altest_handler, .help_string = menu_help_altest}
};
