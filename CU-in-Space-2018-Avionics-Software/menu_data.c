//
//  menu_data.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "menu_data.h"

#include <stdlib.h>
#include <string.h>

#include <util/atomic.h>
#include <avr/wdt.h>

#include "global.h"
#include "pindefinitions.h"
#include "serial0.h"

#include "bus_tests.h"

#include "ematch_detect.h"

#include "serial1.h"
#include "25LC1024.h"
#include "SPI.h"
#include "I2C.h"
#include "ADC.h"
#include "EEPROM.h"
#include "XBee.h"

#include "Accel-ADXL343.h"
#include "Barometer-MPL3115A2.h"
#include "Gyro-FXAS21002C.h"
#include "GPS-FGPMMOPA6H.h"

#include "telemetry_format.h"


#define STR_LEN 128
static char str[STR_LEN];

// MARK: Strings

const char prompt_string[] PROGMEM = "> ";
const char menu_unkown_cmd_prt1[] PROGMEM = "Unkown command: \"";
const char menu_unkown_cmd_prt2[] PROGMEM = "\"\nUse \"help --list\" to get a list of avaliable commands.\n";
static const char string_nl[] PROGMEM = "\n";
static const char string_on[] PROGMEM = "on";
static const char string_off[] PROGMEM = "off";

// MARK: Welcome
const char welcome_string[] PROGMEM = "CU InSpace 2018 Avionics Software -> ";
const char version_string[] PROGMEM = "Built "__DATE__" at "__TIME__" with avr-gcc "__VERSION__"\n";

const char welcome_reset_title[] PROGMEM = "Reset due to: ";

static const char str_reset_poweron[] PROGMEM = "Power On\n";
static const char str_reset_external[] PROGMEM = "External Reset (Reset Pin)\n";
static const char str_reset_brownout[] PROGMEM = "Brownout Detected\n";
static const char str_reset_watchdog[] PROGMEM = "Watchdog Timer\n";
static const char str_reset_jtag[] PROGMEM = "JTAG\n";

void menu_print_welcome (void)
{
    serial_0_put_string_P(welcome_string);
    serial_0_put_string_P(version_string);
    
    serial_0_put_string_P(welcome_reset_title);
    switch (reset_type) {
        case JTAG:
            serial_0_put_string_P(str_reset_jtag);
            break;
        case WATCHDOG:
            serial_0_put_string_P(str_reset_watchdog);
            break;
        case BROWNOUT:
            serial_0_put_string_P(str_reset_brownout);
            break;
        case EXTERNAL:
            serial_0_put_string_P(str_reset_external);
            break;
        case POWERON:
            serial_0_put_string_P(str_reset_poweron);
            break;
    }
}

// MARK: Commands

// Version
static const char menu_cmd_version_string[] PROGMEM = "version";
static const char menu_help_version[] PROGMEM = "Get information about the version of this software\n";

void menu_cmd_version_handler(uint8_t arg_len, char** args)
{
    serial_0_put_string_P(welcome_string);
    serial_0_put_string_P(version_string);
}

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

// Reset
static const char menu_cmd_reset_string[] PROGMEM = "reset";
static const char menu_help_reset[] PROGMEM = "Performs a software reset with the watchdog timer.\n";

void menu_cmd_reset_handler(uint8_t arg_len, char** args)
{
    wdt_enable(WDTO_15MS);
    for (;;);
}

// Stat
static const char menu_cmd_stat_string[] PROGMEM = "stat";
static const char menu_help_stat[] PROGMEM = "Get status information\n";

static const char stat_str_time_units[] PROGMEM = " ms\n";

static const char stat_str_state_title[] PROGMEM = "Current State\n";
static const char stat_str_state_time[] PROGMEM = "\tMission Time: ";
static const char stat_str_state_state[] PROGMEM = "\tMain FSM State: ";
static const char stat_str_state_state_stby[] PROGMEM = "STANDBY (0x";
static const char stat_str_state_state_pf[] PROGMEM = "PRE-FLIGHT (0x";
static const char stat_str_state_state_pa[] PROGMEM = "POWERED ASCENT (0x";
static const char stat_str_state_state_ca[] PROGMEM = "COASTING ASCENT (0x";
static const char stat_str_state_state_des[] PROGMEM = "DESCENT (0x";
static const char stat_str_state_state_rec[] PROGMEM = "RECOVERY (0x";
static const char stat_str_state_state_unkown[] PROGMEM = "UNKNOWN (0x";
static const char stat_str_state_end[] PROGMEM = ")\n";
static const char stat_str_state_ematch_1[] PROGMEM = "\tE-Match 1: ";
static const char stat_str_state_ematch_2[] PROGMEM = "\tE-Match 2: ";
static const char stat_str_state_ematch_t[] PROGMEM = "READY\n";
static const char stat_str_state_ematch_f[] PROGMEM = "NOT PRESENT\n";

static const char stat_str_volt_title[] PROGMEM = "Voltages\n";
static const char stat_str_volt_bat[] PROGMEM = "\tBattery: ";
static const char stat_str_volt_cap[] PROGMEM = "\tCapacitor Bank: ";
static const char stat_str_volt_units[] PROGMEM = " V\n";

static const char stat_str_times_title[] PROGMEM = "Last Sample Times\n";
static const char stat_str_times_alt[] PROGMEM = "\tBarometric Altimiter: ";
static const char stat_str_times_accel[] PROGMEM = "\tAccelerometer: ";
static const char stat_str_times_gyro[] PROGMEM = "\tGyroscope: ";
static const char stat_str_times_gps[] PROGMEM = "\tGPS: ";

