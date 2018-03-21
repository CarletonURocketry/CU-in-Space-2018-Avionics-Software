//
//  GPS-FGPMMOPA6H.c
//  CU-in-Space-2018-Avionics-Software
//
//  GPS
//

#include "GPS-FGPMMOPA6H.h"

//sanity checks currently in place
//ensuring headers
//ensuring number fields are actually numbers
#define SANITYCHECK

//two logging functions should they be needed to be implemented at some point
#define WORRY(x)
#define PANIC(x)

uint32_t fgpmmopa6h_sample_time;
uint32_t fgpmmopa6h_utc_time;
int32_t fgpmmopa6h_lattitude;
int32_t fgpmmopa6h_longitude;
int16_t fgpmmopa6h_speed;
int16_t fgpmmopa6h_course;

//parses number from general form "hhmmss.sss" where there are breaks in the number by a factor of 6 every 2 digits before decimal place
//first_break indicates how many digits are at the beginning of the representation before the first seperation by a factor of 6.
//first_break = 2 for time and latitude, =3 for longitude
int parse_gps_number(char* stuff, int first_break){
    int remainder_of_break = first_break%2;
    int value = 0;
    uint8_t before_decimal = 1;
    for(int i=0; stuff[i] != ','; i++){
        if(stuff[i] == '.'){
            before_decimal = 0;
            continue;
        }
        if(i%2 == remainder_of_break && before_decimal){
            value *= 6;
        }else{
            value *= 10;
        }
#ifdef SANITYCHECK
        if(stuff[i] < '0' || stuff[i] > '9'){
            PANIC("number field contains non number");
            return -1;
        }
#endif
        value += stuff[i] - '0';
    }
    return value;
}


//RMC—Recommended Minimum Navigation Information
//data format from FGPMMOPA6H.pdf page 22
void parse_RMC(char*data){
    //note that all the notes about indice comments - the upper bound is the indice of the comma seperating fields
#ifdef SANITYCHECK
    //index 0-6 header is always $GPRMC
    if(strncmp("$GPRMC",data,6) != 0){
        PANIC("parse_RMC got wrong data");
        return;
    }
#endif
    //index 7-17 UTC time as hhmmss.sss
    fgpmmopa6h_utc_time = parse_gps_number(data+7, 2);
    
    //index 18-19 status - A=data valid or V=data not valid
    if(data[18] != 'A'){
        if(data[18] == 'V'){
            WORRY("parse_RMC has V status (invalid packet)");
        }else{
            PANIC("parse_RMC has unrecognized status flag");
        }
    }
    
    //index 20-29 latitude  ddmm.mmmm (degrees and minutes)
    fgpmmopa6h_lattitude = parse_gps_number(data+20, 2);
    
    //index 30-31 North South indicator (either N or S)
    //probably not useful to track
    
    //index 32-42 Longitude  dddmm.mmmm (degrees and minutes)
    fgpmmopa6h_longitude = parse_gps_number(data+32, 3);
    
    //index 43-44 East West indicator (either E or W)
    //probably not useful to track
    
    //index 45-49 speed over ground  k.kk (knots)
    fgpmmopa6h_speed = parse_gps_number(data+45, 1);
    
    //index 50-56 course over ground  ddd.dd (degrees from true north)
    fgpmmopa6h_course = parse_gps_number(data+50, 3);
    
    //index 57-63 date as DDMMYY
    //probably not useful to track
    
    //index 64-68 magnetic variation d.dd (degrees)
    
    
    //index 69-70 magnetic variation direction (E or W)
    
    
    //index 71 mode?
    
}
