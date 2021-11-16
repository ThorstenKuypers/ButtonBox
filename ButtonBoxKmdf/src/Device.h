/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include "public.h"

EXTERN_C_START

#define DEVICE_ID L"tk\\ButtonBoxKmdf\0\0"
#define DEVICE_ID_LEN sizeof(DEVICE_ID)

#define VID 0x7136
#define PID 0xDEAF
#define ProdVer 0x0100

#define HID_INPUT_REPORT_LEN 8

#define RX_BUF_LEN	0xFF	// receive ring-buffer length


// description of the ring-buffer
typedef struct _RX_RING_BUFFER
{
	PUCHAR buf;
	USHORT writeTo;
	USHORT readFrom;
}RX_RING_BUFFER, *PRX_RING_BUFFER;

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
	WDFDEVICE self;
	HID_DEVICE_ATTRIBUTES deviceAttributes; // HID_DEVICE_ATTRIBUTES
	HID_DESCRIPTOR hidDescriptor; // HID_DESCRIPTOR
	PCHAR reportDescriptor; // REPORT_DESCRIPTOR
	UCHAR hidInputReport[HID_INPUT_REPORT_LEN]; // HID input report

	WDFQUEUE readReqQueue;
	WDFTIMER readTimer;

	PWCHAR DeviceId; // device ID string

	WDFIOTARGET serialIoTarget;	// I/O target to the serial port
	WDFREQUEST serialReadRequest; // request object for read
	WDFMEMORY requestMemory; // memory object for read request

	WDFMEMORY rxRingBufMem;	// RX ring buffer memory object
	RX_RING_BUFFER rxBuf;	// pointer to ring buffer in memory object

	KEVENT RequestWaitEvent; // Notification event for request ompletion

	//HANDLE ComPort;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

EVT_WDF_DRIVER_DEVICE_ADD ButtonBoxEvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL ButtonBoxBusEvtIoDeviceControl;
EVT_WDFDEVICE_WDM_IRP_PREPROCESS ButtonBoxEvtHandleQueryIrp;

//
// Function to initialize the device and its callbacks
//
NTSTATUS
ButtonBoxCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

NTSTATUS ButtonBoxEvtDevicePrepareHardware(
	_In_ WDFDEVICE    Device,
	_In_ WDFCMRESLIST ResourcesRaw,
	_In_ WDFCMRESLIST ResourcesTranslated
	);

NTSTATUS ButtonBoxEvtDeviceReleaseHardware(
	_In_ WDFDEVICE    Device,
	_In_ WDFCMRESLIST ResourcesTranslated
	);

NTSTATUS ButtonBoxEvtDeviceD0Entry(
	_In_ WDFDEVICE              Device,
	_In_ WDF_POWER_DEVICE_STATE PreviousState
	);

NTSTATUS ButtonBoxEvtDeviceD0Exit(
	_In_ WDFDEVICE              Device,
	_In_ WDF_POWER_DEVICE_STATE TargetState
	);

EXTERN_C_END