static const char stat_str_reset_title[] PROGMEM = "Last Reset Due To: ";

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
    switch (fsm_state) {
        case STANDBY:
            serial_0_put_string_P(stat_str_state_state_stby);
            break;
        case PRE_FLIGHT:
            serial_0_put_string_P(stat_str_state_state_pf);
            break;
        case POWERED_ASCENT:
            serial_0_put_string_P(stat_str_state_state_pa);
            break;
        case COASTING_ASCENT:
            serial_0_put_string_P(stat_str_state_state_ca);
            break;
        case DESCENT:
            serial_0_put_string_P(stat_str_state_state_des);
            break;
        case RECOVERY:
            serial_0_put_string_P(stat_str_state_state_rec);
            break;
        default:
            serial_0_put_string_P(stat_str_state_state_unkown);
            break;
    }
    utoa(fsm_state, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_state_end);
    // E-Matches present
    serial_0_put_string_P(stat_str_state_ematch_1);
    serial_0_put_string_P((ematch_1_is_ready()) ? stat_str_state_ematch_t : stat_str_state_ematch_f);
    serial_0_put_string_P(stat_str_state_ematch_2);
    serial_0_put_string_P((ematch_2_is_ready()) ? stat_str_state_ematch_t : stat_str_state_ematch_f);
    while (!serial_0_out_buffer_empty());
    
    // Voltages
    serial_0_put_string_P(stat_str_volt_title);
    // Battery Voltage
    serial_0_put_string_P(stat_str_volt_bat);
    dtostrf(0.01434657506 * (double)adc_avg_data[ADC_NUM_CHANNELS - 1], 7, 3, str);
    serial_0_put_string(str);
    serial_0_put_string_P(stat_str_volt_units);
    // Capacitor Voltage
    serial_0_put_string_P(stat_str_volt_cap);
    dtostrf(0.02625071131 * (double)adc_avg_data[0], 7, 3, str);
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
    
    // Reset Reason
    serial_0_put_string_P(stat_str_reset_title);
    switch (reset_type) {
        case JTAG:
            serial_0_put_string_P(str_reset_jtag);
            break;
        case WATCHDOG:
            serial_0_put_string_P(str_reset_watchdog);
            break;
        case BROWNOUT:
            serial_0_put_string_P(str_reset_brownout);
            break;
        case EXTERNAL:
            serial_0_put_string_P(str_reset_external);
            break;
        case POWERON:
            serial_0_put_string_P(str_reset_poweron);
            break;
    }
}

// EEPROM
static const char menu_cmd_eeprom_string[] PROGMEM = "eeprom";
static const char menu_help_eeprom[] PROGMEM = "Test external 25LC1024 EEPROM.\nValid Usage:\n\tRead: eeprom read <address>\n\tWrite: eeprom write <address> <data>\n\tErase: eeprom erase\n";

static const char eeprom_string_read[] PROGMEM = "read";
static const char eeprom_string_write[] PROGMEM = "write";
static const char eeprom_string_erase[] PROGMEM = "erase";
static const char eeprom_string_dump[] PROGMEM = { 0x5 };

static const char eeprom_string_hex[] PROGMEM = "0x";

void menu_cmd_epprom_handler(uint8_t arg_len, char** args)
{
    if (arg_len < 2) {
        goto invalid_args;
    }
    
    uint8_t id;
    char* end;
    
    if (!strcasecmp_P(args[1], eeprom_string_read)) {
        if (arg_len != 3) {
            goto invalid_args;
        }
        
        uint32_t addr = strtoul(args[2], &end, 0);
        if (*end != '\0') {
            goto invalid_args;
        }
        
        uint32_t buffer;
        eeprom_25lc1024_read(&id, addr, 4, (uint8_t*)&buffer);
        
        while (!eeprom_25lc1024_transaction_done(id)) eeprom_25lc1024_service();
        
        serial_0_put_string_P(eeprom_string_hex);
        ultoa(buffer, str, 16);
        serial_0_put_string(str);
        serial_0_put_string_P(string_nl);
    } else if (!strcasecmp_P(args[1], eeprom_string_write)) {
        if (arg_len != 4) {
            goto invalid_args;
        }
        
        uint32_t addr = strtoul(args[2], &end, 0);
        if (*end != '\0') {
            goto invalid_args;
        }

        uint32_t data = strtoul(args[3], &end, 0);
        if (*end != '\0') {
            goto invalid_args;
        }
        
        eeprom_25lc1024_write(&id, addr, 4, (uint8_t*)&data);
        
        while (!eeprom_25lc1024_transaction_done(id)) eeprom_25lc1024_service();
    } else if (!strcasecmp_P(args[1], eeprom_string_erase)) {
        if (arg_len != 2) {
            goto invalid_args;
        }
        
        eeprom_25lc1024_chip_erase(&id);
        
        while (!eeprom_25lc1024_transaction_done(id)) eeprom_25lc1024_service();
    } else if (!strcasecmp_P(args[1], eeprom_string_dump)) {
        if (arg_len != 2) {
            return;
        }
        
        uint8_t step = (128 > STR_LEN) ? STR_LEN : 128;
        
        for (int i = 0; i < (2 * 0x1FFFF); i += step) {
            eeprom_read(&id, i, (uint8_t*)str, step);
            
            while (!serial_0_out_buffer_empty()) eeprom_25lc1024_service();
            while (!eeprom_25lc1024_transaction_done(id)) eeprom_25lc1024_service();
            
            eeprom_25lc1024_clear_transaction(id);
            
            serial_0_put_string(str);
        }
        return;
    } else {
        goto invalid_args;
    }
    
    eeprom_25lc1024_clear_transaction(id);
    
    return;
    
invalid_args:
    serial_0_put_string_P(menu_help_eeprom);
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
            // Vcap = ((3.3/1024)*n)/(6.59/(6.59+47.09))
            dtostrf(0.02625071131 * (double)adc_avg_data[i], 7, 3, str);
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
            // Vbat = ((3.3/1024)*n)/(5.6/(5.6+19.33))
            dtostrf(0.01434657506 * (double)adc_avg_data[i], 7, 3, str);
            serial_0_put_string(str);
            serial_0_put_string_P(analog_string_four);
        } else {
            serial_0_put_string_P(string_nl);
        }
    }
}

