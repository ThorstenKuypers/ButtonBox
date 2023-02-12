#pragma once

#include <avr/io.h>
#include <util/atomic.h>

#ifdef __AVR_ATmega8U2__
#include "hal_m8u2.h"
#endif

namespace hal
{

    class Timer1;

    class timer1CompA : interrupt
    {
        Timer1 *_owner;

    public:
        timer1CompA();
        void setOwner(Timer1 *p);
        virtual void isr(void) override;
    };

    class Timer1
    {
        friend class timer1CompA;
        volatile uint32_t _cnt;
        timer1CompA tca;

    public:
        Timer1(uint8_t tccr1b, uint16_t ocr1a, uint8_t timsk);

        uint32_t GetCount()
        {
            uint32_t c = 0;
            ATOMIC_BLOCK(ATOMIC_FORCEON)
            {
                c = _cnt;
            }

            return c;
        }

        void Reset()
        {
            ATOMIC_BLOCK(ATOMIC_FORCEON)
            {
                _cnt = 0;
            }
        }
    };
}