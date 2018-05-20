//
//  Barometer-MPL3115A2.c
//  CU-in-Space-2018-Avionics-Software
//
//  Barometric Pressure Sensor
//

#include "Barometer-MPL3115A2.h"
#include "Barometer-MPL3115A2-Registers.h"

#include <avr/io.h>

#include "pindefinitions.h"
#include "I2C.h"

#define SAMPLE_PERIOD (1000 / MPL3115A2_SAMPLE_RATE)
#define WARMUP_TIME 1000

// MARK: States
#define STATE_REQ_SAMPLE_PERIOD     7
#define STATE_REQ_INTERUPT          6
#define STATE_REQ_I2C_DONE          5

#define S_IDLE              0
#define S_WARMUP            1
#define S_READ_ALL          ((1<<STATE_REQ_I2C_DONE) | 3)
#define S_WRITE_CTRL1       ((1<<STATE_REQ_I2C_DONE) | 4)
#define S_WRITE_CTRL3       ((1<<STATE_REQ_I2C_DONE) | 5)
#define S_WRITE_CTRL4       ((1<<STATE_REQ_I2C_DONE) | 6)
#define S_WRITE_CTRL5       ((1<<STATE_REQ_I2C_DONE) | 7)
#define S_WRITE_DATA_CFG    ((1<<STATE_REQ_I2C_DONE) | 8)
#define S_START_PRESSURE    ((1<<STATE_REQ_I2C_DONE) | 9)
#define S_CHECK_PRESS_READY ((1<<STATE_REQ_I2C_DONE) | 10)
#define S_PRESS_MEASURMENT  ((1<<STATE_REQ_I2C_DONE) | 11)
#define S_WRITE_BARO        ((1<<STATE_REQ_I2C_DONE) | 12)
#define S_START_ALTITUDE    ((1<<STATE_REQ_INTERUPT) | 13)
#define S_READ_ALTITUDE     ((1<<STATE_REQ_I2C_DONE) | 14)
#define S_WAIT              ((1<<STATE_REQ_SAMPLE_PERIOD) | (1<<STATE_REQ_INTERUPT) | 15)

// MARK: Variables
uint32_t mpl3115a2_sample_time;
int32_t mpl3115a2_alt;
int32_t mpl3115a2_prev_alt;
uint8_t mpl3115a2_alt_msb;
uint8_t mpl3115a2_alt_csb;
uint8_t mpl3115a2_alt_lsb;
uint8_t mpl3115a2_temp_msb;
uint8_t mpl3115a2_temp_lsb;

/** The current state of the sensor FSM */
static uint8_t state = S_IDLE;
/** The buffer used to read and write from the sensor */
static uint8_t buffer[6];
/** The i2c transaction ID of the transaction used by the sensor */
static uint8_t i2c_id;

// MARK: Functions
void init_mpl3115a2(void)
{
    if (state != S_IDLE) {
        return;
    }
    
    state = S_WARMUP;
}