// Sensors
static const char menu_cmd_sensors_string[] PROGMEM = "sensors";
static const char menu_help_sensors[] PROGMEM = "Print data from sensors\n";

static const char sensors_str_baro_title[] PROGMEM = "Barometric Altimeter (MPL3115A2)\n";
static const char sensors_str_baro_alt[] PROGMEM = "\tAltitude: ";
static const char sensors_str_temp[] PROGMEM = "\tTempurature: ";
static const char sensors_str_raw_1[] PROGMEM = " csb: 0b";
static const char sensors_str_raw_2[] PROGMEM = " lsb: 0b";
static const char sensors_str_raw_3[] PROGMEM = ")\n";
static const char sensors_str_accel_title[] PROGMEM = "Accelerometer (ADXL343)\n";
static const char sensors_str_accel_x[] PROGMEM = "\tX: ";
static const char sensors_str_accel_y[] PROGMEM = "\tY: ";
static const char sensors_str_accel_z[] PROGMEM = "\tZ: ";
static const char sensors_str_gyro_title[] PROGMEM = "Gyroscope (FXAS21002)\n";
static const char sensors_str_gyro_pitch[] PROGMEM = "\tPitch: ";
static const char sensors_str_gyro_roll[] PROGMEM = "\tRoll: ";
static const char sensors_str_gyro_yaw[] PROGMEM = "\tYaw: ";

static const char sensors_str_temp_units[] PROGMEM = " ºC\n";
static const char sensors_str_alt_units[] PROGMEM = " m\t(msb: 0b";
static const char sensors_str_accel_units[] PROGMEM = " g\n";
static const char sensors_str_gyro_units[] PROGMEM = " º/s\n";

