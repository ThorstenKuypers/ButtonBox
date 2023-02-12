
#include "../inc/hal_m8u2.h"

using namespace hal;

interrupt* interrupt::isrVectorTable[] = {0};

void interrupt::Register(int num, interrupt* intPtr)
{
    isrVectorTable[num] = intPtr;
}

void interrupt::timer1CompA()
{
    if (isrVectorTable[TIMER1_COMPA_VEC] != nullptr)
        isrVectorTable[TIMER1_COMPA_VEC]->isr();
}

void interrupt::usartRx()
{
    if (isrVectorTable[USART1_RX_VEC] != nullptr)
        isrVectorTable[USART1_RX_VEC]->isr();
}