#pragma once
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
#include "hal.h"

#define USART_RX_BUFLEN 32
#define USART_TX_BUFLEN 16

#define USART_RX_MAX USART_RX_BUFLEN - 1
#define USART_TX_MAX USART_TX_BUFLEN - 1

namespace usart
{

	class Usart
	{
		friend class hal::usartRx;
		hal::usartRx usartRx;
		
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
	};

}