/*
 * ButtonBoxFw.cpp
 *
 * Created: 4/7/2021 11:41:30 PM
 * Author : iceri
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <stdlib.h>

#include "../include/Usart.h"
#include "../include/commondefs.h"
#include "../include/hal.h"
#include "../include/twi.h"

int main(void)
{
  Usart<usart0_traits> usart{};
  usart.Init();
  uint8_t buf[] = {'A', 'B', 'C', '\n'};

  //auto& twi = TwoWire<twi_traits>::Instance();

  sei();

  uint8_t b = 0;

  while (1)
  {
    usart.Write(buf, 4);

    //twi.ReadFromDevice(0x20, &b, 1);
    //usart.Write(&b, 1);
  }

  return 0;
}
