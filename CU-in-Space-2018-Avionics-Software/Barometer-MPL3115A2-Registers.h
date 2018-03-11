//
//  Barometer-MPL3115A2-Registers.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-11-03.
//
//  Register definitions for MPL3115A2 barometric altimiter
//

#ifndef Barometer_MPL3115A2_Registers_h
#define Barometer_MPL3115A2_Registers_h

#define ADDRESS             0x60    // Device address

#define STATUS              0x00    // Allias to one of DR_STATUS or F_STATUS

#define OUT_P_MSB           0x01    // Most signifigant byte of pressure/altitude data
#define OUT_P_CSB           0x02    // Center? signifigant byte of pressure/altitude data
#define OUT_P_LSB           0x03    // Least signifigant byte of pressure/altitude data

#define OUT_T_MSB           0x04    // Most signifigant byte of temperature value
#define OUT_T_LSB           0x04    // Least signifigant byte of temperature value

#define DR_STATUS           0x06    // Status register
#define DR_STATUS_PTOW      7       // 1 if the previous pressure/altitude or temp value was overwritten before it being read
#define DR_STATUS_POW       6       // 1 if the previous pressure/altitude value was overwritten before it could be read
#define DR_STATUS_TOW       5       // 1 if the previous temp value was overwritten before it could be read
#define DR_STATUS_PTDR      3       // 1 if a new set of presure/altitude or temperature data is ready to be read
#define DR_STATUS_PDR       2       // 1 if a new set of presure/altitude data is ready to be read
#define DR_STATUS_PTR       1       // 1 if a new temperature value is ready to be read

#define OUT_P_DELTA_MSB     0x07    // The MSB of the differnece between the most recent and previous pressure/altitude samples
#define OUT_P_DELTA_CSB     0x08    // The CSB of the differnece between the most recent and previous pressure/altitude samples
#define OUT_P_DELTA_LSB     0x09    // The LSB of the differnece between the most recent and previous pressure/altitude samples

#define OUT_T_DELTA_MSB     0x0a    // The MSB of the differnece between the most recent and previous temperature samples
#define OUT_T_DELTA_LSB     0x0b    // The LSB of the differnece between the most recent and previous temperature samples

#define WHO_AM_I            0x0c    // Device ID register (value is 0xC4)

#define F_STATUS            0x0d    // Status register for FIFO mode
#define F_STATUS_OVF        7       // 1 if a FIFO overflow event has been detected
#define F_STATUS_WMRK_FLAG  8       // 1 if a FIFO watermark value has been deteced
#define F_STATUS_CNT_MASK   0x3f    // Mask to read the count of sample in the FIFO buffer from F_STATUS

#define F_DATA              0x0e    // FIFO data register. Reads oldest byte in FIFI buffer

#define F_SETUP             0x0f    // FIFO setup register
#define F_SETUP_MODE0       7       // See datasheet
#define F_SETUP_MODE1       6       // See datasheet
#define F_SETUP_WMRK_MASK   0x3f    // The number of samples that the FIFO can store before triggering a watermark warning

#define TIME_DLY            0x10    // The number of ticks of data sample time since the last byte of the FIFO was written

#define SYSMOD              0x11    // System mode register
#define SYSMOD_SYSMOD       0       // System mode: 0 = standby, 1 = active

#define INT_SOURCE          0x12    // System interrupt status register: Values are 1 if corresponding interupt is asserted
#define INT_SRC_DRDY        7       // Data ready
#define INT_SRC_FIFO        6       // FIFO overlfow
#define INT_SRC_PW          5       // Altitude/Presure alert
#define INT_SRC_TW          4       // Temperature alert
#define INT_SRC_PTH         3       // Altitude/presure threshold
#define INT_SRC_TTH         2       // Temperature threshold
#define INT_SRC_PCHG        1       // Presure change
#define INT_SRC_TCHG        0       // Temperature change

#define PT_DATA_CFG         0x13    // Configures the pressure data, temperature data and event flag generator.
#define PT_DATA_CFG_DREM    2       // Generate data ready events if set
#define PT_DATA_CFG_PDEFE   1       // Generate data ready events for altitude/pressure if set
#define PT_DATA_CFG_TDEFE   0       // Generate data ready events temperature if set

#define BAR_IN_MSB          0x14    // Most signifigant byte of barometric presure used as sea level for altitude calcualtions
#define BAR_IN_LSB          0x15    // Least signifigant byte of barometric presure used as sea level for altitude calcualtions

