//
//  SPI.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "SPI.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

// MARK: Constants
#define SPI_BUFFER_FRAME_LENGTH         256
#define SPI_BUFFER_MAX_FRAMES           4

#define EOT_CHAR_LITERAL                4  //␄ char

#define SPI_FLAG_ACTIVE 0
#define SPI_FD_BYTE_RECIEVED 1  // True if a byte was recieved last time the SPI interupt ran

// MARK: Variable Definitions
/** Buffer in which data recieved during full duplex operation of the SPI bus is stored*/
static uint8_t spi_full_duplex_buffer[SPI_BUFFER_MAX_FRAMES][SPI_BUFFER_FRAME_LENGTH];

/** The position in the full duplex input buffer where the next byte to be added should go*/
static volatile uint8_t full_duplex_buffer_insert_p;
/** The position in the full duplex input buffer where the next byte should be read from*/
static volatile uint8_t full_duplex_buffer_withdraw_p;

/** The index in the current full duplex input buffer where the next byte should be placed*/
static volatile uint8_t full_duplex_input_possition;

/** The input register where the ATTN pin is located*/
static volatile uint8_t *attn_pin_reg;
/** The bit number for the ATTN pin within the input registe*/
static uint8_t attn_pin_num;
/** The outpyt register where the SPU pins are located*/
static volatile uint8_t *spi_port_reg;
/** The bit number for the CS pin of the full duplex device within the output register*/
static uint8_t full_duplex_cs_num;

/** The number of bytes to be sent during the current transaction*/
static volatile uint8_t num_bytes_out;
/** The buffer from which bytes are sent*/
static uint8_t *out_buffer;
/** The number of bytes to be received during the current half duplex transaction*/
static volatile uint8_t num_bytes_in;
/** The buffer in which bytes recieved in half duplex are placed*/
static uint8_t *in_buffer;
/** The possition whithin the current transaction*/
static uint8_t possition;
/** The bit number for the CS pin of the device currently being accessed within the output register*/
static uint8_t hd_cs_num;

static volatile uint8_t spi_flags;

// MARK: Function Definitions
void init_spi(volatile uint8_t *spi_port)
{
    spi_port_reg = spi_port;
    // Set up SPI as follows:
    //  SPI interupt enabled
    //  SPI master mode
    //  SPI polarity/phase mode 0
    //  SPI clock rate is F_OSC/4 (3MHz)
    // and enable it
    SPCR |= (1<<SPIE) | (1<<SPE) | (1<<MSTR);
}

void spi_register_full_duplex(volatile uint8_t *attn_pin, uint8_t attn_num, uint8_t cs_num)
{
    attn_pin_reg = attn_pin;
    attn_pin_num = attn_num;
    full_duplex_cs_num = cs_num;
}

void spi_service(void)
{
    if (!(spi_flags & (1<<SPI_FLAG_ACTIVE)) && (*attn_pin_reg & (1<<attn_pin_num))) {
        spi_flags |= (1<<SPI_FLAG_ACTIVE);          // Lock SPI interface
        *spi_port_reg &= ~(1<<full_duplex_cs_num);  // Select full duplex device
        
        spi_full_duplex_buffer[full_duplex_buffer_insert_p][0] = 0;
        
        num_bytes_out = 0;
        full_duplex_input_possition = 1;
        SPDR = 0;                                  // Write junk to the SPI bus to start the clock
    }
}

uint8_t spi_transfer_active(void)
{
    return spi_flags & (1<<SPI_FLAG_ACTIVE);
}

uint8_t spi_full_duplex_has_transmition(void)
{
    return (full_duplex_buffer_insert_p != full_duplex_buffer_withdraw_p) &&
        (spi_full_duplex_buffer[full_duplex_buffer_withdraw_p][0] != 0);
}

uint8_t spi_full_duplex_get_transmition(uint8_t *buf, int len)
{
    if ((full_duplex_buffer_insert_p == full_duplex_buffer_withdraw_p) ||
        (spi_full_duplex_buffer[full_duplex_buffer_withdraw_p][0] == 0)) {
        return 0;
    }
    uint8_t *transmition = spi_full_duplex_buffer[full_duplex_buffer_withdraw_p];
    full_duplex_buffer_withdraw_p++;
    full_duplex_buffer_withdraw_p = (full_duplex_buffer_withdraw_p > SPI_BUFFER_MAX_FRAMES) ? 0 : SPI_BUFFER_MAX_FRAMES;
    
    uint8_t bytes = (len > SPI_BUFFER_FRAME_LENGTH) ? SPI_BUFFER_FRAME_LENGTH : len;
    memcpy(buf, transmition, bytes);
    return bytes;
}

