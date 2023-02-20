
#include <Arduino.h>
#include <HardwareSerial.h>

#define LED 13

int main(void)
{
    uint8_t ms = 0;
    
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    pinMode(12, INPUT);

    Serial.begin(38400);

    while(true)
    {
        if ((millis() - ms) >= 5)
        {
            uint8_t b = digitalRead(12);
            if (b)
                digitalWrite(LED, HIGH);
            else
                digitalWrite(LED,LOW);

            Serial.write(&b, 1);
        }
    }
}