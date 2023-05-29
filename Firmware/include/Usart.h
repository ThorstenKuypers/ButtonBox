#pragma once
/*
 * Usart.h
 *
 * Created: 4/7/2021 11:53:23 PM
 *  Author: iceri
 */

#include <stdlib.h>
#include <inttypes.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "../include/RingBuffer.h"

#ifdef __AVR_ATmega328P__
#define URSEL UMSEL00
#define UCSZ0 UCSZ00
#define UCSZ1 UCSZ01
#define RXCIE RXCIE0
#define RXEN RXEN0
#define TXEN TXEN0
#define TXCIE TXCIE0
#define UDRIE UDRIE0
#define UDRE UDRE0

#endif

#define USART_RX_BUFLEN 32
#define USART_TX_BUFLEN 16

#define USART_RX_MAX USART_RX_BUFLEN - 1
#define USART_TX_MAX USART_TX_BUFLEN - 1

// extern "C" void UDRE_vec() __asm__("__vector_12") __attribute__((signal, used));
// extern "C" void RXC_vec() __asm__("__vector_11") __attribute__((signal, used));

extern "C" void USART_UDRE_vect(void) __attribute__((signal, used));
extern "C" void USART_RX_vect(void) __attribute__((signal, used));

struct usart0_traits
{
	static constexpr volatile uint8_t UCSRA0{0xC0};
	static constexpr volatile uint8_t UCSRB0{0xC1};
	static constexpr volatile uint8_t UCSRC0{0xC2};
	static constexpr volatile uint8_t UBRRH0{0xC4};
	static constexpr volatile uint8_t UBRRL0{0xC5};
	static constexpr volatile uint8_t UDR00{0xC6};
};

template <typename usart0_traits>
class Usart
{

public:
	Usart();
	~Usart() = default;

	Usart(Usart &) = delete;
	Usart &operator=(Usart &) = delete;
	Usart(Usart &&) = delete;
	Usart &operator=(Usart &&) = delete;

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

	friend void USART_UDRE_vect();
	friend void USART_RX_vect();

private:
	RingBuffer<USART_RX_BUFLEN> _rxBuf;
	RingBuffer<USART_TX_BUFLEN> _txBuf;

	uint8_t volatile &UCSRA0;
	uint8_t volatile &UCSRB0;
	uint8_t volatile &UCSRC0;
	uint8_t volatile &UBRRH0;
	uint8_t volatile &UBRRL0;
	uint8_t volatile &UDR00;

	static Usart *_usart;
};
