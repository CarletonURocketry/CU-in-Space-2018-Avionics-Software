/*
 * DeviceClasses.h
 * 
 * Defines action functions and interrupt routines for communicating with each device
 *
 * Created: 2017/10/25
 * Author : Zhongtian Xia
 */ 

#pragma once

#define RETRY_TIMES 200 //retrying times

typedef enum twistatus {OK=0xA1, BUS_ERROR=0xA2, TIMEOUT=0xA3, MODULE_FAIL=0XA4} I2CSTATUS;

class I2CDevice 
{
public:
	I2CSTATUS write(const uint8_t byte);
	I2CSTATUS read(uint8_t* start, uint8_t bytes_to_be_received);
	I2CDevice(uint8_t addr) : address(addr) {}
	static void set_prescaler(uint8_t prescaler);
	
private:
	uint8_t address;
};