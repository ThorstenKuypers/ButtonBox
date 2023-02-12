#pragma once

#include <avr/io.h>


namespace hal
{
    const uint8_t TIMER1_COMPA_VEC = 0;
    const uint8_t USART1_RX_VEC = 1;
    
    class interrupt
    {
        static interrupt *isrVectorTable[2];
        static void timer1CompA() __asm__("__vector_15") __attribute__((__signal__, __used__, __externally_visible__));
        static void usartRx() __asm__("__vector_23") __attribute__((__signal__, __used__, __externally_visible__));

    public:
        static void Register(int num, interrupt *intPtr);

        virtual void isr(void) = 0;
    };

}