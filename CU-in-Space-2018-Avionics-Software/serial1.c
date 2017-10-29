//
//  serial1.c
//  CU-in-Space-2018-Avionics-Software
//
//  Created by Samuel Dewan on 2017-10-28.
//

#include "serial1.h"

#include "pindefinitions.h"

#include <avr/io.h>
#include <util/atomic.h>
#include <avr/eeprom.h>
#include <ctype.h>

// MARK: Constants
#define serial_in_buffer_length     256
#define serial_out_buffer_length    256

// MARK: Variable Definitions
static char serial_in_buffer[serial_in_buffer_length];
static char serial_out_buffer[serial_out_buffer_length];

static volatile uint8_t in_buffer_insert_p;
static volatile uint8_t in_buffer_withdraw_p;
static volatile uint8_t out_buffer_insert_p;
static volatile uint8_t out_buffer_withdraw_p;

// MARK: Function Definitions
void init_serial_1 (void)
{
    serial_in_buffer[in_buffer_insert_p] = '\0';
    serial_out_buffer[out_buffer_insert_p] = '\0';
    
    UBRR1H = 0;                                     // Set baud rate: 9.6Kbaud at 12mhz clock, 0.1602564103% error
    UBRR1L = 77;
    UCSR1B |= (1<<TXEN1)|(1<<TXCIE1)|(1<<RXEN1)|(1<<RXCIE1); // Enable transmitter and reciver, TX and RX interupts enabled
    UCSR1C = (1<<UCSZ10)|(1<<UCSZ11);               // Set frame format: 8 data, 1 stop bit(s)
}

void serial_1_put_string (char *str)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        for (int i = 0; str[i] != '\0'; i++) {
            serial_out_buffer[out_buffer_insert_p] = str[i];
            out_buffer_insert_p++;
            if (out_buffer_insert_p == out_buffer_withdraw_p) {
                out_buffer_withdraw_p++;
            }
            
            if (str[i] == '\n') {                   // Insert a carriage return after new lines
                serial_out_buffer[out_buffer_insert_p] = '\r';
                out_buffer_insert_p++;
                if (out_buffer_insert_p == out_buffer_withdraw_p) {
                    out_buffer_withdraw_p++;
                }
            }
        }
        serial_out_buffer[out_buffer_insert_p] = '\0';
        serial_1_service();                           // Start transmition right away
    }
}

void serial_1_put_string_P (const char *str)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        char next = pgm_read_byte(&str[0]);
        for (int i = 0; next != '\0'; i++, next = pgm_read_byte(&str[i])) {
            serial_out_buffer[out_buffer_insert_p] = next;
            out_buffer_insert_p++;
            if (out_buffer_insert_p == out_buffer_withdraw_p) {
                out_buffer_withdraw_p++;
            }
            
            if (next == '\n') {                     // Insert a carriage return after new lines
                serial_out_buffer[out_buffer_insert_p] = '\r';
                out_buffer_insert_p++;
                if (out_buffer_insert_p == out_buffer_withdraw_p) {
                    out_buffer_withdraw_p++;
                }
            }
        }
        serial_out_buffer[out_buffer_insert_p] = '\0';
        serial_1_service();                           // Start transmition right away
    }
}

void serial_1_put_from_eeprom (uint16_t addr)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        char next = eeprom_read_byte((uint8_t*)addr);
        for (int i = 0; next != '\0'; i++, next = eeprom_read_byte((uint8_t*)(addr + i))) {
            serial_out_buffer[out_buffer_insert_p] = next;
            out_buffer_insert_p++;
            if (out_buffer_insert_p == out_buffer_withdraw_p) {
                out_buffer_withdraw_p++;
            }
            
            if (next == '\n') {                       // Insert a carriage return after new lines
                serial_out_buffer[out_buffer_insert_p] = '\r';
                out_buffer_insert_p++;
                if (out_buffer_insert_p == out_buffer_withdraw_p) {
                    out_buffer_withdraw_p++;
                }
            }
        }
        serial_out_buffer[out_buffer_insert_p] = '\0';
        serial_1_service();                           // Start transmition right away
    }
}

void serial_1_put_byte (char c)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {             // Must use restorestate as this function may be called from ISR
        serial_out_buffer[out_buffer_insert_p] = c;
        out_buffer_insert_p++;
        serial_out_buffer[out_buffer_insert_p] = '\0';
        if (out_buffer_insert_p == out_buffer_withdraw_p) {
            out_buffer_withdraw_p++;
        }
    }
    
    if (c == '\n') {                                // Insert a carriage return after new lines
        serial_1_put_byte('\r');
    }
}

