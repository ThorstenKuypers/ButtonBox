/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_ButtonBox,
    0xc2c291c7,0xa873,0x467e,0xb4,0xcd,0x24,0x68,0x83,0x3a,0x8b,0xa8);
// {c2c291c7-a873-467e-b4cd-2468833a8ba8}

// {84B514DD-ABFA-4040-B4A4-4FC7E61B2972}
DEFINE_GUID(GUID_DEVICE_ENUM ,
	0x84b514dd, 0xabfa, 0x4040, 0xb4, 0xa4, 0x4f, 0xc7, 0xe6, 0x1b, 0x29, 0x72);
