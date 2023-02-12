
#include "../inc/timer.h"


timer::Timer1::Timer1(uint8_t tccr1b, uint16_t ocr1a, uint8_t timsk) : _cnt(0)
{
    tca.setOwner(this);
    // init timer
    TCCR1B = tccr1b;
    OCR1A = ocr1a;
    TIMSK1 = timsk;
}
