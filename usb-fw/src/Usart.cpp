/*
 * Usart.cpp
 *
 * Created: 4/10/2021 12:50:26 AM
 *  Author: iceri
 */

#include "../inc/Usart.h"

using namespace usart;

Usart::Usart()
{
	usartRx.setOwner(this);
	Init();
}

Usart::~Usart()
{
}

void Usart::Init()
{

	// set the baud-rate according to table 60 in datasheet
	// 12 = 76.8k @ 16.000 MHz
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;

	// frame format: 8N1
	UCSR1C = /*_BV(UMSEL10) |*/ _BV(UCSZ11) | _BV(UCSZ10);
	// enable receiver/transmitter - interrupt driven
	UCSR1B = _BV(RXCIE1) | _BV(RXEN1) | _BV(TXEN1);
}

void Usart::PutByte(uint8_t byte)
{
	_txBuf.PutByte(byte);
}

uint8_t Usart::GetByte()
{
	uint8_t b = _rxBuf.GetByte();
	return b;
}

void Usart::Write(uint8_t *buf, uint8_t len)
{
	uint8_t c = len;
	while (c > 0)
	{
		_txBuf.PutByte(*buf++);
		c--;
	}

	UCSR1B |= _BV(UDRIE1);
}

uint8_t Usart::Read(uint8_t *buf, uint8_t buflen)
{
	auto len = _rxBuf.Available();
	if (len > buflen)
		len = buflen;

	while (len--)
	{
		*buf = _rxBuf.GetByte();
		buf++;
	}

	return buflen;
}

uint8_t Usart::Available()
{
	return _rxBuf.Available();
}

// ISR(USART1_UDRE_vect)
// {

// }

// ISR(USART1_RX_vect)
// {

// }

// // USART Data register empty interrupt handler
// void UDRE_vec()
// {
// 	Usart *u = Usart::_usart;

// 	if (u->_txBuf.Available() == 0)
// 	{
// 		// no more bytes in TX-buffer, so disable UDRE interrupt
// 		UCSRB &= ~_BV(UDRIE);

// 		// clear UDRE flag, to stop further interrupts
// 		UCSRA = _BV(UDRE);
// 	}
// 	else
// 	{
// 		// get byte from TX-buffer and send it down the line
// 		uint8_t b = u->_txBuf.GetByte();
// 		UDR = b;
// 	}
// }

// // USART Data received interrupt handler
// void RXC_vec()
// {
// 	Usart *u = Usart::_usart;

// 	byte_t b = UDR;
// 	u->_rxBuf.PutByte(b);
// }