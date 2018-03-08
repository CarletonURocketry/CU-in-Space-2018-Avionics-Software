//
//  Accel-ADXL343.c
//  CU-in-Space-2018-Avionics-Software
//
//  Accelerometer
//

#include "Accel-ADXL343.h"
#include "Accel-ADXL343-Registers.h"
#include "I2C.h"

#define POLL_INTERVAL (1000/(ADXL343_SAMPLE_RATE)) // interval between each poll in milliseconds
#define FIFO_SETTING &0x00 // disable FIFO
#define INT_SETTING &0x00 // not using any interrupt
#define DATA_FORMAT_SETTING &0x0b // full resolution, range -16g~16g
#define BW_RATE_SETTING &0x0d // output data rate = 800 Hz
#define POWER_CTL_SETTING &(1 << PWR_CTL_MEASURE)

#define MAX_TRANSACTION_ID_NUM 5
#define  EARTH_GRAVITATIONAL_ACCEL_METRIC 9.81 // [m/s^2]
#define  EARTH_GRAVITATIONAL_ACCEL_IMPERIAL 32.2 // [ft/s^2]
#define ACCEL_SCALE_FACTOR 0.004 // 4mg per LSB in full res mode

static uint8_t accel_data_buffer[6];
static uint8_t accel_transaction_id[MAX_TRANSACTION_ID_NUM];
typedef enum {ACCEL_INIT, ACCEL_WAIT, ACCEL_READ} sensor_state;
static sensor_state state;

//TODO: implement self testing, if fail, return 1
uint8_t init_adxl343(void) {
	i2c_write(&accel_transaction_id[0], ADDRESS, FIFO_CTL, FIFO_SETTING, 1); 
	i2c_write(&accel_transaction_id[1], ADDRESS, INT_ENABLE, INT_SETTING, 1);
	i2c_write(&accel_transaction_id[2], ADDRESS, DATA_FORMAT, DATA_FORMAT_SETTING, 1);
	i2c_write(&accel_transaction_id[3], ADDRESS, BW_RATE, BW_RATE_SETTING, 1);
	i2c_write(&accel_transaction_id[4], ADDRESS, POWER_CTL, POWER_CTL_SETTING, 1);
	state = ACCEL_INIT;
	return 0;
}

void adxl343_service(void) {
	switch(state) {
		case ACCEL_INIT:
			uint8_t all_clear_flag = 1;
			for (uint8_t i = 0; i < MAX_TRANSACTION_ID_NUM; ++i)
			{
				if (i2c_transaction_done(accel_transaction_id[i]) && accel_transaction_id[i])
				{
					i2c_clear_transaction(accel_transaction_id[i]);
					accel_transaction_id[i] = 0;
				}
				else {
					all_clear_flag = 0;
				}
			}
			if (all_clear_flag)
			{
				state = ACCEL_WAIT;
				adxl343_sample_time = millis;
			}
			break;
		case ACCEL_WAIT:
			if (millis - adxl343_sample_time >= POLL_INTERVAL)
			{
				i2c_read(&accel_transaction_id[0], ADDRESS, DATAX0, &accel_data_buffer[0], 6); // multibyte reading starting from DATAX0
				state = ACCEL_READ;
			}
			break;
		case ACCEL_READ:
			if (i2c_transaction_done(accel_transaction_id[0]))
			{
				adxl343_sample_time = millis;
				i2c_clear_transaction(accel_transaction_id[0]);
				accel_transaction_id[0] = 0;
				adxl343_accel_x = ((accel_data_buffer[1] << 8) | accel_data_buffer[0]);
				adxl343_accel_y = ((accel_data_buffer[3] << 8) | accel_data_buffer[2]);
				adxl343_accel_z = ((accel_data_buffer[5] << 8) | accel_data_buffer[4]);
				state = ACCEL_WAIT;
			}
	}
}