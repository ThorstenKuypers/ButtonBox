
#include "../inc/hal.h"
#include "../inc/timer.h"
#include "../inc/Usart.h"

using namespace hal;

timer1CompA::timer1CompA() : _owner(nullptr)
{
    Register(TIMER1_COMPA_VEC, this);
}

void timer1CompA::setOwner(timer::Timer1 *p)
{
    _owner = p;
}

void timer1CompA::isr(void)
{
    _owner->_cnt++;
}

usartRx::usartRx() : _owner(nullptr)
{
    Register(USART1_RX_VEC, this);
}

void usartRx::setOwner(usart::Usart *p)
{
    _owner = p;
}

void usartRx::isr()
{
    // handle interrupt
    uint8_t b = UDR1;
    _owner->_rxBuf.PutByte(b);
}