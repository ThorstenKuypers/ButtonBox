#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>

using device_register = volatile uint8_t;

#define TWISR_MASK 0xF8 // Bitmask to get TWI status code ignoring prescaler bits

extern "C" void TWI_vect(void) __attribute__((signal));

struct twi_traits
{
    static constexpr device_register TWICR{0x36 + __SFR_OFFSET};
    static constexpr device_register TWIBR{0 + __SFR_OFFSET};
    static constexpr device_register TWISR{0x1 + __SFR_OFFSET};
    static constexpr device_register TWIDR{0x3 + __SFR_OFFSET};
    static constexpr device_register TWIAR{0x2 + __SFR_OFFSET};
};

template <typename twi_traits>
class TwoWire
{
    friend void TWI_vect(void);

    uint8_t volatile &TWICR;
    uint8_t volatile &TWIBR;
    uint8_t volatile &TWISR;
    uint8_t volatile &TWIDR;
    uint8_t volatile &TWIAR;

    static TwoWire<twi_traits> twi;

    // I²c device address
    static uint8_t device_address;

    static uint8_t bytesToRead; // number of bytes to read for MasterReceive mode
    static uint8_t* readBuf; // receive buffer for MR mode
    static uint8_t readCnt; // number of bytes received

    uint8_t readFromDevice(uint8_t address, uint8_t *buf, uint8_t buflen);

    TwoWire() : TWICR{*reinterpret_cast<uint8_t *>(twi_traits::TWICR)},
                TWIBR{*reinterpret_cast<uint8_t *>(twi_traits::TWIBR)},
                TWISR{*reinterpret_cast<uint8_t *>(twi_traits::TWISR)},
                TWIDR{*reinterpret_cast<uint8_t *>(twi_traits::TWIDR)},
                TWIAR{*reinterpret_cast<uint8_t *>(twi_traits::TWIAR)}
    {
        // set frequency to 400khz
        TWIBR = 0x0C;
        TWISR = 0;
    }

public:
    static TwoWire &Instance() { return twi; }

    TwoWire(TwoWire &) = delete;
    TwoWire &operator=(TwoWire &) = delete;

    uint8_t ReadFromDevice(uint8_t address, uint8_t *buf, uint8_t numberToRead)
    {
        return readFromDevice(address, buf, numberToRead);
    }

};