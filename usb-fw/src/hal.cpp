
#include "../inc/hal.h"

using namespace hal;

        timer1CompA::timer1CompA() : _owner(nullptr)
        {
            Register(0, this);
        }

        void timer1CompA::setOwner(Timer1 *p)
        {
            _owner = p;
        }

        void timer1CompA::isr(void)
        {
            _owner->_cnt++;
        }

        Timer1::Timer1(uint8_t tccr1b, uint16_t ocr1a, uint8_t timsk) : _cnt(0)
        {
            tca.setOwner(this);
            // init timer
            TCCR1B = tccr1b;
            OCR1A = ocr1a;
            TIMSK1 = timsk;
        }
