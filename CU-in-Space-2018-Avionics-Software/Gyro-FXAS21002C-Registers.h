//
//  Gyro-FXAS21002C-Registers.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-11-03.
//
//  Register definitions for FXA21002C gyroscope
//

#ifndef Gyro-FXAS21002C_Registers_h
#define Gyro-FXAS21002C_Registers_h

#define ADDRESS             0x20    // Device address with SAO pin grounded

#define STATUS              0x00    // Allias to one of DR_STATUS or F_STATUS

#define OUT_X_MSB           0x01    // Most signifigant byte of x axis angular velocity
#define OUT_X_LSB           0x02    // Least signifigant byte of x axis angular velocity
#define OUT_Y_MSB           0x03    // Most signifigant byte of y axis angular velocity
#define OUT_Y_LSB           0x04    // Least signifigant byte of y axis angular velocity
#define OUT_Z_MSB           0x05    // Most signifigant byte of z axis angular velocity
#define OUT_Z_LSB           0x06    // Least signifigant byte of z axis angular velocity

#define DR_STATUS           0x07    // Status register
#define DR_STATUS_ZYXOW     7       // Set if the data in the Z,Y or X register has been overwritten before being read
#define DR_STATUS_ZOW       6       // Set if the data in the Z register has been overwritten before being read
#define DR_STATUS_YOW       5       // Set if the data in the Y register has been overwritten before being read
#define DR_STATUS_XOW       4       // Set if the data in the X register has been overwritten before being read
#define DR_STATUS_ZYXDR     3       // Set if new data is ready on any of the Z, X or Y channels
#define DR_STATUS_ZDR       2       // Set if new data is ready on the Z channel
#define DR_STATUS_YDR       1       // Set if new data is ready on the Y channel
#define DR_STATUS_XDR       0       // Set if new data is ready on the X channel

#define F_STATUS            0x08    // FIFO buffer status register
#define F_STATUS_F_OVF      7       // Set if the FIFO buffer has overflowed
#define F_STATUS_F_WMKF     6       // Set if the number of samples in the FIFO buffer has reached the watermark
#define F_STATUS_F_CNT_MASK 0x3f    // The number of samples in the FIFO buffer

#define F_SETUP             0x09    // FIFO buffer setup register
#define F_SETUP_F_MODE1     7       // Set the FIFO buffer mode
#define F_SETUP_F_MODE0     6       // Set the FIFO buffer mode
#define F_SETUP_F_WMRK_MASK 0x3f    // Sets the watermark for the FIFO buffer

#define F_EVENT             0x0a    // FIFO event register
#define F_EVENT_EVENT       5       // Set if a FIFO event has been detected
#define F_EVENT_TIME_MASK   0x1f    // The number of ODR periods since F_EVENT_EVENT was set

#define INT_SOURCE_FLAG     0x0b    // Source of interupt events
#define INT_SOURCE_BOOTEND  3       // Boot sequence complete
#define INT_SOURCE_FIFO     2       // FIFO event
#define INT_SOURCE_RT       1       // Rate threshold event
#define INT_SOURCE_DRDY     0       // Data-ready event

#define WHO_AM_I            0x0c    // Device identifier (factory programmed to 0xD7)

#define CTRL_REG0           0x0d    // General control and configuration register
#define CTRL_REG0_BW1       7       // Bandwidth of digital output
#define CTRL_REG0_BW0       6       // Bandwidth of digital output
#define CTRL_REG0_SPIW      5       // SPI Mode: 3 wire mode if set, 4 wire mode othewise
#define CTRL_REG0_SEL1      4       // High pass filter cuttoff frequency
#define CTRL_REG0_SEL0      3       // High pass filter cuttoff frequency
#define CTRL_REG0_HPF_EN    2       // High pass filter is enabled if set
#define CTRL_REG0_FS1       1       // Select full scale range
#define CTRL_REG0_FS0       0       // Select full scale range

#define RT_CFG              0x0e    // Enables rate threshold interupt generation
#define RT_CFG_ELE          3       // If set event flag latch is enabled
#define RT_CFG_ZTEFE        2       // Enables event detection in the z axis
#define RT_CFG_YTEFE        1       // Enables event detection in the y axis
#define RT_CFG_XTEFE        0       // Enables event detection in the x axis

#define RT_SRC              0x0f    // Indicates the source of a rate threshold event
#define RT_SRC_EA           6       // Set if one or more events have occured
#define RT_SRC_ZRT          5       // Set if a rate threshold event has been detected on the z axis
#define RT_SRC_Z_RT_POL     4       // Set if the z axis rate threshold event was in a negative direction
#define RT_SRC_YRT          3       // Set if a rate threshold event has been detected on the y axis
#define RT_SRC_Y_RT_POL     2       // Set if the y axis rate threshold event was in a negative direction
#define RT_SRC_XRT          1       // Set if a rate threshold event has been detected on the x axis
#define RT_SRC_X_RT_POL     0       // Set if the x axis rate threshold event was in a negative direction

#define RT_THS              0x10    // Sets the rate threshold
#define RT_THS_CBCNTM       7       // Select mode for debounce counter
#define RT_THS_MASK         0x7f    // The rate threshold

#define RT_COUNT            0x11    // The number of rate threshold debounce counts (number of counts before interupt asserted)

#define TEMP                0x12    // 8 bit two's complement temperature value (1 degC/LSB)

#define CTRL_REG1           0x13    // Device configuration register 1
#define CTRL_REG1_RST       6       // Setting this bit triggers as software reset
#define CTRL_REG1_ST        5       // Enables self test when set
#define CTRL_REG1_DR2       4       // Sets data rate
#define CTRL_REG1_DR1       3       // Sets data rate
#define CTRL_REG1_DR0       2       // Sets data rate
#define CTRL_REG1_ACTIVE    1       // Selects standby or active mode
#define CTRL_REG1_READY     0       // Selects standby or ready mode

#define CTRL_REG2           0x14    // Selects output pins and polarities for interupt sources
#define CTRL_REG2_CFG_FIFO  7       // If set FIFO interupt is routed to the INT1 pin, othewise the INT2 pin
#define CTRL_REG2_EN_FIFO   6       // If set the rate  interupt is enabled
#define CTRL_REG2_CFG_RT    5       // If set rate threshold interupt is routed to the INT1 pin, othewise the INT2 pin
#define CTRL_REG2_EN_RT     4       // If set the rate threshold interupt is enabled
#define CTRL_REG2_CFG_DRDY  3       // If set data ready interupt is routed to the INT1 pin, othewise the INT2 pin
#define CTRL_REG2_EN_DRDY   2       // If set the data ready interupt is enabled
#define CTRL_REG2_IPOL      1       // Sets interupt logic polarity: if set active high, otherwise active low
#define CTRL_REG2_PP_OD     0       // Sets interupt driver configuration, if set push-pull driver

#define CTRL_REG3           0x15    // Device configuration register 3
#define CTRL_REG3_WRAPTOONE 3       // If set auto-increment pointer rolls over to 0x01 (OUT_X_MSB), otherwise 0x00 (STATUS)
#define CTRL_REG3_EXTCTRLEN 2       // If set INT2 pin acts as an external power mode control input
#define CTRL_REG3_FS_DOUBLE 0       // 
#endif /* Gyro-FXAS21002C_Registers_h */
