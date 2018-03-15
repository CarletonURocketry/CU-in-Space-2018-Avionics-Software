//
//  Barometer-MPL3115A2.c
//  CU-in-Space-2018-Avionics-Software
//
//  Barometric Pressure Sensor
//

#include "Barometer-MPL3115A2.h"
#include "Barometer-MPL3115A2-Registers.h"

uint32_t mpl3115a2_sample_time;
uint32_t mpl3115a2_alt;
uint8_t mpl3115a2_alt_msb;
uint8_t mpl3115a2_alt_csb;
uint8_t mpl3115a2_alt_lsb;
uint8_t mpl3115a2_temp_msb;
uint8_t mpl3115a2_temp_lsb;
