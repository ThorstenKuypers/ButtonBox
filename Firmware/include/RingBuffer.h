/*
* RingBuffer.h
*
* Created: 4/10/2021 10:55:06 PM
*  Author: iceri
*/
//////////////////////////////////////////////////////////////////////////
/* RingBuffer - This class implements a circular buffer array. This buffer
* fill up to its maximum capacity and than rolls over to the beginning. */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <inttypes.h>
#include <stdlib.h>

#define BUF_MAX_VAL(s) s - 1

template <uint8_t size>
class RingBuffer
{
public:
	RingBuffer() : _readfrom(0),
				   _writeto(0),
				   _avail(0),
				   _bufferSize(size)
	{
	}

	RingBuffer(const RingBuffer &b)
	{
		_avail = b._avail;
		_bufferSize = b._bufferSize;
		_buf = b._buf;
		_writeto = b._writeto;
		_readfrom = b._readfrom;
	}

	RingBuffer &operator=(const RingBuffer &&b)
	{
		_avail = b._avail;
		_buf = b._buf;
		_bufferSize = b._bufferSize;
		_readfrom = b._readfrom;
		_writeto = b._writeto;

		return *this;
	}

	~RingBuffer() {}

	void PutByte(uint8_t byte)
	{
		_buf[_writeto] = byte;
		_writeto++;
		_writeto &= BUF_MAX_VAL(_bufferSize);

		_avail++;
	}

	uint8_t GetByte()
	{
		uint8_t b = _buf[_readfrom];
		_readfrom++;
		_readfrom &= BUF_MAX_VAL(_bufferSize);
		_avail--;

		return b;
	}

	uint8_t Available()
	{
		return _avail;
	}

private:
	uint8_t _readfrom;
	uint8_t _writeto;
	uint8_t _avail;
	uint8_t _bufferSize;
	uint8_t _buf[size];
};

#endif /* RINGBUFFER_H_ */