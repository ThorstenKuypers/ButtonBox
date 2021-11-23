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

#include "../inc/Usart.h"
#include "../inc/commondefs.h"

int main(void) {
  Usart usart{};
  usart.Init();
  uint8_t b = 0;
  uint8_t buf[] = {'A', 'B', 'C'};

  sei();

  while (1) {
    usart.Write(buf, 3);
  }
}
