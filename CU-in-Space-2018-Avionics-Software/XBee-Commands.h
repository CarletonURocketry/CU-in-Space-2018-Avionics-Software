//
//  XBee-Commands.h
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2018-05-20.
//

#ifndef XBee_Commands_h
#define XBee_Commands_h

// MARK: Frame Types
#define FRAME_AT_CMD                            0x08
#define FRAME_AT_CMD_QUEUE_PARAM                0x09
#define FRAME_TRANSMIT_REQUEST                  0x10
#define FRAME_EXPLICIT_ADDR_ZB_CMD              0x11
#define FRAME_REMOTE COMMAND REQUEST            0x17
#define FRAME_CREATE_SOURCE_ROUTE               0x21

#define FRAME_AT_CMD_RESPONSE                   0x88
#define FRAME_MODEM_STATUS                      0x8A
#define FRAME_ZB_TRANSMIT_STATUS                0x8B
#define FRAME_ZB_RECIEVE_PACKET                 0x90
#define FRAME_ZB_EXPLICIT_RX_INDICATOR          0x91
#define FRAME_ZB_IO_DATA_SAMPLE_RX_INDICATOR    0x92
#define FRAME_XB_SENSOR_READ_INDICATOR          0x94
#define FRAME_NODE_IDENTIFICATION_INDICATOR     0x95
#define FRAME_REMOTE_CMD_RESPONSE               0x97
#define FRAME_OTA_FIRMWARE_UPDATE_STATUS        0xA0
#define FRAME_ROUTE_RECORD_IDENTIFIER           0xA1
#define FRAME_MTO_ROUTE_REQUEST_INDICATOR       0xA3



#endif /* XBee_Commands_h */
