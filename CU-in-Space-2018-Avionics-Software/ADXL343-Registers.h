//
//  ADXL343-Registers.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-11-03.
//
//  Register definitions for ADXL343 accelerometer
//

#ifndef ADXL343_Registers_h
#define ADXL343_Registers_h

#define DEVID               0x00    // Device ID (fixed value of 0xE5)

#define THRESH_TAP          0x1D    // Tap threshold (62.5mg/LSB)

#define OFSX                0x1E    // X-axis offset (15.6 mg/LSB)
#define OFSY                0x1F    // Y-axis offset (15.6 mg/LSB)
#define OFSZ                0x20    // Z-axis offset (15.6 mg/LSB)

#define DUR                 0x21    // Tap duration (625 microseconds/LSB)
#define LATENT              0x22    // Tap latency (1.25 ms/LSB)
#define WINDOW              0x23    // Tap window (1.25 ms/LSB)

#define THRESH_ACT          0x24    // Activity threshold (62.5 mg/LSB)
#define THRESH_INACT        0x25    // Inactivity threshold (62.5 mg/LSB)
#define TIME_INACT          0x26    // Inactivity time (1 sec/LSB)

#define ACT_INACT_CTL       0x27    // Activity inactivity control
#define A_I_CTL_ACT_AC_DC   7       // Selects AC (1) or DC (0) coupled operation for activity detection
#define A_I_CTL_ACT_X_EN    6       // Enables use of x axis data in detection of activity
#define A_I_CTL_ACT_Y_EN    5       // Enables use of y axis data in detection of activity
#define A_I_CTL_ACT_Z_EN    4       // Enables use of z axis data in detection of activity
#define A_I_CTL_INACT_AC_DC 7       // Selects AC (1) or DC (0) coupled operation for inactivity detection
#define A_I_CTL_INACT_X_EN  6       // Enables use of x axis data in detection of inactivity
#define A_I_CTL_INACT_Y_EN  5       // Enables use of y axis data in detection of inactivity
#define A_I_CTL_INACT_Z_EN  4       // Enables use of z axis data in detection of inactivity

#define THRESH_FF           0x28    // The threshold for drop detection (62.5 mg/LSB)
#define TIME_FF             0x29    // Time that the value for all axes must be bellow threshold for drop detection (5 ms/LSB)

#define TAP_AXES            0x2a    // Tap axes selection register
#define TAP_AXES_SUPPRESS   3       // If set no douple tap will be registered if acceleration is greater than THRESH_TAP
#define TAP_AXES_TAP_X      2       // Enables participation of x axis in tap detection
#define TAP_AXES_TAP_Y      1       // Enables participation of y axis in tap detection
#define TAP_AXES_TAP_Z      0       // Enables participation of z axis in tap detection

#define ACT_TAP_STATUS      0x2b    // List which axes where involved in last detected tap or activity
#define A_T_STATUS_ACT_X    6       // Set if the x axis was the first invovled in the latest activity event
#define A_T_STATUS_ACT_Y    5       // Set if the y axis was the first invovled in the latest activity event
#define A_T_STATUS_ACT_Z    4       // Set if the z axis was the first invovled in the latest activity event
#define A_T_STATUS_ASLEEP   3       // Set if the part is asleep (only toggle in auto sleep mode)
#define A_T_STATUS_TAP_X    2       // Set if the x axis was the first invovled in the latest tap event
#define A_T_STATUS_TAP_Y    1       // Set if the y axis was the first invovled in the latest tap event
#define A_T_STATUS_TA_Z     0       // Set if the z axis was the first invovled in the latest tap event

#define BW_RATE             0x2c    // Selects rate
#define BW_RATE_LOW_POWER   4       // If set low power operation is selected
#define BW_RATE_DATA_MASK   0x0f    // These bits select the device sample rate

#define POWER_CTL           0x2d    // Power settings register
#define PWR_CTL_LINK        5       // Serially links activity and inactivity detection if set
#define PWR_CTL_AUTO_SLEEP  4       // Automatically places device in sleep mode when inactive if set
#define PWR_CTL_MEASURE     3       // Devices is in measure mode if set, standby otherwise
#define PWR_CTL_SLEEP       2       // Puts the device in a reduced power sleep mode
#define PWR_CTL_WAKEUP0     1       // Controls the frequency of measurements during sleep mode
#define PWR_CTL_WAKEUP1     0       // Controls the frequency of measurements during sleep mode