uint8_t spi_start_half_duplex(uint8_t cs_num, uint8_t bytes_out, uint8_t *out_buf, uint8_t bytes_in, uint8_t *in_buf)
{
    if (spi_flags & (1<<SPI_FLAG_ACTIVE)) {
        return 0;
    }
    spi_flags |= (1<<SPI_FLAG_ACTIVE);
    
    hd_cs_num = cs_num;
    num_bytes_out = bytes_out;
    out_buffer = out_buf;
    num_bytes_in = bytes_in;
    in_buffer = in_buf;
    
    possition = 1;
    SPDR = out_buffer[0];
    
    return 1;
}

uint8_t spi_start_full_duplex_write(uint8_t bytes_out, uint8_t *out_buf)
{
    uint8_t active = spi_flags & (1<<SPI_FLAG_ACTIVE);
    if (active && !(active & (*attn_pin_reg & (1<<attn_pin_num)))) {
        return 0;
    }
    spi_flags |= (1<<SPI_FLAG_ACTIVE);
    
    out_buffer = out_buf;
    num_bytes_out = bytes_out;
    
    if (!active) {
        possition = 1;
        SPDR = out_buffer[0];
    } else {
        possition = 0;
    }
    
    return 1;
}

// MARK: Interupt service routines
ISR (SPI_STC_vect)
{
    // Recieve byte
    if ((*spi_port_reg & ~(1<<full_duplex_cs_num)) && (*attn_pin_reg & (1<<attn_pin_num))) {
        // Recieve byte in full duplex
        spi_flags |= (1<<SPI_FD_BYTE_RECIEVED);
        spi_full_duplex_buffer[full_duplex_buffer_insert_p][full_duplex_input_possition] = SPDR;
        spi_full_duplex_buffer[full_duplex_buffer_insert_p][0] = 0;
        full_duplex_input_possition++;
    } else if ((*spi_port_reg & ~(1<<full_duplex_cs_num)) && (spi_flags & (1<<SPI_FD_BYTE_RECIEVED))) {
        // Finished receiving in full duplex
        spi_flags &= ~(1<<SPI_FD_BYTE_RECIEVED);
        spi_full_duplex_buffer[full_duplex_buffer_insert_p][0] = full_duplex_input_possition;
        full_duplex_buffer_insert_p++;
        full_duplex_buffer_insert_p = (full_duplex_buffer_insert_p > SPI_BUFFER_MAX_FRAMES) ? 0 : SPI_BUFFER_MAX_FRAMES;
        if (full_duplex_buffer_insert_p == full_duplex_buffer_withdraw_p) {
            full_duplex_buffer_withdraw_p++;
        }
         *spi_port_reg |= (1<<full_duplex_cs_num);    // raise CS
        spi_flags &= ~(1<<SPI_FLAG_ACTIVE);
    } else if ((num_bytes_out = 0) && (possition < num_bytes_in)) {
        // Recieve bytes in half duplex
        in_buffer[possition + 1] = SPDR;
        possition++;
    } else if ((num_bytes_out = 0) && (possition == num_bytes_in)) {
        // Finished recieving bytes in half duplex
        in_buffer[0] = possition + 1;
    }
    
    // Transmit byte
    if (possition < num_bytes_out) {
        SPDR = out_buffer[possition];
        possition++;
        if (possition == num_bytes_out) {
            // Done transmitting
            possition = 1;
            num_bytes_out = 0;
        }
        if ((num_bytes_in == 0) && !(*spi_port_reg & ~(1<<full_duplex_cs_num))) {
            // Finished transaction
            *spi_port_reg |= (1<<hd_cs_num);    // raise CS
            spi_flags &= ~(1<<SPI_FLAG_ACTIVE);
        }
    } else if ((spi_flags & (1<<SPI_FD_BYTE_RECIEVED) || (possition < num_bytes_in))) {
        // Send junk data to continue recieving
        SPDR = 0;
    }
}
