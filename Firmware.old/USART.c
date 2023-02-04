// USART.c - implementation of USART
//
// (C) 2013 - 
// This file is part of simDash
// 
////////////////////////////////////////////////////////////////////////

#include "USART.h"
#include "SerialProto.h"

 static volatile struct usart_rx_buf usart_rx ={{0}, 0, 0, 0};
 static volatile struct usart_tx_buf usart_tx ={{0}, 0, 0, 0};

static uint8_t volatile tx_cpl;

////////////////////////////////////////////////////////////////////////
//
// void init_usart()
// 
// initializes the USART RX/TX module
// 
// parameters: none
// return: nothing
////////////////////////////////////////////////////////////////////////
void init_usart()
{
	// set the baud-rate according to table 60 in datasheet
	// 12 = 76.8k @ 16.000 MHz
	UBRRH = 0;
	UBRRL = 51;
	usart_tx.avail =0;
	tx_cpl =1;

	// enable receiver/transmitter - interrupt driven
	UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN)|(1<<TXCIE);
	// frame format: 8N1
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);

}

////////////////////////////////////////////////////////////////////////
//
// void putbyte(uint8_t c)
// 
// writes one byte to usart tx buffer for later transmission
// 
// parameters: byte to send over USART
// return: nothing
////////////////////////////////////////////////////////////////////////
void usart_putbyte(uint8_t c)
{
	usart_tx.buf[usart_tx.writeto] =c;
	usart_tx.writeto++;
	usart_tx.writeto &= USART_TX_MAX;

	usart_tx.avail++;
}

uint8_t usart_avail()
{
	return usart_rx.avail;
}


void usart_send(uint8_t *buf, uint8_t len)
{
	tx_cpl =0;

	do {
		
		usart_putbyte(*buf++);
		len--;
	} while(len);

	// enable UDRE interrupt
	UCSRB |=(1<<UDRIE);
}



////////////////////////////////////////////////////////////////////////
//
// uint8_t getbyte(void)
// 
// receives one byte from usart rx buffer
// 
// parameters: nothing
// return: received byte
////////////////////////////////////////////////////////////////////////
uint8_t usart_getbyte(void)
{
	if (usart_rx.avail ==0)
		return 0;

	uint8_t c = usart_rx.buf[usart_rx.readfrom];
	usart_rx.readfrom++;
	usart_rx.readfrom &= USART_RX_MAX;
	
	usart_rx.avail--;

	// set DTR LOW, to signal PC that sending data is allowed
	// MAX232 inverts the signal
	if (usart_rx.avail < USART_RX_MAX)
		PORTD &= ~_BV(CTS);

	return c;
}

uint8_t usart_tx_empty()
{
	if ((tx_cpl ==1) && (usart_tx.avail == 0)) {
		//UCSRA |=(1<<TXC);
		return 1;
	}

	return 0;
}

//////////////////////////// ISRs /////////////////////////////////////
///////////////////////////////////////////////////////////////////////
ISR(USART_RXC_vect)
{
	uint8_t c =UDR;

	usart_rx.buf[usart_rx.writeto] =c;
	usart_rx.writeto++;
	usart_rx.writeto &= USART_RX_MAX;
	usart_rx.avail++;

	// !!! EXPERIMENTAL !!!
	// if RX buf is full -> usart_rx.avail ==USART_RX_MAX
	// drive CTS line HIGH to signal send hold to PC
	// PC MUST check CTS line EVERY time prior to sending
	// data to uC
	// MAX232 inverts the signal
	if (usart_rx.avail ==USART_RX_MAX)
		PORTD |= _BV(CTS);
}

ISR(USART_TXC_vect)
{
}

ISR(USART_UDRE_vect)
{
	if (usart_tx.avail ==0)
	{
		// no more bytes in TX-buffer, so disable UDRE interrupt
		UCSRB &= ~(1<<UDRIE);
		tx_cpl =1;

		// clear UDRE flag, to stop further interrupts
		UCSRA = (1<<UDRE);
	}
	else
	{
		tx_cpl =0;
		// get byte from TX-buffer and send it down the line
		UDR =usart_tx.buf[usart_tx.readfrom];
		usart_tx.readfrom++;
		usart_tx.readfrom &= USART_TX_MAX;
		usart_tx.avail--;
	}	
}
