
#include <gtest/gtest.h>

#include "RingBuffer.h"

TEST(RingBuffer, Available_Returns0_WhenEmpty)
{
	RingBuffer<16> rb;

	auto r = rb.Available();

	EXPECT_EQ(r, 0);
}

TEST(RingBuffer, Available_ReturnsBytesInBufferToRead_WhenBuuferIsNotEmpty)
{
	RingBuffer<16> rb;
	rb.PutByte(0x12);

	auto r = rb.Available();

	EXPECT_EQ(r, 1);
}

