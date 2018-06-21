//
//  telemetry.c
//  CU-in-Space-2018-Avionics-Software
//
//

#include "telemetry.h"
#include "telemetry_format.h"

#include "ematch_detect.h"
#include "EEPROM.h"

#include "XBee.h"
#include "25LC1024.h"

#include "ADC.h"
#include "Barometer-MPL3115A2.h"
#include "Accel-ADXL343.h"
#include "Gyro-FXAS21002C.h"
#include "GPS-FGPMMOPA6H.h"

#define EEPROM_TELEMETRY_SPACING    64

#define RADIO_WARMUP_TIME   250

static uint32_t last_eeprom_time;
static uint32_t last_radio_time;

static uint8_t  has_sent_packet;

uint32_t eeprom_telemetry_period;
uint32_t radio_telemetry_period;

static uint16_t eeprom_frame_number;

static uint8_t eeprom_transaction_id;
static uint8_t internal_eeprom_transaction_id;
static uint8_t xbee_transaction_id;

static struct telemetry_api_frame frame;


static void update_telemetry_packet (void)
{
    frame.payload.mission_time = millis;
    
    /*** ADC Data and Flags ***/
    
    // ADC0
    frame.payload.state = fsm_state;
    frame.payload.flag_ematch_1_present = ematch_1_is_ready();
    frame.payload.flag_ematch_2_present = ematch_2_is_ready();
    frame.payload.flag_parachute_deployed = 0;
    frame.payload.adc_cap_voltage = adc_avg_data[0];
    // ADC1
    frame.payload.flag_gps_data_valid = (fgpmmopa6h_data_valid & 1);
    frame.payload.adc_temp_1 = adc_avg_data[1];
    // ADC2
    frame.payload.adc_temp_2 = adc_avg_data[2];
    // ADC3
    frame.payload.adc_3 = adc_avg_data[3];
    // ADC4
    frame.payload.adc_4 = adc_avg_data[4];
    // ADC5
    frame.payload.adc_5 = adc_avg_data[5];
    // ADC6
    frame.payload.adc_6 = adc_avg_data[6];
    // ADC7
    frame.payload.adc_batt_voltage = adc_avg_data[7];
    
    /*** Accelerometer ***/
    frame.payload.acceleration_x = adxl343_accel_x;
    frame.payload.acceleration_y = adxl343_accel_y;
    frame.payload.acceleration_z = adxl343_accel_z;
    
    /*** Gyroscope ***/
    frame.payload.pitch_rate = fxas21002c_pitch_rate;
    frame.payload.roll_rate = fxas21002c_roll_rate;
    frame.payload.yaw_rate = fxas21002c_yaw_rate;
    frame.payload.gyro_temp = fxas21002c_temp;
    
    /*** Altimiter ***/
    frame.payload.altitude_lsb = mpl3115a2_alt_lsb;
    frame.payload.altitude_csb = mpl3115a2_alt_csb;
    frame.payload.altitude_msb = mpl3115a2_alt_msb;
    frame.payload.alt_temp_lsb = mpl3115a2_temp_lsb;
    frame.payload.alt_temp_msb = mpl3115a2_temp_msb;
    
    /*** GPS ***/
    frame.payload.gps_time = fgpmmopa6h_utc_time;
    frame.payload.latitude = fgpmmopa6h_latitude;
    frame.payload.longitude = fgpmmopa6h_longitude;
    frame.payload.ground_speed = fgpmmopa6h_speed;
    frame.payload.course_over_ground = fgpmmopa6h_course;
    frame.payload.gps_sample_time = fgpmmopa6h_sample_time;
}

void init_telemetry (void) {
    frame.start_delimiter = FRAME_START_DELIMITER;
    
    frame.source_address = ADDRESS_ROCKET;
    frame.destination_address = ADDRESS_GROUND_STATION;
    
    frame.payload_type = FRAME_TYPE_ROCKET_PRIMARY;
    
    frame.length = sizeof(frame.payload);
    frame.crc_present = 0;
    
    frame.end_delimiter = FRAME_END_DELIMITER;
    
    // Read address of next eeprom telemetry frame
    eeprom_read(&internal_eeprom_transaction_id, EEPROM_ADDR_TELEMETRY_LOCATION, (uint8_t*)&eeprom_frame_number, sizeof(eeprom_frame_number));
    
    update_telemetry_packet();
}

void telemetry_send_packet (void)
{
    has_sent_packet = 0;
}

void telemetry_service(void)
{
    if ((eeprom_transaction_id != 0) && eeprom_25lc1024_transaction_done(eeprom_transaction_id)) {
        eeprom_25lc1024_clear_transaction(eeprom_transaction_id);
        eeprom_transaction_id = 0;
    }
    
    if ((internal_eeprom_transaction_id != 0) && eeprom_transaction_done(internal_eeprom_transaction_id)) {
        eeprom_clear_transaction(internal_eeprom_transaction_id);
        internal_eeprom_transaction_id = 0;
    }
    
    if ((xbee_transaction_id != 0) && xbee_transaction_done(xbee_transaction_id)) {
        xbee_clear_transaction(xbee_transaction_id);
        xbee_transaction_id = 0;
    }
    
    uint16_t eeprom_addr = EEPROM_TELEMETRY_SPACING * eeprom_frame_number;
    uint8_t save_packet = (eeprom_telemetry_period != 0) && ((millis - last_eeprom_time) > eeprom_telemetry_period) && (eeprom_transaction_id == 0) && (internal_eeprom_transaction_id == 0) && (eeprom_addr < EEPROM_25LC1024_MAX);
    uint8_t send_packet = ((radio_telemetry_period != 0) && ((millis - last_radio_time) > radio_telemetry_period) && (xbee_transaction_id == 0)) || (!has_sent_packet && (millis > RADIO_WARMUP_TIME));
    
    if (send_packet || save_packet) {
        // Need to generate a new telemetry packet
        update_telemetry_packet();
    } else {
        return;
    }
    
    if (save_packet) {
        // Save telemetry to EEPROM
        eeprom_25lc1024_write(&eeprom_transaction_id, eeprom_addr, sizeof(frame.payload), (uint8_t*)&frame.payload);
        last_eeprom_time = millis;
        // Save updated eeprom frame number to internal eeprom
        eeprom_frame_number++;
        eeprom_write(&internal_eeprom_transaction_id, EEPROM_ADDR_TELEMETRY_LOCATION, (uint8_t*)&eeprom_frame_number, sizeof(eeprom_frame_number));
    }
    
    if (send_packet) {
        // Send telemetry over radio
        xbee_transmit_command(&xbee_transaction_id, 0, XBEE_ADDRESS_64_BROADCAST, XBEE_ADDRESS_16_UNKOWN, 0, 0, (uint8_t*)&frame, sizeof(frame));
        last_radio_time = millis;
        has_sent_packet = 1;
    }
}
