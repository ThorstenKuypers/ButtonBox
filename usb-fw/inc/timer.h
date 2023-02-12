#pragma once

#include "hal.h"

namespace timer
{
   
    class Timer1
    {
        friend class hal::timer1CompA;
        volatile uint32_t _cnt;
        hal::timer1CompA tca;

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