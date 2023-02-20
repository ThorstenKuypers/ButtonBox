#include <Arduino.h>

volatile uint8_t btn = 0;

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(38400);
  while (!Serial)
    ;
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(12, INPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:

  btn = digitalRead(12);
  Serial.write(btn);
  
  if (btn)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
    digitalWrite(LED_BUILTIN, LOW);
}