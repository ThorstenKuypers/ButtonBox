#pragma once

#include <avr/io.h>
#include <util/atomic.h>

#ifdef __AVR_ATmega8U2__
#include "hal_m8u2.h"
#endif

namespace timer
{
    class Timer1;
}

namespace hal
{

    class timer1CompA : interrupt
    {
        timer::Timer1 *_owner;

    public:
        timer1CompA();
        void setOwner(timer::Timer1 *p);
        virtual void isr(void) override;
    };

}