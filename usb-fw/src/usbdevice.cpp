
#include <avr/interrupt.h>

#include "../inc/usbdevice.h"

volatile uint8_t usbState = 0;

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
                    UEDATX = pgm_read_byte(&usb_device_descriptor[i]);
                }
                UEINTX &= ~_BV(TXINI);
                // WaitTx();
                return;
            }
            if (2 == stp[3])
            {
                // config desc
                for (uint8_t i = 0; i < sizeof(usb_config_descriptor); i++)
                {
                    UEDATX = pgm_read_byte(&usb_config_descriptor[i]);
                }
                if (wLength >= sizeof(buttonbox_interface_descriptor) + sizeof(buttonbox_hid_descriptor) + sizeof(out_endpoint_escriptor) + sizeof(in_endpoint_escriptor))
                {
                    for (uint8_t i = 0; i < sizeof(buttonbox_interface_descriptor); i++)
                    {
                        UEDATX = pgm_read_byte(&buttonbox_interface_descriptor[i]);
                    }
                    for (uint8_t i = 0; i < sizeof(buttonbox_hid_descriptor); i++)
                    {
                        UEDATX = pgm_read_byte(&buttonbox_hid_descriptor[i]);
                    }
                    for (uint8_t i = 0; i < sizeof(out_endpoint_escriptor); i++)
                    {
                        UEDATX = pgm_read_byte(&out_endpoint_escriptor[i]);
                    }
                    for (uint8_t i = 0; i < sizeof(in_endpoint_escriptor); i++)
                    {
                        UEDATX = pgm_read_byte(&in_endpoint_escriptor[i]);
                    }
                }
                UEINTX &= ~_BV(TXINI);
                // WaitTx();
                return;
            }
            if (4 == stp[3])
            {
                for (uint8_t i = 0; i < sizeof(buttonbox_interface_descriptor); i++)
                {
                    UEDATX = pgm_read_byte(&buttonbox_interface_descriptor[i]);
                }
                UEINTX &= ~_BV(TXINI);
                return;
            }
            if (0 == stp[2] && 0x22 == stp[3])
            {
                // get HID descriptor
                for (uint8_t i = 0; i < sizeof(hidReportDescriptor); i++)
                {
                    UEDATX = pgm_read_byte(&hidReportDescriptor[i]);
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
        PORTD &= ~_BV(PD4);
}

uint8_t getUsbState()
{
    return usbState;
}