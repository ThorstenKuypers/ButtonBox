
#include "../include/twi.h"

template <>
uint8_t TwoWire<twi_traits>::device_address = 0;

template <>
TwoWire<twi_traits> TwoWire<twi_traits>::twi{};

template <>
uint8_t *TwoWire<twi_traits>::readBuf = nullptr;

template <>
uint8_t TwoWire<twi_traits>::readCnt = 0;

template <>
uint8_t TwoWire<twi_traits>::bytesToRead = 0;

using twi = TwoWire<twi_traits>;

ISR(TWI_vect)
{
    switch (TWSR)
    {
    case 0x08: // ACK for previous START condition
        twi::twi.TWIAR = twi::device_address & 0xFE;
        twi::twi.TWICR |= _BV(TWINT);
        break;

    case 0x40: // ACK for address
        twi::twi.TWICR |= _BV(TWINT) | _BV(TWEA);
        break;

    case 0x50: // data received and ACK sent
        if (twi::readCnt < twi::bytesToRead)
        {
            twi::twi.readBuf[twi::readCnt] = twi::twi.TWIDR;
            twi::readCnt++;

            twi::twi.TWICR |= _BV(TWINT) | _BV(TWEA);
        }
        else
        {
            // buffer full; so send stop condition
            twi::twi.TWICR = _BV(TWSTO) | _BV(TWINT) | _BV(TWEN); // send STOP condition (TWINT is NOT set after the STOP condition was sent!)
        }

        break;
    }
}

template <>
uint8_t TwoWire<twi_traits>::readFromDevice(uint8_t address, uint8_t *buf, uint8_t buflen)
{
    twi::device_address = address;
    twi::bytesToRead = buflen;
    twi::readBuf = buf;
    twi::readCnt = 0;

    TWICR = _BV(TWIE) | _BV(TWSTA) | _BV(TWINT);
    while (TWICR & _BV(TWIE))
        ;

    return twi::readCnt;
}