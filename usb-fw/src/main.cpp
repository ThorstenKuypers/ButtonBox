
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "ReportDesc.h"

#ifndef F_CPU
#error "F_CPU not set!"
#endif

struct USB_Setup {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint8_t wValueL;
    uint8_t wValueH;
    uint16_t wIndex;
    uint16_t wLength;
};

// TODO: configure and enable EP1 for sending
//       HID reports

const uint8_t devDesc[18] = {
    18,
    1,
    0x00, 0x02,
    0,
    0,
    0,
    64,
    0xED, 0xFE,
    0xEF, 0xBE,
    0x00, 0x01,
    0,
    0,
    0,
    1
};

const uint8_t configDescriptor[9] = {
    9, // bLength
    2, // bDescriptorType
    34,00,
    1,
    1,
    0,
    0x80,
    50
};

const uint8_t interfaceDescriptor[9] = {
    9,
    4,
    0,
    0,
    1,
    3,
    0,
    0,
    0
};

const uint8_t hidDescriptor[9] = {
    9, // bLength
    0x21, // HID descriptor
    0x10, 0x01,
    0,
    1,
    0x22, // report descriptor
    sizeof(hidReportDescriptor), 0 // report descriptor length
};

const uint8_t endpointDescriptor[7] = {
    7,
    5,
    0b10000001,
    0b00000011,
    8, 0,
    8
};

#define RX_LED PD4
#define TX_LED PD5


void WaitTx()
{
    while(!(UEINTX & _BV(TXINI))) {}
}

void initEp0()
{
    //cli();
    UENUM = 0; // EP0
    UECONX = 0;
    UECFG0X = 0;
    UECFG1X = (1<<EPSIZE0)|(1<<EPSIZE1)|(1<<ALLOC); // alloc 64 bytes for EP0
    UECONX |= _BV(EPEN); // enable EP0
    UEIENX = (1<<RXSTPE);
    UDIEN = (1<<SOFE)|(1<<EORSTE);
    //sei();

    if (!(UESTA0X & _BV(CFGOK)))
        PORTD &= ~_BV(RX_LED);
}

// Generic USB interrupt
ISR(USB_GEN_vect)
{
    uint8_t udint = UDINT;

    if (udint & (1<<EORSTI))
    {
        // end-of-reset
        initEp0();
        UDINT &= ~(1<<EORSTI);
    }

    if (udint & _BV(SOFI)) // start of frame sequence found; happens every 1ms
    {
        UDINT &= ~(1<<SOFI);
    }
}

// Endpoint interrupt
ISR(USB_COM_vect)
{
    uint8_t stp[8] = {0};

    if (UEINTX & (1 << RXSTPI)) // setup packet received
    {

        for (uint8_t i = 0; i < 8; i++)
        {
            stp[i] = UEDATX;
        }
        UEINTX &= ~(1 << RXSTPI); // acknowlege interrupt
        uint16_t wLength = (stp[7] << 8) | (stp[6] & 0xFF);

        WaitTx();
        if (6 == stp[1]) // device descriptor
        {
            if (1 == stp[3])
            {
                for (uint8_t i = 0; i < 18; i++)
                {
                    UEDATX = devDesc[i];
                }
                UEINTX = ~_BV(TXINI);
                WaitTx();
                PORTD &= ~_BV(RX_LED);
            }
            if (2 == stp[3])
            {
                // config desc
                for (uint8_t i = 0; i < sizeof(configDescriptor); i++)
                {
                    UEDATX = configDescriptor[i];
                }
                if (wLength == 34)
                {
                    for (uint8_t i = 0; i < sizeof(interfaceDescriptor); i++)
                    {
                        UEDATX = interfaceDescriptor[i];
                    }
                    for (uint8_t i = 0; i < sizeof(hidDescriptor); i++)
                    {
                        UEDATX = hidDescriptor[i];
                    }
                    for (uint8_t i = 0; i < sizeof(endpointDescriptor); i++)
                    {
                        UEDATX = endpointDescriptor[i];
                    }
                }
                UEINTX = ~_BV(TXINI);
                WaitTx();
            }
            if (4 == stp[3])
            {
                for (uint8_t i = 0; i < sizeof(interfaceDescriptor); i++)
                {
                    UEDATX = interfaceDescriptor[i];
                }
                UEINTX = ~_BV(TXINI);
            }
            if (0 == stp[2] && 0x22 == stp[3])
            {
                // get HID descriptor
                for (uint8_t i = 0; i < sizeof(hidReportDescriptor); i++)
                {
                    UEDATX = hidReportDescriptor[i];
                }
                UEINTX = ~_BV(TXINI);
                WaitTx();
            }
        }
        else if (5 == stp[1]) // set address
        {
            UDADDR = stp[2];
            UEINTX &= ~_BV(TXINI); // set empty package back to acknowledge address
            WaitTx();
            UDADDR |= _BV(ADDEN);
        }
        else if (9 == stp[1])
        {
            // set config
            UEINTX = ~_BV(TXINI);
            WaitTx();
        }
        else if (0x0a == stp[1])
        {
            UEINTX = ~_BV(TXINI);
            WaitTx();
        }
    }
}

int main(void)
{
    DDRD = _BV(RX_LED)|_BV(TX_LED); // RX/TX led pins as output
    PORTD = _BV(RX_LED)|_BV(TX_LED); // ports high = LEDs off (LEDs are pulled up to 5V)

    cli();
    USBCON = _BV(USBE)|_BV(FRZCLK);
    // config PLL interface
    PLLCSR = (1<<PLLE)|(1<<PLLP0);
    while(!(PLLCSR & (1<<PLOCK))) // wait until PLL locked
    {      
    }
    // enable USB controller and unfreeze clock
    USBCON = 0x80;  // (1<<USBE)
    initEp0();
    UDCON = 0; //&= ~(1<<DETACH); // attach device
    sei();

    while(true)
    {
        _delay_ms(1);
    }
 }