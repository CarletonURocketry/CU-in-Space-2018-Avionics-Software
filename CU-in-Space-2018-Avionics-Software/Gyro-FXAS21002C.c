//
//  Gyro-FXAS21002C.c
//  CU-in-Space-2018-Avionics-Software
//
//  Gyroscope
//

#include "Gyro-FXAS21002C.h"
#include "Gyro-FXAS21002C-Registers.h"

int16_t fxas21002c_pitch_rate;
int16_t fxas21002c_roll_rate;
int16_t fxas21002c_yaw_rate;
int8_t fxas21002c_temp;