void mpl3115a2_service(void)
{
    // Return if the current state is not finished
    if ((state == S_IDLE) ||
        ((state == S_WARMUP) && (millis < WARMUP_TIME)) ||
        ((state & (1<<STATE_REQ_SAMPLE_PERIOD)) && ((millis - mpl3115a2_sample_time) < SAMPLE_PERIOD)) ||
        ((state & (1<<STATE_REQ_INTERUPT)) && (ALT_INT_PIN & (1<<ALT_INT_NUM)))) {
        return;
    } else if ((state & (1<<STATE_REQ_I2C_DONE))) {
        if (!i2c_transaction_done(i2c_id)) return;  // Do not continue if the I2C transaction is not yet finished
        
        uint8_t success = i2c_transaction_successful(i2c_id);
        i2c_clear_transaction(i2c_id);
        
        if ((state != S_READ_ALTITUDE) && !success) {
            // If we are in the init process and a transaction fails we jump back to the idle state.
            state = S_IDLE;
            return;
        } else if (!success) {
            // If a transaction fails after initilization we try to read again by going back to S_WAIT.
            state = S_WAIT;
            return;
        }
    }
    
    // Start the next state
    switch (state) {
        case S_WARMUP:
            // Read all registers to clear interupts
            i2c_read(&i2c_id, ADDRESS, STATUS, buffer, 6);
            state = S_READ_ALL;
            break;
        case S_READ_ALL:
            // Write oversample settings to control register 1 (and confirm that part is in standby)
            buffer[0] = (1<<CTRL_REG1_OS0) | (1<<CTRL_REG1_OS1) | (1<<CTRL_REG1_OS2);
            i2c_write(&i2c_id, ADDRESS, CTRL_REG1, buffer, 1);
            state = S_WRITE_CTRL1;
            break;
        case S_WRITE_CTRL1:
            // Set interupt 1 as active high in control register 3
            buffer[0] = (1<<CTRL_REG3_IPOL1);
            i2c_write(&i2c_id, ADDRESS, CTRL_REG3, buffer, 1);
            state = S_WRITE_CTRL3;
            break;
        case S_WRITE_CTRL3:
            // Enable interupts in control register 4
            buffer[0] = (1<<CTRL_REG4_EN_DRDY);
            i2c_write(&i2c_id, ADDRESS, CTRL_REG4, buffer, 1);
            state = S_WRITE_CTRL4;
            break;
        case S_WRITE_CTRL4:
            // Route interupts in control register 5
            buffer[0] = (1<<CTRL_REG5_EN_DRDY);
            i2c_write(&i2c_id, ADDRESS, CTRL_REG5, buffer, 1);
            state = S_WRITE_CTRL5;
            break;
        case S_WRITE_CTRL5:
            // Enable events on new pressure data in data event config register
            buffer[0] = (1<<PT_DATA_CFG_PDEFE);
            i2c_write(&i2c_id, ADDRESS, PT_DATA_CFG, buffer, 1);
            state = S_WRITE_DATA_CFG;
            break;
        case S_WRITE_DATA_CFG:
            // Write control register 1 to start a one shot pressure measurment
            buffer[0] = (1<<CTRL_REG1_OS0) | (1<<CTRL_REG1_OS0) | (1<<CTRL_REG1_OS1) | (1<<CTRL_REG1_OS2) | (1<<CTRL_REG1_OST);
            i2c_write(&i2c_id, ADDRESS, CTRL_REG1, buffer, 1);
            state = S_START_PRESSURE;
            break;
        case S_START_PRESSURE:
            // Read status register
            i2c_read(&i2c_id, ADDRESS, DR_STATUS, buffer, 1);
            state = S_CHECK_PRESS_READY;
            break;
        case S_CHECK_PRESS_READY:
            if (buffer[0] & (1<<DR_STATUS_PDR)) {
                // Start reading result of pressure measurment
                i2c_read(&i2c_id, ADDRESS, OUT_P_MSB, buffer, 3);
                state = S_PRESS_MEASURMENT;
            } else {
                // Read status register again
                i2c_read(&i2c_id, ADDRESS, DR_STATUS, buffer, 1);
            }
            break;
        case S_PRESS_MEASURMENT:
            // Shift pressure measurment and write it back to the sensor
            buffer[3] = (buffer[0] << 1) | !!(buffer[1] & (1<<7));
            buffer[4] = (buffer[1] << 1) | !!(buffer[2] & (1<<7));
            i2c_write(&i2c_id, ADDRESS, BAR_IN_MSB, buffer + 3, 2);
            state = S_WRITE_BARO;
            break;
        case S_WRITE_BARO:
            // Write control register 1 to start continuous altitude measurments
            buffer[0] = (1<<CTRL_REG1_ALT) | (1<<CTRL_REG1_OS0) | (1<<CTRL_REG1_OS1) | (1<<CTRL_REG1_OS2) |
                (1<<CTRL_REG1_SBYB);
            i2c_write(&i2c_id, ADDRESS, CTRL_REG1, buffer, 1);
            state = S_START_ALTITUDE;
            break;
        case S_START_ALTITUDE:
            // Start reading first altitude and temperature measurments
        case S_WAIT:
            // Start reading altitude and temperature measurments
            i2c_read(&i2c_id, ADDRESS, OUT_P_MSB, buffer, 5);
            state = S_READ_ALTITUDE;
            break;
        case S_READ_ALTITUDE:
            // Process altitude measurment
            mpl3115a2_alt_msb = buffer[0];
            mpl3115a2_alt_csb = buffer[1];
            mpl3115a2_alt_lsb = buffer[2];
            mpl3115a2_temp_msb = buffer[3];
            mpl3115a2_temp_lsb = buffer[4];
            // Store previous sample
            mpl3115a2_prev_alt = mpl3115a2_alt;
            // Put the whole part of the new data in the highest bytes of the altitude value
            ((uint8_t*)&mpl3115a2_alt)[3] = mpl3115a2_alt_msb;
            ((uint8_t*)&mpl3115a2_alt)[2] = mpl3115a2_alt_csb;
            // Shift the whole part into place (sign extending)
            mpl3115a2_alt >>= 12;
            // OR the fractional part into the altitude value
            mpl3115a2_alt |= (mpl3115a2_alt_lsb >> 4);
            // Update sample time
            mpl3115a2_sample_time = millis;
            state = S_WAIT;
            break;
        default:
            // We should never reach here
            state = S_IDLE;
            break;
    }
}

uint8_t mpl3115a2_init_done(void)
{
    return (state == S_WAIT) || (state == S_READ_ALTITUDE);
}

 uint8_t mpl3115a2_init_succesful(void)
{
    return state != S_IDLE;
}
