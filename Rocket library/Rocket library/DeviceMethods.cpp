/*
 * DeviceClasses.cpp
 * 
 * Defines action functions and interrupt routines for communicating with each device
 *
 * Created: 2017/10/25
 * Author : Zhongtian Xia
 */ 

#include <avr/io.h>
#include <util/twi.h>
#include <DeviceClasses.h>
#define F_CPU 1625000UL // 1.625 MHZ CPU clock frequency

//TODO: timeout auto retry feature
//TODO: implement RESTART condition

//NOTE: This chip does not support 10-bit I2C slave address

static void I2CDevice::set_prescaler(uint8_t prescaler) {
	switch (prescaler)
	{
	case : 1
		TWSR &= ~(_BV(TWPS0)|_BV(TWPS1)); // 00
		break;

	case : 4
		TWSR |= _BV(TWPS0); // 01
		TWSR &= ~_BV(TWPS1);
		break;

	case : 16
		TWSR &= ~_BV(TWPS0); // 10
		TWSR |= _BV(TWPS1);
		break;

	case : 64
		TWSR |= _BV(TWPS1)|_BV(TWPS0); // 11
		break;

	default: // Do nothing with all other inputs
	}
}


I2CSTATUS I2CDevice::write(const uint8_t byte) {
	uint8_t retry = 0;
	uint8_t TWIstatus;
RETRY:
	if (++retry > RETRY_TIMES) // if fail after 200 times, time out
	{
		TWCR |= _BV(TWSTA)|_BV(TWINT)|_BV(TWEN);
		return TIMEOUT;
	}

START:
	// generate START condition
	TWCR |= _BV(TWINT)|_BV(TWSTA)|_BV(TWEN);
	while(!(TWCR & _BV(TWINT))); //wait for TWINT flag set
	TWIstatus = TW_STATUS;
	if (TWIstatus == TW_BUS_ERROR) { 
		TWCR |= _BV(TWSTO)|_BV(TWINT)|_BV(TWEN);
		return BUS_ERROR;
	}
	if (TWIstatus != TW_START && TWIstatus != TW_REP_START) {
		return MODULE_FAIL;
	}

	// send SLA+W
	TWDR = (address<<1) | TW_WRITE; // write address and write bit to data register
	TWCR |= _BV(TWINT)|_BV(TWEN);
	while(!(TWCR & _BV(TWINT)));
	TWIstatus = TW_STATUS;
	if (TWIstatus == TW_MT_SLA_NACK ) {
		goto RETRY;
	}

	// send data
	TWDR = byte;
	TWCR |= _BV(TWINT)|_BV(TWEN);
	while(!(TWCR & _BV(TWINT)));
	TWIstatus = TW_STATUS;
	if (TWIstatus == TW_MT_DATA_NACK ) {
		goto RETRY;
	}

	// generate STOP condition
	TWCR |= _BV(TWSTO)|_BV(TWINT)|_BV(TWEN);
	return OK;
	
}

I2CSTATUS I2CDevice::read(uint8_t* start, uint8_t bytes_to_be_received) {
	uint8_t retry = 0;
	uint8_t TWIstatus;
	uint8_t* pointer = start;
RETRY:
	if (++retry > RETRY_TIMES) // if fail after 200 times, time out
	{
		TWCR |= _BV(TWSTA)|_BV(TWINT)|_BV(TWEN);
		return TIMEOUT;
	}

START:
	// generate START condition
	TWCR |= _BV(TWINT)|_BV(TWSTA)|_BV(TWEN);
	while(!(TWCR & _BV(TWINT))); //wait for TWINT flag set
	TWIstatus = TW_STATUS;
	if (TWIstatus == TW_BUS_ERROR) {
		TWCR |= _BV(TWSTO)|_BV(TWINT)|_BV(TWEN);
		return BUS_ERROR;
	}
	if (TWIstatus != TW_START && TWIstatus != TW_REP_START) {
		return MODULE_FAIL;
	}

	// send SLA+R
	TWDR = (address<<1) | TW_READ; // write address and write bit to data register
	TWCR |= _BV(TWINT)|_BV(TWEN);
	while(!(TWCR & _BV(TWINT)));
	TWIstatus = TW_STATUS;
	if (TWIstatus == TW_MR_SLA_NACK ) {
		goto RETRY;
	}

	//receive data
	while(bytes_to_be_received-- > 0) {
		if (bytes_to_be_received > 0) 
			TWCR |= _BV(TWEN)|_BV(TWEA)|_BV(TWINT);
		else // last transmission, return NOT ACK
			TWCR |= _BV(TWEN)|_BV(TWINT);
		TWIstatus = TW_STATUS;
		if (TWIstatus == TW_MR_DATA_ACK || TWIstatus == TW_MR_DATA_NACK) {
			*pointer = TWDR;
			pointer++;
		}
		else return MODULE_FAIL;
	}

	// generate STOP condition
	TWCR |= _BV(TWSTO)|_BV(TWINT)|_BV(TWEN);
	return OK;
}