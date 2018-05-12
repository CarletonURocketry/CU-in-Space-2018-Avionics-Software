//
//  GPS-FGPMMOPA6H.c
//  CU-in-Space-2018-Avionics-Software
//
//  GPS
//

#include "GPS-FGPMMOPA6H.h"
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "serial1.h"

// MARK: Constants
#define GPS_BUFFER_LENGTH 128

static const char gps_tid_RMC[] PROGMEM = "$GPRMC";
static const char gps_tid_GSV[] PROGMEM = "$GPGSV";

static const char gps_cmd_set_output[] PROGMEM = "$PMTK314,0,1,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n";
static const char gps_cmd_set_rate[] PROGMEM = "$PMTK220,2000*1C\r\n";

// MARK: Macros
//sanity checks currently in place
//ensuring headers
//ensuring number fields are actually numbers
#define SANITYCHECK

//two logging functions should they be needed to be implemented at some point
#define WORRY(x)
#define PANIC(x)

// MARK: GPS packet handler
typedef void (*gps_func_t)(uint8_t, char**);
typedef struct {
    const PGM_P string;
    const gps_func_t func;
} gps_handler_t;

// MARK: Variables
uint32_t fgpmmopa6h_sample_time;
uint32_t fgpmmopa6h_utc_time;
int32_t fgpmmopa6h_latitude;
int32_t fgpmmopa6h_longitude;
int16_t fgpmmopa6h_speed;
int16_t fgpmmopa6h_course;
uint8_t fgpmmopa6h_satellites_in_view;
uint8_t fgpmmopa6h_data_valid;

static char gps_buffer[GPS_BUFFER_LENGTH];

// MARK: Function declerations
static uint8_t verify_checksum(char *str);

static void func_rmc(uint8_t argc, char **argv);
static void func_gsv(uint8_t argc, char **argv);

static void parse_RMC(char*data);

// MARK: Packet handlers
static const uint8_t num_handlers = 2;
static const gps_handler_t gps_handlers[] PROGMEM = {
    {.string = gps_tid_RMC, .func = func_rmc},
    {.string = gps_tid_GSV, .func = func_gsv}
};

// MARK: Function definitions
uint8_t init_fgpmmopa6h(void)
{
    init_serial_1();
    
    serial_1_put_string_P(gps_cmd_set_rate);
    serial_1_put_string_P(gps_cmd_set_output);
    return 0;
}

void fgpmmopa6h_service(void)
{
    //serial_1_service();
    if (serial_1_has_line('\n')) {
        serial_1_get_line('\n', gps_buffer, GPS_BUFFER_LENGTH);
        
        if (!verify_checksum(gps_buffer)) {
            return; // Data is invalid
        }
        
        char *line = gps_buffer;
        
        int num_tokens = 1;
        for (uint8_t i = 0; i < strlen(line); i++) {
            num_tokens += gps_buffer[i] == ',';
        }
        
        char *args[num_tokens];
        for (uint8_t i = 0; (args[i] = strsep(&line, ",")) != NULL; i++);
        
        // Replace the astrix in the input with a null char so that the checksum is ignored
        for (uint8_t i = 0; args[num_tokens - 1][i] != '\0'; i++) {
            if (args[num_tokens - 1][i] == '*') {
                args[num_tokens - 1][i] = '\0';
                break;
            }
        }
        
        for (uint8_t i = 0; i < num_handlers; i++) {
            if (!strcasecmp_P(args[0], (char*)pgm_read_word(&gps_handlers[i].string))) {
                (*((gps_func_t)pgm_read_word(&gps_handlers[i].func)))(num_tokens, args);
                return;
            }
        }
        // Unkown command, ignored
    }
}


static uint8_t verify_checksum(char *str)
{
    uint8_t checksum = str[1];
    uint8_t i;
    
    for (i = 2; (str[i] != '*') && (str[i] != '\0'); i++) {
        checksum ^= str[i];
    }
    
    if (str[i] != '*') {
        // No cheksum present
        return 0;
    }
    
    char* end;
    uint8_t chk = strtoul(str + i + 1, &end, 16);
    return (*end == '\0') ? (checksum == chk) : 0;
}

// MARK: Packet Handlers

/**
 * Parse an RMC (Recommended Minimum Navigation Information) NMEA sentence and store the results in the appropriate variables
 * @param argc The number of fields in the sentence (should always be 13)
 * @param argv The fields of the sentence
 */