void menu_cmd_sensors_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_sensors);
        return;
    }
    
    double val = 0;

    // Altimeter
    serial_0_put_string_P(sensors_str_baro_title);
    serial_0_put_string_P(sensors_str_baro_alt);
    val = ((double)(mpl3115a2_alt_csb + (((uint16_t)mpl3115a2_alt_msb) << 8))) +
                    (((double)(mpl3115a2_alt_lsb >> 4)) / 16);
    dtostrf(val, 12, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_alt_units);
    utoa(mpl3115a2_alt_msb, str, 2);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_raw_1);
    utoa(mpl3115a2_alt_csb, str, 2);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_raw_2);
    utoa(mpl3115a2_alt_lsb, str, 2);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_raw_3);
    serial_0_put_string_P(sensors_str_temp);
    val = ((double)(mpl3115a2_temp_msb + (((double)(mpl3115a2_temp_lsb >> 4)) / 16)));
    dtostrf(val, 9, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(sensors_str_temp_units);
    while (!serial_0_out_buffer_empty());
    
    // Accelerometer
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
    
    // Gyroscope
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

// GPS
static const char menu_cmd_gps_string[] PROGMEM = "gps";
static const char menu_help_gps[] PROGMEM = "Read from GPS\n";

static const char gps_str_title[] PROGMEM = "GPS Data\n";
static const char gps_str_mission_time[] PROGMEM = "\tTime since last valid RMC packet: ";
static const char gps_str_mission_time_units[] PROGMEM = " ms\n";
static const char gps_str_colon[] PROGMEM = ":";
static const char gps_str_time[] PROGMEM = "\tUTC Time: ";
static const char gps_str_space[] PROGMEM = " ";
static const char gps_str_loc[] PROGMEM = "\n\tLocation: ";
static const char gps_str_lat_N[] PROGMEM = "N   ";
static const char gps_str_lat_S[] PROGMEM = "S   ";
static const char gps_str_long_E[] PROGMEM = "E\n";
static const char gps_str_long_W[] PROGMEM = "W\n";
static const char gps_str_lat[] PROGMEM = "\tLatitude: ";
static const char gps_str_long[] PROGMEM = "\tLongitude: ";
static const char gps_str_coord_units[] PROGMEM = " 100 micro-minutes\n";
static const char gps_str_speed[] PROGMEM = "\tSpeed over Ground: ";
static const char gps_str_speed_units[] PROGMEM = " centi-knots\n";
static const char gps_str_course[] PROGMEM = "\tCourse: ";
static const char gps_str_course_units[] PROGMEM = " centi-degrees\n";
static const char gps_str_sats[] PROGMEM = "\tSatellites in View: ";
static const char gps_str_valid[] PROGMEM = "\tData Validity: ";
static const char gps_str_valid_0[] PROGMEM = "0";
static const char gps_str_valid_1[] PROGMEM = "1";

void menu_cmd_gps_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_gps);
        return;
    }
    
    serial_0_put_string_P(gps_str_title);
    
    // Mission Time
    serial_0_put_string_P(gps_str_mission_time);
    ultoa(millis - fgpmmopa6h_sample_time, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_mission_time_units);
    
    // UTC Time
    serial_0_put_string_P(gps_str_time);
    uint32_t hours = fgpmmopa6h_utc_time / 3600000;
    uint32_t hours_rem = fgpmmopa6h_utc_time % 3600000;
    uint32_t mins = hours_rem / 60000;
    uint32_t minutes_rem = hours_rem % 60000;
    double seconds = ((double)minutes_rem) / 1000.0;
    
    ltoa(hours, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_colon);
    ltoa(mins, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_colon);
    dtostrf(seconds, 6, 3, str);
    serial_0_put_string(str);
    
    // Latitude
    serial_0_put_string_P(gps_str_loc);
    uint8_t south = fgpmmopa6h_latitude < 0;
    int32_t lat = south ? (fgpmmopa6h_latitude * -1) : fgpmmopa6h_latitude;
    
    int32_t degrees = lat / 600000;
    double minutes = ((double)(lat % 600000)) / 10000.0;
    
    ltoa(degrees, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_space);
    dtostrf(minutes, 7, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(south ? gps_str_lat_S : gps_str_lat_N);
    
    // Longitude
    uint8_t west = fgpmmopa6h_longitude < 0;
    int32_t lng = west ? (fgpmmopa6h_longitude * -1) : fgpmmopa6h_longitude;
    
    degrees = lng / 600000;
    minutes = ((double)(lng % 600000)) / 10000.0;
    
    ltoa(degrees, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_space);
    dtostrf(minutes, 7, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(west ? gps_str_long_W : gps_str_long_E);
    
    // Raw latitude
    serial_0_put_string_P(gps_str_lat);
    ltoa(fgpmmopa6h_latitude, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_coord_units);
    
    while (!serial_0_out_buffer_empty());
    
    // Raw longitude
    serial_0_put_string_P(gps_str_long);
    ltoa(fgpmmopa6h_longitude, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_coord_units);
    
    // Ground Speed
    serial_0_put_string_P(gps_str_speed);
    itoa(fgpmmopa6h_speed, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_speed_units);
    
    // Course
    serial_0_put_string_P(gps_str_course);
    itoa(fgpmmopa6h_course, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(gps_str_course_units);
    
    // Sattelites in View
    serial_0_put_string_P(gps_str_sats);
    itoa(fgpmmopa6h_satellites_in_view, str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(string_nl);
    
    // Valid
    serial_0_put_string_P(gps_str_valid);
    for (int8_t i = 7; i >= 0; i--) {
        serial_0_put_string_P((fgpmmopa6h_data_valid & (1<<i)) ? gps_str_valid_1 : gps_str_valid_0);
    }
    serial_0_put_string_P(string_nl);
}

// GPS serial
static const char menu_cmd_gps_ser_string[] PROGMEM = "gpsser";
static const char menu_help_gps_ser[] PROGMEM = "Manually communicate with GPS\nValid Usage:\n\tRead last line: gpsser read\n\tWrite: gpsser write <command>\n\tRead Continuously: gpsser readcont <time in seconds>\n";

#ifdef ENABLE_GPS
static const char gps_ser_string_enabled[] PROGMEM = "Cannot use gpsser, GPS driver is enabled.\n";
#else
static const char gps_ser_read[] PROGMEM = "read";
static const char gps_ser_write[] PROGMEM = "write";
static const char gps_ser_readcont[] PROGMEM = "readcont";
static const char gps_ser_noline[] PROGMEM = "No Data Avaliable\n";
static const char gps_ser_cmdend[]  PROGMEM = "\r\n";
#endif

void menu_cmd_gps_ser_handler(uint8_t arg_len, char** args)
{
#ifdef ENABLE_GPS
    serial_0_put_string_P(gps_ser_string_enabled);
#else
    if (arg_len == 1) {
        goto invalid_args;
    }
    
    if (!strcasecmp_P(args[1], gps_ser_read)) {
        if (serial_1_has_line('\n')) {
            serial_1_get_line('\n', str, STR_LEN);
            str[strlen(str)-1] = '\n';
            serial_0_put_string(str);
        } else {
            serial_0_put_string_P(gps_ser_noline);
        }
    } else if (!strcasecmp_P(args[1], gps_ser_write)) {
        if (arg_len != 3) {
            goto invalid_args;
        }
        
        serial_1_put_string(args[2]);
        serial_1_put_string_P(gps_ser_cmdend);
    } else if (!strcasecmp_P(args[1], gps_ser_readcont)) {
        if (arg_len != 3) {
            goto invalid_args;
        }
        
        char* end;
        uint32_t time = strtoul(args[2], &end, 0);
        if (*end != '\0') {
            goto invalid_args;
        }
        
        for (time = (time * 1000) + millis; millis <= time;) {
            if (serial_1_has_line('\n')) {
                serial_1_get_line('\n', str, STR_LEN);
                str[strlen(str)-1] = '\n';
                serial_0_put_string(str);
            }
            wdt_reset();
        }
        
    } else {
        goto invalid_args;
    }
    return;
    
invalid_args:
    serial_0_put_string_P(menu_help_gps_ser);
#endif
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

static const char altest_space[] PROGMEM = " ";

void menu_cmd_altest_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_altest);
        return;
    }
    
    uint8_t transaction_id;
    uint8_t buffer[5];
    
    i2c_read(&transaction_id, 0x60, 0x14, buffer, 2);
    while (!i2c_transaction_done(transaction_id)) i2c_service();
    i2c_clear_transaction(transaction_id);
    
    serial_0_put_string("Sea Level: ");
    for (int8_t i = 7; i >= 0; i--) {
        serial_0_put_string_P((buffer[0] & (1<<i)) ? gps_str_valid_1 : gps_str_valid_0);
    }
    serial_0_put_string_P(altest_space);
    for (int8_t i = 7; i >= 0; i--) {
        serial_0_put_string_P((buffer[1] & (1<<i)) ? gps_str_valid_1 : gps_str_valid_0);
    }
    serial_0_put_string_P(string_nl);
    
    
    // Write PT_DATA_CFG to enable data events on new pressure/altitude value
    buffer[0] = 0b00000010;
    i2c_write(&transaction_id, 0x60, 0x13, buffer, 1);
    while (!i2c_transaction_done(transaction_id)) i2c_service();
    i2c_clear_transaction(transaction_id);

    serial_0_put_string("Wrote PT_DATA_CFG\n");

    // Write CTRL_REG_1 to start collecting data
    buffer[0] = 0b00111010;
    i2c_write(&transaction_id, 0x60, 0x26, buffer, 1);
    while (!i2c_transaction_done(transaction_id)) i2c_service();
    i2c_clear_transaction(transaction_id);

    serial_0_put_string("Wrote CTRL_REG_1\n");

    // Wait for data to be ready
    do {
        i2c_read(&transaction_id, 0x60, 0x0, buffer, 1);
        while (!i2c_transaction_done(transaction_id)) i2c_service();
        i2c_clear_transaction(transaction_id);
    } while (!(buffer[0] & 0x04));

    serial_0_put_string("Data Ready\n");

    // Read data
    i2c_read(&transaction_id, 0x60, 0x01, buffer, 5);
    while (!i2c_transaction_done(transaction_id)) i2c_service();
    i2c_clear_transaction(transaction_id);
    
    for (int8_t i = 7; i >= 0; i--) {
        serial_0_put_string_P((buffer[0] & (1<<i)) ? gps_str_valid_1 : gps_str_valid_0);
    }
    serial_0_put_string_P(altest_space);
    for (int8_t i = 7; i >= 0; i--) {
        serial_0_put_string_P((buffer[1] & (1<<i)) ? gps_str_valid_1 : gps_str_valid_0);
    }
    serial_0_put_string_P(altest_space);
    for (int8_t i = 7; i >= 0; i--) {
        serial_0_put_string_P((buffer[2] & (1<<i)) ? gps_str_valid_1 : gps_str_valid_0);
    }
    serial_0_put_string_P(string_nl);

    double alt = (double)1024 * buffer[0];
    alt += (double)4 * buffer[1];
    alt += 0.25 * (double)((buffer[2] >> 4) & 0b1111);

    dtostrf(alt, 10, 4, str);
    serial_0_put_string(str);
    serial_0_put_string_P(string_nl);
    
    // write back pressure
    uint8_t high = (buffer[0] << 1) | !!(buffer[1] & (1<<7));
    uint8_t low = (buffer[1] << 1) | !!(buffer[2] & (1<<7));
    
    buffer[1] = low;
    buffer[0] = high;
    i2c_write(&transaction_id, 0x60, 0x14, buffer, 2);
    while (!i2c_transaction_done(transaction_id)) i2c_service();
    i2c_clear_transaction(transaction_id);
}

// checkid
static const char menu_cmd_checkid_string[] PROGMEM = "checkid";
static const char menu_help_checkid[] PROGMEM = "Check the device IDs of attached peripherals\n";

#ifdef ENABLE_SPI
static const char checkid_eeprom_1_title[] PROGMEM = "EEPROM 1 (25LC1024)\n";
static const char checkid_eeprom_2_title[] PROGMEM = "EEPROM 2 (25LC1024)\n";
static const char checkid_sig[] PROGMEM = "\tElectronic Signature: 0x";
#else
static const char checkid_no_spi[] PROGMEM = "SPI queue disabled. Cannot check EEPROMs.\n";
#endif

#ifdef ENABLE_I2C
static const char checkid_devid[] PROGMEM = "\tDevice ID: 0x";
static const char checkid_who_am_i[] PROGMEM = "\tWho Am I: 0x";
#else
static const char checkid_no_i2c[] PROGMEM = "I2C queue disabled. Cannot check sensors.\n";
#endif

#if defined(ENABLE_SPI) || defined(ENABLE_I2C)
static const char checkid_good[] PROGMEM = " (Good)\n";
static const char checkid_bad[] PROGMEM = " (Bad)\n";
#endif

void menu_cmd_checkid_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_checkid);
        return;
    }
    
