
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "../inc/ReportDesc.h"
#include "../inc/Usart.h"
#include "../inc/RingBuffer.h"
#include "../inc/hal.h"

#ifndef F_CPU
#error "F_CPU not set!"
#endif

struct USB_Setup
{
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
    1};

const uint8_t configDescriptor[9] = {
    9, // bLength
    2, // bDescriptorType
    34, 00,
    1,
    1,
    0,
    0x80,
    200};

const uint8_t interfaceDescriptor[9] = {
    9,
    4,
    0,
    0,
    1,
    3,
    0,
    0,
    0};

const uint8_t hidDescriptor[9] = {
    9,    // bLength
    0x21, // HID descriptor
    0x10, 0x01,
    0,
    1,
    0x22,                          // report descriptor
    sizeof(hidReportDescriptor), 0 // report descriptor length
};

const uint8_t endpointDescriptor[7] = {
    7,
    5,
    0b10000001,
    0b00000011,
    8, 0,
    8};

volatile uint8_t cnt = 0;
volatile uint8_t buttons = 0;
volatile uint8_t btnUpdate = 0;
volatile uint8_t usbState = 0;
volatile uint32_t millis = 0;

#define RX_LED PD4
#define TX_LED PD5
#define SET_ADDRESS 5
#define GET_DESCRIPTOR 6

inline void WaitTx()
{
    while (!(UEINTX & _BV(TXINI)))
    {
    }
}

void initEp0()
{
    UENUM = 0; // EP0
    UECONX = 0;
    UECFG0X = 0;
    UECFG1X = (1 << EPSIZE0) | (1 << EPSIZE1) | (1 << ALLOC); // alloc 64 bytes for EP0
    UECONX |= _BV(EPEN);                                      // enable EP0
    UEIENX = (1 << RXSTPE);
    UDIEN = (1 << SOFE) | (1 << EORSTE);

    if (!(UESTA0X & _BV(CFGOK)))
        PORTD &= ~_BV(RX_LED);
}

int main(void)
{
    DDRD = _BV(RX_LED) | _BV(TX_LED);  // RX/TX led pins as output
    PORTD = _BV(RX_LED) | _BV(TX_LED); // ports high = LEDs off (LEDs are pulled up to 5V)

    cli();
    USBCON = _BV(USBE) | _BV(FRZCLK);
    // config PLL interface
    PLLCSR = (1 << PLLE) | (1 << PLLP0);
    while (!(PLLCSR & (1 << PLOCK))) // wait until PLL locked
    {
    }
    // enable USB controller and unfreeze clock
    USBCON = 0x80; // (1<<USBE)
    initEp0();
    UDCON = 0; // attach device
    
    hal::Timer1 timer1{_BV(CS10) | _BV(CS11) | _BV(WGM12), 250 - 1, _BV(OCIE1A)};
    
    sei();

    while (true)
    {
        if ((timer1.GetCount() - millis) >= 1000)
        {
            millis = timer1.GetCount();
            PORTD ^= _BV(TX_LED);            
        }

        if (usbState == 2)
        {
            // send back HID report
            if (btnUpdate != 0)
            {
                UENUM = 1;
                UEDATX = 0x01; // report ID
                UEDATX = buttons;
                UEINTX &= ~(_BV(TXINI) | _BV(FIFOCON));
                WaitTx();
                btnUpdate = 0;
            }
        }
    }
}

// Generic USB interrupt
ISR(USB_GEN_vect)
{
    uint8_t udint = UDINT;

    if (udint & (1 << EORSTI))
    {
        // end-of-reset
        initEp0();
        UDINT &= ~(1 << EORSTI);

        return;
    }

    if (udint & _BV(SOFI)) // start of frame sequence found; happens every 1ms
    {
        UDINT &= ~(1 << SOFI);
        return;
    }
}

// Endpoint interrupt
ISR(USB_COM_vect)
{
    uint8_t stp[8] = {0};

    if (UEINT & _BV(EPINT0))
        UENUM = 0;

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
                UEINTX &= ~_BV(TXINI);
                //WaitTx();
                return;
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
                UEINTX &= ~_BV(TXINI);
                //WaitTx();
                return;
            }
            if (4 == stp[3])
            {
                for (uint8_t i = 0; i < sizeof(interfaceDescriptor); i++)
                {
                    UEDATX = interfaceDescriptor[i];
                }
                UEINTX &= ~_BV(TXINI);
                return;
            }
            if (0 == stp[2] && 0x22 == stp[3])
            {
                // get HID descriptor
                for (uint8_t i = 0; i < sizeof(hidReportDescriptor); i++)
                {
                    UEDATX = hidReportDescriptor[i];
                }
                UEINTX &= ~_BV(TXINI);
                usbState = 2;
                return;
            }
        }
        else if (5 == stp[1]) // set address
        {
            UDADDR = stp[2];
            UEINTX &= ~_BV(TXINI); // set empty package back to acknowledge address
            WaitTx();
            UDADDR |= _BV(ADDEN);

            return;
        }
        else if (9 == stp[1])
        {
            // set config
            UEINTX &= ~_BV(TXINI);
            WaitTx();

            // the device is now in the configured state; so setup additional endpoint(s)
            UENUM = 1; // select EP1
            UECFG0X = _BV(EPTYPE0) | _BV(EPTYPE1) | _BV(EPDIR);
            UECFG1X = _BV(ALLOC);
            UECONX = _BV(EPEN);

            UENUM = 0;
            usbState = 1;

            return;
        }
        else if (0x0a == stp[1])
        {
            // SET_IDLE
            // the device is configured and ready, but the host has not got
            // the report descriptor. So it requests to hold any HID reports
            // for now. Because it can't interpret them yet.
            UEINTX &= ~_BV(TXINI);
            WaitTx();

            return;
        }
    }
}
