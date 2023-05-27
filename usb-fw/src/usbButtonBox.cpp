
#include "../inc/usbdevice.h"
#include "../inc/usbButtonBox.h"

usart::Usart usart1;

volatile uint8_t buttons[6];
volatile uint8_t btnUpdate = 0;

void UpdateButtons()
{
    uint8_t protoHeaderByte = 0;

    if (usart1.Available() > 0)
    {
        protoHeaderByte = usart1.GetByte();
        uint8_t dataLen = protoHeaderByte & 0x0F; // get length of data
        if (usart1.Available() <= sizeof(buttons) && dataLen <= sizeof(buttons))
        {
            for (uint8_t i = 0; i < dataLen; i++)
            {
                buttons[i] = usart1.GetByte();
            }
        }
    }

    sendHidReport();
}

void sendHidReport()
{
    if (getUsbState() == 2)
    {
        // send back HID report
        if (btnUpdate)
        {
            UENUM = 1;
            UEDATX = 0x01; // report ID
            UEDATX = buttons[0];
            UEINTX &= ~(_BV(TXINI) | _BV(FIFOCON));
            WaitTx();
            btnUpdate = 0;
            PORTD ^= _BV(PD4);
        }
    }
}