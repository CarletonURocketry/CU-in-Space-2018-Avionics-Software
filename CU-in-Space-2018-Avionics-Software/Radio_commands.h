//
//  Radio_commands.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Maxim Zakirov
//
//  Handle the transmition and reception of data in proper API format
//

#ifndef Radio_commands_h
#define Radio_commands_h

#define DELIMITER_COMMAND 0x7E
//MSB is first. SPI needs to be mode 0.
#define API_PARAMETER 1


//Transaction Stuff
#define QUEUE_LENGTH 4

#define ID_INVALID   0  // The transaction ID for an unused transaction
#define ID_FIRST     1  // The first valid transaction ID

//API frame types. Do not mix up with the frame ids.

//command frames sent to module
#define AT_COMMAND 0x08
#define QUEUE_PARAMETER 0x09
#define TRANSMIT_REQUEST 0x10
#define EXPLICIT_COMMAND_FRAME 0x11
#define REMOTE_COMMAND_REQEUEST 0x17
#define CREATE_SOURCE_ROUTE 0x21

//command frames responses
#define AT_COMMAND_RESPONSE 0x88
#define MODEM_STATUS 0x8A
#define ZIGBEE_TRANSMIT_STATUS 0x8B
#define ZIGBEE_RECEIVE_PACKET 0x90 //Also known as the Rx indicator in other datasheets
#define ZIGBEE_EXPLICIT_RX_INDICATOR 0x91 //can't be used for this. AO = 1 needed.
#define ZIGBEE_IO_DATA_SAMPLE_RX_INDICATOR 0x92
#define XBEE_SENSOR_READ_INDICATOR 0x94 //can't be used because there is no 1 wire digi adapter.
#define NODE_IDENTIFICATION_INDICATOR 0x95
#define REMOTE_COMMAND_RESPONSE 0x97
#define OVER_AIR_FIRMWARE_UPDATE_STATUS 0xA0
#define ROUTE_RECORD_INDICATOR 0xA1
#define MANY_TO_ONE_ROUTE_REQUEST_INDICATOR 0xA3 //pretty sure this is the one we need


//AT commands
#define AT_FRAME_ID_RESPONSE 0x52 //R tells the xbee to send a response command

//Baud rates
#define PARAMETER_1200_BAUD 0x00 //if parameter is set to zero, it will make the baud rate 1200.

#endif /* Radio_commands_h */