#define INT_ENABLE          0x2e    // Interupt enable register
#define INT_EN_DATA_READY   7       // Enables the data ready interupt if set
#define INT_EN_SINGLE_TAP   6       // Enables the single tap interupt if set
#define INT_EN_DOUBLE_TAP   5       // Enables the double tap interupt if set
#define INT_EN_ACTIVITY     4       // Enables the activity interupt if set
#define INT_EN_INACTIVITY   3       // Enables the inactivity interupt if set
#define INT_EN_FREE_FALL    2       // Enables the free fall interupt if set
#define INT_EN_WATERMARK    1       // Enables the FIFO watermark interupt if set
#define INT_EN_OVERRUN      0       // Enables the FIFO overrun interupt if set

#define INT_MAP             0x2f    // Interupt mapping register
#define INT_MAP_DATA_READY  7       // The data ready interupt is sent to the INT2 pin set, otherwise the INT1 pin
#define INT_MAP_SINGLE_TAP  6       // The single tap interupt is sent to the INT2 pin set, otherwise the INT1 pin
#define INT_MAP_DOUBLE_TAP  5       // The double tap interupt is sent to the INT2 pin set, otherwise the INT1 pin
#define INT_MAP_ACTIVITY    4       // The activity interupt is sent to the INT2 pin set, otherwise the INT1 pin
#define INT_MAP_INACTIVITY  3       // The inactivity interupt is sent to the INT2 pin set, otherwise the INT1 pin
#define INT_MAP_FREE_FALL   2       // The free fall interupt is sent to the INT2 pin set, otherwise the INT1 pin
#define INT_MAP_WATERMARK   1       // The FIFO watermark interupt is sent to the INT2 pin set, otherwise the INT1 pin
#define INT_MAP_OVERRUN     0       // The FIFO overrun interupt is sent to the INT2 pin set, otherwise the INT1 pin

#define INT_SOURCE          0x30    // Interupt source register
#define INT_SRC_DATA_READY  7       // Set if the data ready interupt has triggered an event
#define INT_SRC_SINGLE_TAP  6       // Set if the single tap interupt has triggered an event
#define INT_SRC_DOUBLE_TAP  5       // Set if the double tap interupt has triggered an event
#define INT_SRC_ACTIVITY    4       // Set if the activity interupt has triggered an event
#define INT_SRC_INACTIVITY  3       // Set if the inactivity interupt has triggered an event
#define INT_SRC_FREE_FALL   2       // Set if the free fall interupt has triggered an event
#define INT_SRC_WATERMARK   1       // Set if the FIFO watermark interupt has triggered an event
#define INT_SRC_OVERRUN     0       // Set if the FIFO overrun interupt has triggered an event

#define DATA_FORMAT         0x31    // Data format settings register
#define D_F_SELF_TEST       7       // Enables application of self test acceleration
#define D_F_SPI             6       // If set device is in 3 wire SPI mode, otherwise 4 wire SPI mode is used
#define D_F_INT_INVERT      5       // If set interupts are active low, othewise they are active high
#define D_F_FULL_RES        3       // If set a constant scale factor of 4 mg/LSB is used, otherwise constant 10 bit resolution
#define D_F_JUSTIFY         2       // If set left justified, otherwise right justified with sign extention
#define D_F_RANGE1          1       // Select the range
#define D_F_RANGE0          0       // Select the range

#define DATAX0              0x32    // Least signifigant byte of x axis acceleration
#define DATAX1              0x33    // Most signifigant byte of x axis acceleration
#define DATAY0              0x34    // Least signifigant byte of y axis acceleration
#define DATAY1              0x35    // Most signifigant byte of y axis acceleration
#define DATAZ0              0x36    // Least signifigant byte of z axis acceleration
#define DATAZ1              0x37    // Most signifigant byte of z axis acceleration

#define FIFO_CTL            0x38    // FIFO buffer settings register
#define FIFO_CTL_MODE0      7       // Sets FIFO buffer mode
#define FIFO_CTL_MODE1      6       // Sets FIFO buffer mode
#define FIFO_CTL_TRIGGER    5       // Selects interupt source for trigger mode: INT2 if set, INT1 otherwise
#define FIFO_CTL_SAMPLE_MSK 0x1f    // Number of trigger entries, usage is dependent on selected mode

#define FIFO_STATUS         0x39    // Status information for FIFO buffer
#define FIFO_TRIG           7       // Set if FIFO trigger event has occured
#define FIFO_ENTRIES_MASK   0x1f    // Number of entries in FIFO buffer

#endif /* ADXL343_Registers_h */