static void func_rmc(uint8_t argc, char **argv)
{
    if (argc != 13) {
        // Not the right type of packet, don't even try to parse it
        goto invalid_sentence;
    }
    
    char* end;
    
    
    fgpmmopa6h_data_valid <<= 1;
    
    /* 0: Message ID -> $GPRMC (no need to check, this function wouldn't have been called if this field wasn't correct) */
    
    /* 1: UTC Time -> hhmmss.sss */
    // Decimal part is already in milliseconds
    fgpmmopa6h_utc_time = strtoul(argv[1] + 7, &end, 10);
    if (*end != '\0') goto invalid_sentence;
    // Seconds part must be multiplied by 1000
    argv[1][6] = '\0';
    fgpmmopa6h_utc_time += (strtoul(argv[1] + 4, &end, 10) * 1000);
    if (*end != '\0') goto invalid_sentence;
    // Minutes part must be multiplied by 60000
    argv[1][4] = '\0';
    fgpmmopa6h_utc_time += (strtoul(argv[1] + 2, &end, 10) * 60000);
    if (*end != '\0') goto invalid_sentence;
    // Hours part must be multiplied by 3600000
    argv[1][2] = '\0';
    fgpmmopa6h_utc_time += (strtoul(argv[1], &end, 10) * 3600000);
    if (*end != '\0') goto invalid_sentence;
    
    /* 2: Status -> 'A' or 'V' */
    if (*(argv[2]) == 'V') {
        // Data is not invalid (no fix)
        return;
    }
    
    /* 3: Latitude -> ddmm.mmmm */
    // Decimal part is already in 100 milli-minutes
    fgpmmopa6h_latitude = strtoul(argv[3] + 5, &end, 10);
    if (*end != '\0') goto invalid_sentence;
    // Minutes part must be multiplied by 10000
    argv[3][4] = '\0';
    fgpmmopa6h_latitude += (strtoul(argv[3] + 2, &end, 10) * 10000);
    if (*end != '\0') goto invalid_sentence;
    // Degrees part must be multiplied by 600000
    argv[3][2] = '\0';
    fgpmmopa6h_latitude += (strtoul(argv[3], &end, 10) * 600000);
    if (*end != '\0') goto invalid_sentence;
    
    /* 4: N/S (Latitude) -> 'N' or 'S' */
    if (*(argv[4]) == 'S') {
        fgpmmopa6h_latitude *= -1;
    }
    
    /* 5: Longitude -> dddmm.mmmm */
    // Decimal part is already in 100 milli-minutes
    fgpmmopa6h_longitude = strtoul(argv[5] + 6, &end, 10);
    if (*end != '\0') goto invalid_sentence;
    // Minutes part must be multiplied by 10000
    argv[5][5] = '\0';
    fgpmmopa6h_longitude += (strtoul(argv[5] + 3, &end, 10) * 10000);
    if (*end != '\0') goto invalid_sentence;
    // Degrees part must be multiplied by 600000
    argv[5][3] = '\0';
    fgpmmopa6h_longitude += (strtoul(argv[5], &end, 10) * 600000);
    if (*end != '\0') goto invalid_sentence;
    
    /* 6: E/W (Longitude) -> 'E' or 'W' */
    if (*(argv[6]) == 'W') {
        fgpmmopa6h_longitude *= -1;
    }
    
    /* 7: Speed over ground -> Fixed point decimal with two decimal places */
    uint8_t decimal_index;
    for (decimal_index = 1; (argv[7][decimal_index] != '.') && (argv[7][decimal_index]) != '\0'; decimal_index++);
    if (argv[7][decimal_index] == '\0') {
        goto invalid_sentence;
    }
    
    // Part after decimal is already in centi-knots
    fgpmmopa6h_speed = strtoul(argv[7] + decimal_index + 1, &end, 10);
    if (*end != '\0') goto invalid_sentence;
    // Part before decimal must be multiplied by 100
    argv[7][decimal_index] = '\0';
    fgpmmopa6h_speed += (strtoul(argv[7], &end, 10) * 100);
    if (*end != '\0') goto invalid_sentence;
    
    /* 8: Course over ground -> Fixed point decimal with two decimal places */
    for (decimal_index = 1; (argv[8][decimal_index] != '.') && (argv[8][decimal_index]) != '\0'; decimal_index++);
    if (argv[8][decimal_index] == '\0') {
        goto invalid_sentence;
    }
    
    // Part after decimal is already in centi-knots
    fgpmmopa6h_course = strtoul(argv[8] + decimal_index + 1, &end, 10);
    if (*end != '\0') goto invalid_sentence;
    // Part before decimal must be multiplied by 100
    argv[8][decimal_index] = '\0';
    fgpmmopa6h_course += (strtoul(argv[8], &end, 10) * 100);
    if (*end != '\0') goto invalid_sentence;
    
    /* 9: Date -> ddmmyy */
    
    /* 10: Magnetic Variation -> Never populated (do not attempt to parse) */
    
    /* 11: E/W (Magnetic Variation) -> Never populated (do not attempt to parse) */
    
    /* 12: Mode -> 'A', 'D' or 'E' */
    
    fgpmmopa6h_data_valid |= 1;
    fgpmmopa6h_sample_time = millis;
    return;
    
invalid_sentence:
    return;
}

