
#include <gtest/gtest.h>

#include "RingBuffer.h"

struct UsartBase
{
	RingBuffer<16> _rxBuf;
	RingBuffer<32> _txBuf;

	virtual uint8_t read(uint8_t* buf, uint8_t len) = 0;
	virtual uint8_t getc() = 0;
};

struct Twi
{

};

struct UsartMock : UsartBase
{
	uint8_t UDR1;

	virtual uint8_t read(uint8_t* buf, uint8_t len) override
	{
		return 0;
	}

	virtual uint8_t getc() override
	{
		return UDR1;
	}
};

struct ButtonBox
{
	const volatile UsartBase* _usart;
	const volatile Twi* _twi;
	explicit ButtonBox(const UsartBase* usart, const Twi* twi) :
		_usart(usart),
		_twi(twi) {}

	void ReadButtonStates() {}

};

TEST(TestCaseName, TestName) {

	UsartMock usart;
	Twi twi;
	ButtonBox bb{ &usart, &twi };

	usart.UDR1 = 0x12;
	bb.ReadButtonStates();
}