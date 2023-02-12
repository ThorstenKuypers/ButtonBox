
#include "../inc/hal_m8u2.h"

using namespace hal;

interrupt* interrupt::isrVectorTable[] = {0};

void interrupt::Register(int num, interrupt* intPtr)
{
    isrVectorTable[num] = intPtr;
}

void interrupt::timer1CompA()
{
    if (isrVectorTable[0] != nullptr)
        isrVectorTable[0]->isr();
}
