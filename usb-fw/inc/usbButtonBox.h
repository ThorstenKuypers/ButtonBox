#pragma once 

#include <avr/io.h>

#include "ReportDesc.h"
#include "Usart.h"
#include "RingBuffer.h"

constexpr uint8_t numEndpoints = 2;

const uint8_t PROGMEM buttonbox_interface_descriptor[9] = {
    9,
    4,
    0,
    0,
    numEndpoints,
    3,
    0,
    0,
    0};

const uint8_t PROGMEM buttonbox_hid_descriptor[9] = {
    9,    // bLength
    0x21, // HID descriptor
    0x10, 0x01,
    0,
    1,
    0x22,                          // report descriptor
    sizeof(hidReportDescriptor), 0 // report descriptor length
};

const uint8_t PROGMEM out_endpoint_escriptor[7] = {
    7,
    5,
    0b10000001,
    0b00000011,
    8, 0,
    8};

const uint8_t PROGMEM in_endpoint_escriptor[7] = {
    7,
    5,
    0b00000010,
    0b00000011,
    8, 0,
    8};


void UpdateButtons();