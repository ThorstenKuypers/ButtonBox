#ifndef _USART_H_
#define _USART_H_

// USART.h - implementation of USART
//
// (C) 2013 - 
// This file is part of simDash
// 
////////////////////////////////////////////////////////////////////////

#include "simDash.h"


#define USART_RX_BUFLEN 32
#define USART_TX_BUFLEN 16

#define USART_RX_MAX USART_RX_BUFLEN - 1
#define USART_TX_MAX USART_TX_BUFLEN - 1


struct usart_rx_buf
{
	volatile uint8_t buf[USART_RX_BUFLEN];
	 uint8_t readfrom;
	 uint8_t writeto;
	 uint8_t avail;
};

struct usart_tx_buf
{
	volatile uint8_t buf[USART_TX_BUFLEN];
	 uint8_t readfrom;
	 uint8_t writeto;
	 uint8_t avail;
};


void init_usart();
void usart_putbyte(uint8_t c);
uint8_t usart_getbyte(void);

void usart_send(uint8_t *buf, uint8_t len);
uint8_t usart_avail();
uint8_t usart_tx_empty();

#endif /* _USART_H_ */
