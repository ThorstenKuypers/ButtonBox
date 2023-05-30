/*
 * Usart.cpp
 *
 * Created: 4/10/2021 12:50:26 AM
 *  Author: iceri
 */

#include "../include/Usart.h"

using usart0 = Usart<usart0_traits>;

template <>
usart0 *usart0::_usart = 0;

// USART Data register empty interrupt handler
ISR(USART_UDRE_vect)
{
	usart0 *u = usart0::_usart;

	if (u->_txBuf.Available() == 0)
	{
		// no more bytes in TX-buffer, so disable UDRE interrupt
		// u->UCSRB0 &= ~_BV(UDRIE);
		u->UCSRB0 &= ~_BV(UDRIE);

		// clear UDRE flag, to stop further interrupts
		// u->UCSRA0 = _BV(UDRE);
	}
	else
	{
		// get byte from TX-buffer and send it down the line
		u->UDR00 = u->_txBuf.GetByte();
	}
}

// USART Data received interrupt handler
ISR(USART_RX_vect)
{
	usart0 *u = usart0::_usart;

	uint8_t b = u->UDR00;
	u->_rxBuf.PutByte(b);
}