#ifdef ENABLE_I2C
    uint8_t alt_id, accel_id, gyro_id;
    uint8_t alt_buffer, accel_buffer, gyro_buffer;
    
    i2c_read(&alt_id, 0x60, 0x0c, &alt_buffer, 1); // 0xC4
    i2c_read(&accel_id, 0x53, 0x0, &accel_buffer, 1); // 0xE5
    i2c_read(&gyro_id, 0x20, 0x0c, &gyro_buffer, 1); // 0xD7
    
    // Altimeter
    serial_0_put_string_P(sensors_str_baro_title);
    serial_0_put_string_P(checkid_who_am_i);
    while (!i2c_transaction_done(alt_id)) i2c_service();
    i2c_clear_transaction(alt_id);
    utoa(alt_buffer, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P((alt_buffer == 0xc4) ? checkid_good : checkid_bad);
    
    // Accelerometer
    serial_0_put_string_P(sensors_str_accel_title);
    serial_0_put_string_P(checkid_devid);
    while (!i2c_transaction_done(accel_id)) i2c_service();
    i2c_clear_transaction(accel_id);
    utoa(accel_buffer, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P((accel_buffer == 0xe5) ? checkid_good : checkid_bad);
    
    while (!serial_0_out_buffer_empty());

    // Gyroscope
    serial_0_put_string_P(sensors_str_gyro_title);
    serial_0_put_string_P(checkid_who_am_i);
    while (!i2c_transaction_done(gyro_id)) i2c_service();
    i2c_clear_transaction(gyro_id);
    utoa(gyro_buffer, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P((gyro_buffer == 0xd7) ? checkid_good : checkid_bad);
#else
    serial_0_put_string_P(checkid_no_i2c);
#endif

#ifdef ENABLE_SPI
    uint8_t eeprom_1_id, eeprom_2_id;
    uint8_t eeprom_1_buffer, eeprom_2_buffer;
    
    uint8_t eeprom_deep_power_down = 0b10111001;
    uint8_t eeprom_read_id[] = {0b10101011, 0, 0, 0};
    
    spi_start_half_duplex(&eeprom_1_id, EEPROM_CS_NUM, eeprom_read_id, 4, &eeprom_1_buffer, 1); // 0x29
    spi_start_half_duplex(&eeprom_2_id, EEPROM2_CS_NUM, eeprom_read_id, 4, &eeprom_2_buffer, 1); // 0x29
    
    // Eeprom 1
    serial_0_put_string_P(checkid_eeprom_1_title);
    serial_0_put_string_P(checkid_sig);
    while (!spi_transaction_done(eeprom_1_id)) spi_service();
    spi_clear_transaction(eeprom_1_id);
    utoa(eeprom_1_buffer, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P((eeprom_1_buffer == 0x29) ? checkid_good : checkid_bad);
    
    // Eeprom 2
    serial_0_put_string_P(checkid_eeprom_2_title);
    serial_0_put_string_P(checkid_sig);
    while (!spi_transaction_done(eeprom_2_id)) spi_service();
    spi_clear_transaction(eeprom_2_id);
    utoa(eeprom_2_buffer, str, 16);
    serial_0_put_string(str);
    serial_0_put_string_P((eeprom_2_buffer == 0x29) ? checkid_good : checkid_bad);
    
    // Put eeproms back in deep power down
    spi_start_half_duplex(&eeprom_1_id, EEPROM_CS_NUM, &eeprom_deep_power_down, 1, NULL, 0);
    spi_start_half_duplex(&eeprom_2_id, EEPROM2_CS_NUM, &eeprom_deep_power_down, 1, NULL, 0);
    
    while (!spi_transaction_done(eeprom_1_id)) spi_service();
    spi_clear_transaction(eeprom_1_id);
    while (!spi_transaction_done(eeprom_2_id)) spi_service();
    spi_clear_transaction(eeprom_2_id);
#else
    serial_0_put_string_P(checkid_no_spi);
#endif
}

// introm
static const char menu_cmd_introm_string[] PROGMEM = "introm";
static const char menu_help_introm[] PROGMEM = "Read from or write to internal eeprom\n\tRead: introm read <address>\n\tWrite: introm write <address> <data>\n\tSyncronous Read (Single Byte): introm readsync <address>\n";

static const char introm_string_sync[] PROGMEM = "readsync";

void menu_cmd_introm_handler(uint8_t arg_len, char** args)
{
    if (arg_len < 3) goto invalid_args;
    
    uint8_t id;
    char* end;
    uint32_t addr = strtoul(args[2], &end, 0);
    if (*end != '\0') goto invalid_args;
    
    if (!strcasecmp_P(args[1], eeprom_string_read)) {
        uint32_t buffer;
        eeprom_read(&id, addr, (uint8_t*)&buffer, 4);
        while (!eeprom_transaction_done(id));
        serial_0_put_string_P(eeprom_string_hex);
        ultoa(buffer, str, 16);
        serial_0_put_string(str);
        serial_0_put_string_P(string_nl);
    } else if (!strcasecmp_P(args[1], introm_string_sync)) {
        serial_0_put_string_P(eeprom_string_hex);
        ultoa(eeprom_read_byte_sync(addr), str, 16);
        serial_0_put_string(str);
        serial_0_put_string_P(string_nl);
    } else if (!strcasecmp_P(args[1], eeprom_string_write)) {
        if (arg_len < 4) goto invalid_args;
        
        uint32_t data = strtoul(args[3], &end, 0);
        if (*end != '\0') goto invalid_args;
        
        eeprom_write(&id, addr, (uint8_t*)&data, 4);
        while (!eeprom_transaction_done(id));
    } else {
        goto invalid_args;
    }
    eeprom_clear_transaction(id);
    
    return;
invalid_args:
    serial_0_put_string_P(menu_help_introm);
}

// XBeeSend
static const char menu_cmd_xbeesend_string[] PROGMEM = "xbeesend";
static const char menu_help_xbeesend[] PROGMEM = "Send a string via the radio.\nValid Usage: xbeesend <string>\nNote: String must not contain any spaces.\n";

void menu_cmd_xbeesend_handler(uint8_t arg_len, char** args)
{
    if (arg_len > 2) {
        serial_0_put_string_P(menu_help_xbeesend);
        return;
    } else if (arg_len < 2) {
        return;
    }
    
    uint8_t t_id;
    xbee_transmit_command(&t_id, 0, XBEE_ADDRESS_64_BROADCAST, XBEE_ADDRESS_16_UNKOWN, 0, 0, (uint8_t*)args[1], strlen(args[1]));
    
    while (!xbee_transaction_done(t_id)) xbee_service();
    
    xbee_clear_transaction(t_id);
}

// XBeeCont
static const char menu_cmd_xbeecont_string[] PROGMEM = "xbeecont";
static const char menu_help_xbeecont[] PROGMEM = "Send a continuous test signal on the radio\n";

void menu_cmd_xbeecont_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_xbeecont);
        return;
    }
    
    str[0] = '-';
    for (uint32_t i = 0; !serial_0_has_line('\n') ;i++) {
        uint8_t t_id;
        
        ultoa(i, str + 1, 10);
        
        xbee_transmit_command(&t_id, 0, XBEE_ADDRESS_64_BROADCAST, XBEE_ADDRESS_16_UNKOWN, 0, 0, (uint8_t*)str, strlen(str));
        
        while (!xbee_transaction_done(t_id)) xbee_service();
        
        xbee_clear_transaction(t_id);
        
        uint32_t time = millis;
        while ((millis - time) < 1000);
        wdt_reset();
    }
    serial_0_get_line('\n', str, STR_LEN);
}

