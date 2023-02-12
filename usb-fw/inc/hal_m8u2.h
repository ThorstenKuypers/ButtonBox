#pragma once

#include <avr/io.h>

namespace hal
{

    class interrupt
    {
        static interrupt *isrVectorTable[1];
        static void timer1CompA() __asm__("__vector_15") __attribute__((__signal__, __used__, __externally_visible__));

    public:
        static void Register(int num, interrupt *intPtr);

        virtual void isr(void) = 0;
    };

}