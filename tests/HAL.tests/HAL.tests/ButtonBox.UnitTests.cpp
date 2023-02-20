
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "RingBuffer.h"

using namespace ::testing;

struct UsartBase
{
	RingBuffer<16> _rxBuf;
	RingBuffer<32> _txBuf;

	virtual uint8_t read(uint8_t* buf, uint8_t len) = 0;
	virtual uint8_t getc() = 0;
};

struct UsartMock :public UsartBase
{
	MOCK_METHOD(uint8_t, read, (uint8_t* buf, uint8_t len), (override));
	MOCK_METHOD(uint8_t, getc, (), (override));
};

struct TwiBase
{

};

struct ButtonBox
{
	UsartBase* _usart;
	TwiBase* _twi;
	explicit ButtonBox(UsartBase* usart, TwiBase* twi) :
		_usart(usart),
		_twi(twi) {}

	void ReadButtonStates() { _usart->read(0, 0); }

};

TEST(TestCaseName, TestName2) {

	::testing::NiceMock<UsartMock> usart;
	EXPECT_CALL(usart, read).WillOnce(Return(0));

	TwiBase twi;
	ButtonBox bb{ &usart, &twi };

	bb.ReadButtonStates();
}