// XBeetelem
static const char menu_cmd_xbeetelem_string[] PROGMEM = "xbeetelem";
static const char menu_help_xbeetelem[] PROGMEM = "Send a telemetry frame.\n";

static const char string_xbeetelem_length[] PROGMEM = " bytes sent.\n";

void menu_cmd_xbeetelem_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 1) {
        serial_0_put_string_P(menu_help_xbeetelem);
        return;
    }
    
    struct telemetry_api_frame frame;

    // Generate telemetry frame
    frame.start_delimiter = FRAME_START_DELIMITER;
    frame.source_address = ADDRESS_ROCKET;
    frame.destination_address = ADDRESS_GROUND_STATION;
    frame.payload_type = FRAME_TYPE_ROCKET_PRIMARY;
    frame.length = sizeof(frame.payload);
    frame.crc_present = 0;
    frame.end_delimiter = FRAME_END_DELIMITER;
    
    frame.payload.mission_time = millis;
    frame.payload.state = fsm_state;
    frame.payload.flag_ematch_1_present = ematch_1_is_ready();
    frame.payload.flag_ematch_2_present = ematch_2_is_ready();
    frame.payload.flag_parachute_deployed = 0;
    frame.payload.adc_cap_voltage = adc_avg_data[0];
    frame.payload.flag_gps_data_valid = (fgpmmopa6h_data_valid & 1);
    frame.payload.adc_temp_1 = adc_avg_data[1];
    frame.payload.adc_temp_2 = adc_avg_data[2];
    frame.payload.adc_3 = adc_avg_data[3];
    frame.payload.adc_4 = adc_avg_data[4];
    frame.payload.adc_5 = adc_avg_data[5];
    frame.payload.adc_6 = adc_avg_data[6];
    frame.payload.adc_batt_voltage = adc_avg_data[7];
    frame.payload.acceleration_x = adxl343_accel_x;
    frame.payload.acceleration_y = adxl343_accel_y;
    frame.payload.acceleration_z = adxl343_accel_z;
    frame.payload.pitch_rate = fxas21002c_pitch_rate;
    frame.payload.roll_rate = fxas21002c_roll_rate;
    frame.payload.yaw_rate = fxas21002c_yaw_rate;
    frame.payload.gyro_temp = fxas21002c_temp;
    frame.payload.altitude_lsb = mpl3115a2_alt_lsb;
    frame.payload.altitude_csb = mpl3115a2_alt_csb;
    frame.payload.altitude_msb = mpl3115a2_alt_msb;
    frame.payload.alt_temp_lsb = mpl3115a2_temp_lsb;
    frame.payload.alt_temp_msb = mpl3115a2_temp_msb;
    frame.payload.gps_time = fgpmmopa6h_utc_time;
    frame.payload.latitude = fgpmmopa6h_latitude;
    frame.payload.longitude = fgpmmopa6h_longitude;
    frame.payload.ground_speed = fgpmmopa6h_speed;
    frame.payload.course_over_ground = fgpmmopa6h_course;
    frame.payload.gps_sample_time = fgpmmopa6h_sample_time;
    
    // Send frame
    uint8_t t_id;
    xbee_transmit_command(&t_id, 0, XBEE_ADDRESS_64_BROADCAST, XBEE_ADDRESS_16_UNKOWN, 0, 0, (uint8_t*)&frame, sizeof(frame));
    
    utoa(sizeof(frame), str, 10);
    serial_0_put_string(str);
    serial_0_put_string_P(string_xbeetelem_length);
    
    while (!xbee_transaction_done(t_id)) xbee_service();
    xbee_clear_transaction(t_id);
}

