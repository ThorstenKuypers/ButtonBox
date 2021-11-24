/*
* Usart.cpp
*
* Created: 4/10/2021 12:50:26 AM
*  Author: iceri
*/

#include "../include/Usart.h"
#include "../include/hal.h"

Usart *Usart::_usart;

Usart::Usart()
//_rxReadFrom(0),
//_rxWriteTo(0),
//_rxAvailable(0),
//_txReadFrom(0),
//_txWriteTo(0),
//_txAvailable(0)
{
	Usart::_usart = this;
	//_rxBuf = RingBuffer(USART_RX_BUFLEN);
}

Usart::~Usart()
{
}

void Usart::Init()
{
	// set the baud-rate according to table 60 in datasheet
	// 12 = 76.8k @ 16.000 MHz
	UBRRH = 0;
	UBRRL = 51;

	// frame format: 8N1
	UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
	// enable receiver/transmitter - interrupt driven
	UCSRB = _BV(RXCIE) | _BV(RXEN) | _BV(TXEN) | _BV(TXCIE);
}

void Usart::PutByte(uint8_t byte)
{
	//_txBuf[_txWriteTo]=byte;
	//_txWriteTo++;
	//_txWriteTo &=USART_TX_MAX;
	//
	//_txAvailable++;
	_txBuf.PutByte(byte);
}

uint8_t Usart::GetByte()
{
	return 0;
}

void Usart::Write(uint8_t *buf, uint8_t len)
{
	uint8_t c = len;
	while (c > 0)
	{
		_txBuf.PutByte(*buf++);
		c--;
	}

	UCSRB |= _BV(UDRIE);
}

uint8_t Usart::Read(uint8_t *buf, uint8_t buflen)
{
	return 0;
}

uint8_t Usart::Available()
{
	return _rxBuf.Available();
}

// USART Data register empty interrupt handler
void UDRE_vec()
{
	Usart *u = Usart::_usart;

	if (u->_txBuf.Available() == 0)
	{
		// no more bytes in TX-buffer, so disable UDRE interrupt
		UCSRB &= ~_BV(UDRIE);

		// clear UDRE flag, to stop further interrupts
		UCSRA = _BV(UDRE);
	}
	else
	{
		// get byte from TX-buffer and send it down the line
		uint8_t b = u->_txBuf.GetByte();
		UDR = b;
	}
}

// USART Data received interrupt handler
void RXC_vec()
{
	Usart *u = Usart::_usart;

	byte_t b = UDR;
	u->_rxBuf.PutByte(b);
}