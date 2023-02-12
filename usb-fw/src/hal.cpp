
#include "../inc/hal.h"
#include "../inc/timer.h"

using namespace hal;

timer1CompA::timer1CompA() : _owner(nullptr)
{
    Register(0, this);
}

void timer1CompA::setOwner(timer::Timer1 *p)
{
    _owner = p;
}

void timer1CompA::isr(void)
{
    _owner->_cnt++;
}