// 12v
static const char menu_cmd_12v_string[] PROGMEM = "12v";
static const char menu_help_12v[] PROGMEM = "Control the 12v rail\nValid Usage: 12v <on/off>\n";

void menu_cmd_12v_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 2) {
        serial_0_put_string_P(menu_help_12v);
        return;
    }
    
    if (!strcasecmp_P(args[1], string_on)) {
        ENABLE_12V_PORT |= (1<<ENABLE_12V_NUM);
    } else if (!strcasecmp_P(args[1], string_off)) {
        ENABLE_12V_PORT &= ~(1<<ENABLE_12V_NUM);
    } else {
        serial_0_put_string_P(menu_help_12v);
    }
}

// deploy
static const char menu_cmd_deploy_string[] PROGMEM = "deploy";
static const char menu_help_deploy[] PROGMEM = "Deploy the parachutes\nValid Usage: deploy <on/off/sequence>";

static const char deploy_string_sequence[] PROGMEM = "sequence";

void menu_cmd_deploy_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 2) {
        serial_0_put_string_P(menu_help_deploy);
        return;
    }
    
    if (!strcasecmp_P(args[1], string_on)) {
        MAIN_TRIGGER_PORT |= (1<<MAIN_TRIGGER_NUM);
    } else if (!strcasecmp_P(args[1], string_off)) {
        MAIN_TRIGGER_PORT &= ~(1<<MAIN_TRIGGER_NUM);
    } else if (!strcasecmp_P(args[1], deploy_string_sequence)) {
        ENABLE_12V_PORT &= ~(1<<ENABLE_12V_NUM);
        MAIN_TRIGGER_PORT |= (1<<MAIN_TRIGGER_NUM);
        while (adc_avg_data[0] > 50);
        MAIN_TRIGGER_PORT &= ~(1<<MAIN_TRIGGER_NUM);
    } else {
        serial_0_put_string_P(menu_help_deploy);
    }
    
    
}