int serial_1_has_line (void)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        for (int i = in_buffer_withdraw_p; i != in_buffer_insert_p; i++) {
            if (serial_in_buffer[i] == '\n') {
                return 1;
            }
        }
    }
    return 0;
}

void serial_1_get_string (char *str, int len)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        for (int i = 0; i < (len - 1); i++) {
            if (in_buffer_withdraw_p != in_buffer_insert_p) {
                str[i] = serial_in_buffer[in_buffer_withdraw_p];
                in_buffer_withdraw_p++;
            } else {
                str[i] = '\0';
                return;
            }
        }
    }
}

extern void serial_1_get_line (char *str, int len) {
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        for (int i = 0; i < (len - 1); i++) {
            if (in_buffer_withdraw_p != in_buffer_insert_p) {
                if (serial_in_buffer[in_buffer_withdraw_p] == '\n') {
                    str[i] = '\0';
                    in_buffer_withdraw_p++;
                    return;
                    
                } else {
                    str[i] = serial_in_buffer[in_buffer_withdraw_p];
                    in_buffer_withdraw_p++;
                }
            } else {
                str[i] = '\0';
                return;
            }
        }
    }
}

char serial_1_get_byte (void)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        in_buffer_withdraw_p = (in_buffer_withdraw_p == in_buffer_insert_p) ? in_buffer_withdraw_p : in_buffer_withdraw_p + 1;
    }
    return serial_in_buffer[in_buffer_withdraw_p];
}

char serial_1_peak_byte (void)
{
    int index;
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        index = (in_buffer_withdraw_p == in_buffer_insert_p) ? in_buffer_withdraw_p : in_buffer_withdraw_p + 1;
    }
    return serial_in_buffer[index];
}

void serial_1_service (void)
{
    // If serial transmition is not locked (IE. a transmition is not already in progress concurently via the TX ISR) and there
    // are avaliable bytes, write a byte to the serial port and lock transmition. This will start transmition of all avaliable
    // bytes concurently via the ISR.
    if ((!(flags & (1<<FLAG_SERIAL_1_TX_LOCK))) && (out_buffer_withdraw_p != out_buffer_insert_p)) {
        flags |= (1<<FLAG_SERIAL_1_TX_LOCK);
        UDR1 = serial_out_buffer[out_buffer_withdraw_p];
        out_buffer_withdraw_p++;
    }
}

// MARK: Interupt service routines
ISR (USART1_TX_vect)                                 // Transmit finished on USART0
{
    if (out_buffer_withdraw_p != out_buffer_insert_p) {
        UDR1 = serial_out_buffer[out_buffer_withdraw_p];
        out_buffer_withdraw_p++;
    } else {
        flags &= ~(1<<FLAG_SERIAL_1_TX_LOCK);         // Clear serial transmition lock
    }
}

ISR (USART1_RX_vect)                                 // Recieved byte on USART0
{
    //    uint8_t usart_state = UCSR0A;                 //get state before data!
    uint8_t usart_byte = UDR1;                        //get data
    // TODO: Check state state to make sure nothing exploded
    
    usart_byte = (usart_byte == '\r') ? '\n' : usart_byte;
    
    if (!iscntrl(usart_byte) || (usart_byte == '\n')) {
        serial_in_buffer[in_buffer_insert_p] = usart_byte;
        in_buffer_insert_p++;
        serial_in_buffer[in_buffer_insert_p] = '\0';
        if (in_buffer_insert_p == in_buffer_withdraw_p) {
            in_buffer_withdraw_p++;
        }
    }
    
    // If loop back is enabled, append the recieved byte to the output buffer
    if (flags & (1<<FLAG_SERIAL_1_LOOPBACK) && (isprint(usart_byte) || (usart_byte == '\n'))) {
        serial_1_put_byte(usart_byte);
    } else if (flags & (1<<FLAG_SERIAL_1_LOOPBACK) && (usart_byte == 127) && (in_buffer_withdraw_p != in_buffer_insert_p)) {
        in_buffer_insert_p --;
        serial_1_put_byte(0x1B);
        serial_1_put_string("[1D");
        serial_1_put_byte(0x1B);
        serial_1_put_string("[K");
    }
}
