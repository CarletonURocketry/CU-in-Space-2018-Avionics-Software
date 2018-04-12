//
//  Accel-ADXL343.c
//  CU-in-Space-2018-Avionics-Software
//
//  Accelerometer
//

#include "Accel-ADXL343.h"
#include "Accel-ADXL343-Registers.h"
#include "I2C.h"

#define POLL_INTERVAL (1000/(ADXL343_SAMPLE_RATE)) // Calculate interval between each poll in milliseconds based on defined sample rate
#define ROUND_DIVIDE(a,b) ( ((a)+(b)/2) / (b) ) // idea: http://www.nongnu.org/avr-libc/user-manual/FAQ.html#faq_wrong_baud_rate
// Unlike simple divide operation that will always truncate the decimal part of the result, ROUND_DIVIDE rounds the result of a division to the nearest integer.

static uint8_t fifo_setting[] = {0x0};          // disable FIFO
static uint8_t int_setting[] = {0x0};           // not using any interrupt
static uint8_t data_format_setting[] = {0xb};   // full resolution, range -16g~16g
static uint8_t bw_rate_setting[] = {0xd};       // output data rate = 800 Hz
static uint8_t power_ctl_setting[] = {(1<<PWR_CTL_MEASURE)};

#define MAX_TRANSACTION_ID_NUM 5
#define ACCEL_DATA_BUFFER_SIZE 6
#define ACCEL_SCALE_FACTOR 0.0039 // 3.9mg per LSB in full res mode

static uint8_t accel_data_buffer[ACCEL_DATA_BUFFER_SIZE];
static uint8_t accel_transaction_id[MAX_TRANSACTION_ID_NUM];
typedef enum {ACCEL_INIT, ACCEL_WAIT, ACCEL_READ, ACCEL_CALIB_WAIT, ACCEL_CALIB_START, ACCEL_CALIB_READ} sensor_state;
static sensor_state state;

uint32_t adxl343_sample_time;
int16_t adxl343_accel_x;
int16_t adxl343_accel_y;
int16_t adxl343_accel_z;

//TODO: implement self testing, if fail, return 1
//
// init_adxl343 would queue up I2C transactions to send initialization commands to the accelerometer. If any of them failed to queue, abort 
// the initialization process and return 1
uint8_t init_adxl343(void)
{
	if(i2c_write(&accel_transaction_id[0], ADDRESS, FIFO_CTL, fifo_setting, 1)) return 1;
	if(i2c_write(&accel_transaction_id[1], ADDRESS, INT_ENABLE, int_setting, 1)) return 1;
	if(i2c_write(&accel_transaction_id[2], ADDRESS, DATA_FORMAT, data_format_setting, 1)) return 1;
	if(i2c_write(&accel_transaction_id[3], ADDRESS, BW_RATE, bw_rate_setting, 1)) return 1;
	if(i2c_write(&accel_transaction_id[4], ADDRESS, POWER_CTL, power_ctl_setting, 1)) return 1;
	state = ACCEL_INIT;
	return 0;
}

void adxl343_service(void)
{
	switch(state) {
		case ACCEL_INIT:
            ; // Labels must be followed by statements
	    	// Needs to send the configuration to the accelerometer to initialize it
			uint8_t all_clear_flag = 1;
			for (uint8_t i = 0; i < MAX_TRANSACTION_ID_NUM; ++i) {
				if (accel_transaction_id[i]) {
					if (i2c_transaction_done(accel_transaction_id[i])) {
						i2c_clear_transaction(accel_transaction_id[i]);
						accel_transaction_id[i] = 0;
					}
					else all_clear_flag = 0;
				}
			}
			if (all_clear_flag) {
				state = ACCEL_CALIB_START;
			}
			break;
		case ACCEL_WAIT:
			// Send the command to read in x,y,z data registers from the I2C bus
			if ((millis - adxl343_sample_time) >= POLL_INTERVAL) {
				if (!i2c_read(&accel_transaction_id[0], ADDRESS, DATAX0, &accel_data_buffer[0], 6)) 
					// Multibyte reading starting from DATAX0 to guarantee atomic reading of the data registers
					// If i2c_read queue allocation is successful (i.e. it returns 0), change the state
					state = ACCEL_READ;
			}
			break;
		case ACCEL_READ:
			// Waiting the transaction to be done, then copy the data register values into accel variables.
			if (i2c_transaction_done(accel_transaction_id[0])) {
				adxl343_sample_time = millis;
				i2c_clear_transaction(accel_transaction_id[0]);
				accel_transaction_id[0] = 0;
				adxl343_accel_x = ((accel_data_buffer[1] << 8) | accel_data_buffer[0]);
				adxl343_accel_y = ((accel_data_buffer[3] << 8) | accel_data_buffer[2]);
				adxl343_accel_z = ((accel_data_buffer[5] << 8) | accel_data_buffer[4]);
				state = ACCEL_WAIT;
			}
			break;
		case ACCEL_CALIB_READ:
			// Waiting the transaction to be done, then calculate the offset values based on p.28 of the ADXL343 datasheet.
			if (i2c_transaction_done(accel_transaction_id[0])) {
				i2c_clear_transaction(accel_transaction_id[0]);
				accel_transaction_id[0] = 0;
				int8_t offsets[3]; // {x,y,z}
				// Offsets are added to the data registers, not subtracted.
				offsets[0] = (int8_t) -ROUND_DIVIDE((int16_t)((accel_data_buffer[1] << 8) | accel_data_buffer[0]), 4);
				offsets[1] = (int8_t) -ROUND_DIVIDE((int16_t)((accel_data_buffer[3] << 8) | accel_data_buffer[2]), 4);
				offsets[2] = (int8_t) ROUND_DIVIDE(125 - (int16_t)( (accel_data_buffer[5] << 8) | accel_data_buffer[4]), 4); 
				// 125 is the result of 1000/4, @res=4mg/LSB
				if (!i2c_write(&accel_transaction_id[0], ADDRESS, OFSX, (uint8_t*)offsets, 3)) state = ACCEL_CALIB_WAIT; 
				// Multibyte writing of offset to prevent occupying too many transaction queues
				// If i2c_write queue allocation is successful (i.e. it returns 0), change the state

			}
			break;
		case ACCEL_CALIB_WAIT:
			if (i2c_transaction_done(accel_transaction_id[0])) {
				i2c_clear_transaction(accel_transaction_id[0]);
				accel_transaction_id[0] = 0;
				state = ACCEL_WAIT;
			}
			break;
		case ACCEL_CALIB_START:
			// Send the command to read in x,y,z data registers from the I2C bus for calibration
			if (!i2c_read(&accel_transaction_id[0], ADDRESS, DATAX0, &accel_data_buffer[0], 6)) state = ACCEL_CALIB_READ;
	}
}
