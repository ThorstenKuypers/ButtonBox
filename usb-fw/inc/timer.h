#pragma once

#include "hal.h"

namespace timer
{

    class MillisTimer
    {
        friend hal::timer1Interrupt;

        uint16_t _millis;

    public:
        MillisTimer(void);
        uint16_t Count() { return _millis; }
    };
}