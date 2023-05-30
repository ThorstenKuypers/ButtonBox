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

using device_register = uint8_t volatile;

template <typename addr, size_t offset = 0>
struct mmio_reg
{
	static volatile uint8_t *reg;
};

struct usart0_traits
{
	static constexpr device_register UCSRA0{0xC0};
	static constexpr device_register UCSRB0{0xC1};
	static constexpr device_register UCSRC0{0xC2};
	static constexpr device_register UBRRH0{0xC5};
	static constexpr device_register UBRRL0{0xC4};
	static constexpr device_register UDR00{0xC6};
};

template <typename usart0_traits>
class Usart
{

public:
	Usart() : UCSRA0{*reinterpret_cast<uint8_t *>(usart0_traits::UCSRA0)},
			  UCSRB0{*reinterpret_cast<uint8_t *>(usart0_traits::UCSRB0)},
			  UCSRC0{*reinterpret_cast<uint8_t *>(usart0_traits::UCSRC0)},
			  UBRRH0{*reinterpret_cast<uint8_t *>(usart0_traits::UBRRH0)},
			  UBRRL0{*reinterpret_cast<uint8_t *>(usart0_traits::UBRRL0)},
			  UDR00{*reinterpret_cast<uint8_t *>(usart0_traits::UDR00)}
	{
		Usart<usart0_traits>::_usart = this;
	}

	~Usart() = default;

	Usart(Usart &) = delete;
	Usart &operator=(Usart &) = delete;
	Usart(Usart &&) = delete;
	Usart &operator=(Usart &&) = delete;

	void Init()
	{
		// set the baud-rate according to table 60 in datasheet
		// 12 = 76.8k @ 16.000 MHz
		// UBRRH0 = 0;
		UBRRL0 = 51;

		// frame format: 8N1
		UCSRC0 = _BV(UCSZ1) | _BV(UCSZ0);
		// enable receiver/transmitter - interrupt driven
		UCSRB0 = _BV(RXCIE) | _BV(RXEN) | _BV(TXEN);
	}

	//////////////////////////////////////////////////////////////////////////
	// write len bytes from buf to USART
	void Write(uint8_t *buf, uint8_t len)
	{
		for (uint8_t i = 0; i < len; i++)
		{
			_txBuf.PutByte(buf[i]);
		}

		UCSRB0 |= _BV(UDRIE);
	}

	uint8_t Available()
	{
		return _rxBuf.Available();
	}

	//////////////////////////////////////////////////////////////////////////
	// Read at max buflen bytes from USART to buf
	// returns number of bytes read or -1 on error
	uint8_t Read(uint8_t *buf, uint8_t buflen)
	{
		uint8_t avail = Available();

		if (avail > buflen)
			avail = buflen;

		for (uint8_t i = 0; i < avail; i++)
		{
			buf[i] = _rxBuf.GetByte();
		}

		return avail;
	}

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