// capdis
static const char menu_cmd_capdis_string[] PROGMEM = "capdis";
static const char menu_help_capdis[] PROGMEM = "Control the capacitor discarge pin\nValid Usage: capdis <on/off>\n";

void menu_cmd_capdis_handler(uint8_t arg_len, char** args)
{
    if (arg_len != 2) {
        serial_0_put_string_P(menu_help_capdis);
        return;
    }
    
    if (!strcasecmp_P(args[1], string_on)) {
        CAP_DISCHARGE_PORT |= (1<<CAP_DISCHARGE_NUM);
    } else if (!strcasecmp_P(args[1], string_off)) {
        CAP_DISCHARGE_PORT &= ~(1<<CAP_DISCHARGE_NUM);
    } else {
        serial_0_put_string_P(menu_help_capdis);
    }
}

const uint8_t menu_num_items = 24;
const menu_item_t menu_items[] PROGMEM = {
    {.string = menu_cmd_version_string, .handler = menu_cmd_version_handler, .help_string = menu_help_version},
    {.string = menu_cmd_help_string, .handler = menu_cmd_help_handler, .help_string = menu_help_help},
    {.string = menu_cmd_clear_string, .handler = menu_cmd_clear_handler, .help_string = menu_help_clear},
    {.string = menu_cmd_reset_string, .handler = menu_cmd_reset_handler, .help_string = menu_help_reset},
    {.string = menu_cmd_stat_string, .handler = menu_cmd_stat_handler, .help_string = menu_help_stat},
    {.string = menu_cmd_eeprom_string, .handler = menu_cmd_epprom_handler, .help_string = menu_help_eeprom},
    {.string = menu_cmd_spitest_string, .handler = menu_cmd_spitest_handler, .help_string = menu_help_spitest},
    {.string = menu_cmd_spiraw_string, .handler = menu_cmd_spiraw_handler, .help_string = menu_help_spiraw},
    {.string = menu_cmd_spiconc_string, .handler = menu_cmd_spiconc_handler, .help_string = menu_help_spiconc},
    {.string = menu_cmd_analog_string, .handler = menu_cmd_analog_handler, .help_string = menu_help_analog},
    {.string = menu_cmd_sensors_string, .handler = menu_cmd_sensors_handler, .help_string = menu_help_sensors},
    {.string = menu_cmd_gps_string, .handler = menu_cmd_gps_handler, .help_string = menu_help_gps},
    {.string = menu_cmd_gps_ser_string, .handler = menu_cmd_gps_ser_handler, .help_string = menu_help_gps_ser},
    {.string = menu_cmd_actest_string, .handler = menu_cmd_actest_handler, .help_string = menu_help_actest},
    {.string = menu_cmd_altest_string, .handler = menu_cmd_altest_handler, .help_string = menu_help_altest},
    {.string = menu_cmd_iicraw_string, .handler = menu_cmd_iicraw_handler, .help_string = menu_help_iicraw},
    {.string = menu_cmd_iicio_string, .handler = menu_cmd_iicio_handler, .help_string = menu_help_iicio},
    {.string = menu_cmd_introm_string, .handler = menu_cmd_introm_handler, .help_string = menu_help_introm},
    {.string = menu_cmd_checkid_string, .handler = menu_cmd_checkid_handler, .help_string = menu_help_checkid},
    {.string = menu_cmd_xbeesend_string, .handler = menu_cmd_xbeesend_handler, .help_string = menu_help_xbeesend},
    {.string = menu_cmd_xbeecont_string, .handler = menu_cmd_xbeecont_handler, .help_string = menu_help_xbeecont},
    {.string = menu_cmd_xbeetelem_string, .handler = menu_cmd_xbeetelem_handler, .help_string = menu_help_xbeetelem},
    {.string = menu_cmd_12v_string, .handler = menu_cmd_12v_handler, .help_string = menu_help_12v},
    {.string = menu_cmd_deploy_string, .handler = menu_cmd_deploy_handler, .help_string = menu_help_deploy},
    {.string = menu_cmd_capdis_string, .handler = menu_cmd_capdis_handler, .help_string = menu_help_capdis}
};
