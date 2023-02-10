// AUTO-GENERATED by WaratahCmd.exe

// HID Usage Tables: 1.3.0
// Descriptor size: 31 (bytes)
// +----------+-------+------------------+
// | ReportId | Kind  | ReportSizeInBits |
// +----------+-------+------------------+
// |        1 | Input |                8 |
// +----------+-------+------------------+
static const unsigned char hidReportDescriptor [] = 
{
    0x05, 0x01,    // UsagePage(Generic Desktop[1])
    0x09, 0x05,    // UsageId(Gamepad[5])
    0xA1, 0x01,    // Collection(Application)
    0x85, 0x01,    //     ReportId(1)
    0x05, 0x09,    //     UsagePage(Button[9])
    0x19, 0x01,    //     UsageIdMin(Button 1[1])
    0x29, 0x02,    //     UsageIdMax(Button 2[2])
    0x15, 0x00,    //     LogicalMinimum(0)
    0x25, 0x01,    //     LogicalMaximum(1)
    0x95, 0x02,    //     ReportCount(2)
    0x75, 0x01,    //     ReportSize(1)
    0x81, 0x02,    //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x95, 0x01,    //     ReportCount(1)
    0x75, 0x06,    //     ReportSize(6)
    0x81, 0x03,    //     Input(Constant, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0xC0,          // EndCollection()
};