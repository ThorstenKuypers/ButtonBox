/*
 * Usart.cpp
 *
 * Created: 4/10/2021 12:50:26 AM
 *  Author: iceri
 */

#include "../include/Usart.h"
// #include "../include/hal.h"

using usart0 = Usart<usart0_traits>;

template <>
usart0 *usart0::_usart = 0;

template <>
Usart<usart0_traits>::Usart() : _rxBuf(),
								_txBuf(),
								UCSRA0(*reinterpret_cast<uint8_t *>(usart0_traits::UCSRA0)),
								UCSRB0(*reinterpret_cast<uint8_t *>(usart0_traits::UCSRB0)),
								UCSRC0(*reinterpret_cast<uint8_t *>(usart0_traits::UCSRB0)),
								UBRRH0(*reinterpret_cast<uint8_t *>(usart0_traits::UBRRH0)),
								UBRRL0(*reinterpret_cast<uint8_t *>(usart0_traits::UBRRL0)),
								UDR00(*reinterpret_cast<uint8_t *>(usart0_traits::UDR00))
{
	usart0::_usart = this;
}

template <>
void Usart<usart0_traits>::Init()
{
	// set the baud-rate according to table 60 in datasheet
	// 12 = 76.8k @ 16.000 MHz
	UBRRH0 = 0;
	UBRRL0 = 51;

	// frame format: 8N1
	UCSRC0 = _BV(UCSZ1) | _BV(UCSZ0);
	// enable receiver/transmitter - interrupt driven
	UCSRB0 = _BV(RXCIE) | _BV(RXEN) | _BV(TXEN) | _BV(TXCIE);
}

template <>
void Usart<usart0_traits>::PutByte(uint8_t byte)
{
	_txBuf.PutByte(byte);
}

template <>
uint8_t Usart<usart0_traits>::GetByte()
{
	return 0;
}

template <>
void Usart<usart0_traits>::Write(uint8_t *buf, uint8_t len)
{
	for (uint8_t i = 0; i < len; i++)
	{
		_txBuf.PutByte(buf[i]);
	}

	UCSRB0 |= _BV(UDRIE);
}

template <>
uint8_t Usart<usart0_traits>::Read(uint8_t *buf, uint8_t buflen)
{
	return 0;
}

template <>
uint8_t Usart<usart0_traits>::Available()
{
	return _rxBuf.Available();
}

// USART Data register empty interrupt handler
ISR(USART_UDRE_vect)
{
	usart0 *u = usart0::_usart;

	if (u->_txBuf.Available() == 0)
	{
		// no more bytes in TX-buffer, so disable UDRE interrupt
		u->UCSRB0 &= ~_BV(UDRIE);

		// clear UDRE flag, to stop further interrupts
		u->UCSRA0 = _BV(UDRE);
	}
	else
	{
		// get byte from TX-buffer and send it down the line
		uint8_t b = u->_txBuf.GetByte();
		u->UDR00 = b;
	}
}

// USART Data received interrupt handler
ISR(USART_RX_vect)
{
	usart0 *u = usart0::_usart;

	uint8_t b = u->UDR00;
	u->_rxBuf.PutByte(b);
}