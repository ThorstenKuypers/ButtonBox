#pragma once

#include <avr/io.h>

#include "usbButtonBox.h"

constexpr uint16_t VendorID = 0xFEED;
constexpr uint16_t ProductID = 0xBEEF;
constexpr uint16_t Revision = 0x0100;
constexpr uint8_t InterfaceCount = 1;


const uint8_t PROGMEM usb_device_descriptor[18] = {
    18,
    1,
    0x00, 0x02,
    0,
    0,
    0,
    64,
    //0xED, 0xFE,
    (VendorID & 0xFF), ((VendorID >> 8) & 0xFF),
    //0xEF, 0xBE,
    (ProductID & 0xFF), ((ProductID >> 8) & 0xFF),
    //0x00, 0x01,
    (Revision & 0xFF), ((Revision >> 8) & 0xFF),
    0,
    0,
    0,
    1 // bNumConfigurations
    };

    const uint8_t PROGMEM usb_config_descriptor[9] = {
    9, // bLength
    2, // bDescriptorType
    34, 00,
    InterfaceCount,
    1,
    0,
    0x80,
    200};


void initEp0();

inline void WaitTx()
{
    while (!(UEINTX & _BV(TXINI)))
    {
    }
}

uint8_t getUsbState();