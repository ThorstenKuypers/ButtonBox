
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

// #include "../inc/hal.h"
#include "../inc/timer.h"

#include "../inc/usbdevice.h"

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




volatile uint8_t cnt = 0;
volatile uint32_t millis = 0;

#define RX_LED PD4
#define TX_LED PD5
#define SET_ADDRESS 5
#define GET_DESCRIPTOR 6

int main(void)
{
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
    
    timer::Timer1 timer1{_BV(CS10) | _BV(CS11) | _BV(WGM12), 250 - 1, _BV(OCIE1A)};
    
    DDRD = _BV(RX_LED);// | _BV(TX_LED);  // RX/TX led pins as output
    PORTD = _BV(RX_LED);// | _BV(TX_LED); // ports high = LEDs off (LEDs are pulled up to 5V)

    sei();

    while (true)
    {
        UpdateButtons();

        if ((timer1.GetCount() - millis) >= 1000)
        {
            millis = timer1.GetCount();
            // buttons ^= _BV(0);
            // btnUpdate = 1;
        }

     }
}