/**
 * Parse an GSV (GNSS Satellites in View) NMEA sentence and store the results in the appropriate variables
 * @param argc The number of fields in the sentence (should always be 20)
 * @param argv The fields of the sentence
 */
static void func_gsv(uint8_t argc, char **argv)
{
    if (argc <= 4) {
        // Not the right type of packet, don't even try to parse it
        goto invalid_sentence;
    }
    
    char* end;
    
    /* 0: Message ID -> $GPGSV (no need to check, this function wouldn't have been called if this field wasn't correct) */
    
    /* 1: Number of Messages -> int */
    
    /* 2: Message Number -> int */
    
    /* 3: Satellites in View -> int */
    fgpmmopa6h_satellites_in_view = strtoul(argv[3], &end, 10);
    if (*end != '\0') goto invalid_sentence;
    
    return;
    
invalid_sentence:
    return;
}




////parses number from general form "hhmmss.sss" where there are breaks in the number by a factor of 6 every 2 digits before decimal place
////first_break indicates how many digits are at the beginning of the representation before the first seperation by a factor of 6.
////first_break = 2 for time and latitude, =3 for longitude
//static int parse_gps_number(char* stuff, int first_break)
//{
//    int remainder_of_break = first_break%2;
//    int value = 0;
//    uint8_t before_decimal = 1;
//    for(int i=0; stuff[i] != ','; i++){
//        if(stuff[i] == '.'){
//            before_decimal = 0;
//            continue;
//        }
//        if(i%2 == remainder_of_break && before_decimal){
//            value *= 6;
//        }else{
//            value *= 10;
//        }
//#ifdef SANITYCHECK
//        if(stuff[i] < '0' || stuff[i] > '9'){
//            PANIC("number field contains non number");
//            return -1;
//        }
//#endif
//        value += stuff[i] - '0';
//    }
//    return value;
//}
//
//
////RMC—Recommended Minimum Navigation Information
////data format from FGPMMOPA6H.pdf page 22
//static void parse_RMC(char*data)
//{
//    //note that all the notes about indice comments - the upper bound is the indice of the comma seperating fields
//#ifdef SANITYCHECK
//    //index 0-6 header is always $GPRMC
//    if(strncmp("$GPRMC",data,6) != 0){
//        PANIC("parse_RMC got wrong data");
//        return;
//    }
//#endif
//    //index 7-17 UTC time as hhmmss.sss
//    fgpmmopa6h_utc_time = parse_gps_number(data+7, 2);
//
//    //index 18-19 status - A=data valid or V=data not valid
//    if(data[18] != 'A'){
//        if(data[18] == 'V'){
//            WORRY("parse_RMC has V status (invalid packet)");
//        }else{
//            PANIC("parse_RMC has unrecognized status flag");
//        }
//    }
//
//    //index 20-29 latitude  ddmm.mmmm (degrees and minutes)
//    fgpmmopa6h_latitude = parse_gps_number(data+20, 2);
//
//    //index 30-31 North South indicator (either N or S)
//    //probably not useful to track
//
//    //index 32-42 Longitude  dddmm.mmmm (degrees and minutes)
//    fgpmmopa6h_longitude = parse_gps_number(data+32, 3);
//
//    //index 43-44 East West indicator (either E or W)
//    //probably not useful to track
//
//    //index 45-49 speed over ground  k.kk (knots)
//    fgpmmopa6h_speed = parse_gps_number(data+45, 1);
//
//    //index 50-56 course over ground  ddd.dd (degrees from true north)
//    fgpmmopa6h_course = parse_gps_number(data+50, 3);
//
//    //index 57-63 date as DDMMYY
//    //probably not useful to track
//
//    //index 64-68 magnetic variation d.dd (degrees)
//
//
//    //index 69-70 magnetic variation direction (E or W)
//
//
//    //index 71 mode?
//}