#define P_TGT_MSB           0x16    // Most signifigant byte of target altitude/pressure
#define P_TGT_LSB           0x17    // Least signifigant byte of target altitude/pressure

#define T_TGT               0x18    // Target temperature

#define P_WND_LSB           0x19    // Most signifigant byte of altitude/pressure window
#define P_WND_MSB           0x1a    // Least signifigant byte of altitude/pressure window

#define T_WIN               0x1b    // Target temperature window

#define P_MIN_MSB           0x1c    // Most signifigant byte of the minimum pressure/altitude sample
#define P_MIN_CSB           0x1d    // Center? signifigant byte of the minimum pressure/altitude sample
#define P_MIN_LSB           0x1e    // Least signifigant byte of the minimum pressure/altitude sample

#define T_MIN_MSB           0x1f    // Most signifigant byte of the minimum temperature sample
#define T_MIN_LSB           0x20    // Least signifigant byte of the minimum temperature sample

#define P_MAX_MSB           0x21    // Most signifigant byte of the maximum pressure/altitude sample
#define P_MAX_CSB           0x22    // Center? signifigant byte of the maximum pressure/altitude sample
#define P_MAX_LSB           0x23    // Least signifigant byte of the maximum pressure/altitude sample

#define T_MAX_MSB           0x24    // Most signifigant byte of the maximum temperature sample
#define T_MAX_LSB           0x25    // Least signifigant byte of the maximum temperature sample

#define CTRL_REG1           0x26    // Control Register 1
#define CTRL_REG1_ALT       7       // If bit is set the sensor is in altimiter mode, othewise the sensor is in barometer mode
#define CTRL_REG1_RAW       6       // Sets raw mode
#define CTRL_REG1_OS2       5       // Oversample ratio
#define CTRL_REG1_OS1       4       // Oversample ratio
#define CTRL_REG1_OS0       3       // Oversample ratio
#define CTRL_REG1_RST       2       // Enables software reset
#define CTRL_REG1_OST       1       // Starts a measurement immediately
#define CTRL_REG1_SBYB      0       // Device is in active mode when bit set, standby otherwise

#define CTRL_REG2           0x27    // Control Register 2
#define CTRL_REG2_LO        5       // Loads the next measurment into the altitude/pressure target register when set
#define CTRL_REG2_ASEL      4       // Chooses if the values from the data registers or P_TGT_MSB/LSB are used for target
#define CTRL_REG2_ST_MASK   0x0f    // Auto acquisition time

#define CTRL_REG3           0x28    // Control Register 3
#define CTRL_REG3_IPOL1     5       // If set interupts are active high on pad 1, active low otherwise
#define CTRL_REG3_PP_OD1    4       // Sets if interupts on pad 1 are pulled up or open drain
#define CTRL_REG3_IPOL2     1       // If set interupts are active high on pad 2, active low otherwise
#define CTRL_REG3_PP_OD2    0       // Sets if interupts on pad 2 are pulled up or open drain

#define CTRL_REG4           0x29    // Interupt enable register
#define CTRL_REG4_EN_DRDY   7       // Data ready
#define CTRL_REG4_EN_FIFO   6       // FIFO overlfow
#define CTRL_REG4_EN_PW     5       // Altitude/Presure alert
#define CTRL_REG4_EN_TW     4       // Temperature alert
#define CTRL_REG4_EN_PTH    3       // Altitude/presure threshold
#define CTRL_REG4_EN_TTH    2       // Temperature threshold
#define CTRL_REG4_EN_PCHG   1       // Presure change
#define CTRL_REG4_EN_TCHG   0       // Temperature change

#define CTRL_REG5           0x2a    // Interupt configuration register: if bit is set routed to INT1 pin, otherwise INT1 pin
#define CTRL_REG5_EN_DRDY   7       // Data ready
#define CTRL_REG5_EN_FIFO   6       // FIFO overlfow
#define CTRL_REG5_EN_PW     5       // Altitude/Presure alert
#define CTRL_REG5_EN_TW     4       // Temperature alert
#define CTRL_REG5_EN_PTH    3       // Altitude/presure threshold
#define CTRL_REG5_EN_TTH    2       // Temperature threshold
#define CTRL_REG5_EN_PCHG   1       // Presure change
#define CTRL_REG5_EN_TCHG   0       // Temperature change

#define OFF_P               0x2b    // Offset correction for pressure
#define OFF_T               0x2c    // Offset correction for temperature
#define OFF_H               0x2d    // Offset correction for altitude

#endif /* Barometer_MPL3115A2_Registers_h */
