/*
* Usart.h
*
* Created: 4/7/2021 11:53:23 PM
*  Author: iceri
*/

// #include <stdlib.h>
// #include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#define BAUD 38400
#include <util/setbaud.h>

#include "RingBuffer.h"

#ifndef USART_H_
#define USART_H_

#define USART_RX_BUFLEN 32
#define USART_TX_BUFLEN 16

#define USART_RX_MAX USART_RX_BUFLEN - 1
#define USART_TX_MAX USART_TX_BUFLEN - 1

// extern "C" void UDRE_vec() __asm__("__vector_12") __attribute__((signal, used));
// extern "C" void RXC_vec() __asm__("__vector_11") __attribute__((signal, used));

#define CPPISR(vector) __asm__(vector);

class Usart
{

public:
	Usart();
	~Usart();

	void PutByte(uint8_t byte);
	uint8_t GetByte();

	void Init();

	//////////////////////////////////////////////////////////////////////////
	// write len bytes from buf to USART
	void Write(uint8_t *buf, uint8_t len);

	uint8_t Available();

	//////////////////////////////////////////////////////////////////////////
	// Read at max buflen bytes from USART to buf
	// returns number of bytes read or -1 on error
	uint8_t Read(uint8_t *buf, uint8_t buflen);

private:
	RingBuffer<USART_RX_BUFLEN> _rxBuf;
	RingBuffer<USART_TX_BUFLEN> _txBuf;

	//uint8_t _rxBuf[USART_RX_BUFLEN];
	//uint8_t _rxReadFrom;
	//uint8_t _rxWriteTo;
	//uint8_t _rxAvailable;
	//
	//uint8_t _txBuf[USART_TX_BUFLEN];
	//uint8_t _txReadFrom;
	//uint8_t _txWriteTo;
	//uint8_t _txAvailable;
//static void ed() __asm__("__vector_23") __attribute__((__signal__, __used__)){}

	static Usart *_usart;
};

#endif /* USART_H_ */