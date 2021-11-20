/*++

Module Name:

	device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.

Environment:

	Kernel-mode Driver Framework

--*/

#include "driver.h"

#include "hid.h"
#include "utils.h"
#include "Serial.h"
#include "Descriptor.h"

HID_DESCRIPTOR hidDescriptor[] = {

	0x09,	// bLength
	0x21,	// HID descriptor
	0x0111,	// HID protocol version (HID v1.11)
	0x00,	// country code (not localized)
	0x01,	// numDescriptor
	{
		0x22,	// report descriptor
		(USHORT)sizeof(ReportDescriptor)
	}
};


NTSTATUS
ButtonBoxEvtDeviceAdd(
	_In_    WDFDRIVER       Driver,
	_Inout_ PWDFDEVICE_INIT DeviceInit
)
/*++
Routine Description:

EvtDeviceAdd is called by the framework in response to AddDevice
call from the PnP manager. We create and initialize a device object to
represent a new instance of the device.

Arguments:

Driver - Handle to a framework driver object created in DriverEntry

DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

NTSTATUS

--*/
{
	NTSTATUS status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(Driver);
	UNREFERENCED_PARAMETER(DeviceInit);

	//PAGED_CODE();

	KdBreakPoint();

	status = ButtonBoxCreateDevice(DeviceInit);

	return status;
}

NTSTATUS
ButtonBoxCreateDevice(
	_Inout_ PWDFDEVICE_INIT DeviceInit
)
/*++

Routine Description:

	Worker routine called to create a device and its software resources.

Arguments:

	DeviceInit - Pointer to an opaque init structure. Memory for this
					structure will be freed by the framework when the WdfDeviceCreate
					succeeds. So don't access the structure after that point.

Return Value:

	NTSTATUS

--*/
{
	WDF_OBJECT_ATTRIBUTES   deviceAttributes;
	PDEVICE_CONTEXT devCtx;
	WDFDEVICE device;
	NTSTATUS status;
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCb;
	WDFQUEUE queue;
	WDF_IO_QUEUE_CONFIG    queueConfig;
	WDF_TIMER_CONFIG timerConfig;
	WDF_OBJECT_ATTRIBUTES timerAttributes;
	UCHAR minorFunction = IRP_MN_QUERY_ID;

	KdPrint(("ButtonBoxCreateDevice called!\n"));

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCb);
	pnpPowerCb.EvtDevicePrepareHardware = ButtonBoxEvtDevicePrepareHardware;
	pnpPowerCb.EvtDeviceReleaseHardware = ButtonBoxEvtDeviceReleaseHardware;
	pnpPowerCb.EvtDeviceD0Entry = ButtonBoxEvtDeviceD0Entry;
	pnpPowerCb.EvtDeviceD0Exit = ButtonBoxEvtDeviceD0Exit;

	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCb);

	// set this driver as as filter driver for mshidkmdf.sys
	WdfFdoInitSetFilter(DeviceInit);

	// setup handler to process IRP_MJ_PNP/IRP_MN_QUERY_ID since the framework does
	// handle this request by itself, but the root enumerator assigns a NULL ID
	// for DeviceID and HardwareID which causes a crash in the PnP manager!
	status = WdfDeviceInitAssignWdmIrpPreprocessCallback(DeviceInit, ButtonBoxEvtHandleQueryIrp, IRP_MJ_PNP, &minorFunction, 1);
	if (!NT_SUCCESS(status))
	{

		KdBreakPoint();
		return status;
	}

	WDF_OBJECT_ATTRIBUTES_INIT(&deviceAttributes);
	WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

	status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

	if (NT_SUCCESS(status))
	{
		//
		// Get a pointer to the device context structure that we just associated
		// with the device object. We define this structure in the device.h
		// header file. DeviceGetContext is an inline function generated by
		// using the WDF_DECLARE_CONTEXT_TYPE_WITH_NAME macro in device.h.
		// This function will do the type checking and return the device context.
		// If you pass a wrong object handle it will return NULL and assert if
		// run under framework verifier mode.
		//
		devCtx = DeviceGetContext(device);

		devCtx->deviceAttributes.Size = sizeof(HID_DEVICE_ATTRIBUTES);
		devCtx->deviceAttributes.VendorID = (USHORT)VID;
		devCtx->deviceAttributes.ProductID = (USHORT)PID;
		devCtx->deviceAttributes.VersionNumber = (USHORT)ProdVer;

		RtlCopyMemory(&devCtx->hidDescriptor, hidDescriptor, sizeof(HID_DESCRIPTOR));
		devCtx->reportDescriptor = &ReportDescriptor[0];
		devCtx->self = device;

		//
		// Initialize the I/O Package and any Queues
		//
		//
		// Configure a default queue so that requests that are not
		// configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
		// other queues get dispatched here.
		//
		WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
			&queueConfig,
			WdfIoQueueDispatchParallel
		);

		queueConfig.PowerManaged = WdfUseDefault;
		queueConfig.EvtIoInternalDeviceControl = ButtonBoxHidInternalDeviceControl;

		status = WdfIoQueueCreate(
			device,
			&queueConfig,
			WDF_NO_OBJECT_ATTRIBUTES,
			&queue
		);

		if (!NT_SUCCESS(status))
		{
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
			return status;
		}

		// create a queue to queue read requests 
		WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);
		queueConfig.PowerManaged = WdfFalse;

		status = WdfIoQueueCreate(device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, &devCtx->readReqQueue);
		if (!NT_SUCCESS(status))
		{
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed (Manual Queue) %!STATUS!", status);
			return status;
		}

		// create timer for the polling interval of the serial port interface
		WDF_TIMER_CONFIG_INIT_PERIODIC(&timerConfig, ButtonBoxEvtReadInput, 16); // 16ms ~60Hz
		//timerConfig.AutomaticSerialization = TRUE;
		//WDF_TIMER_CONFIG_INIT(&timerConfig, ButtonBoxEvtReadInput); // 8ms

		WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
		//timerAttributes.ExecutionLevel = WdfExecutionLevelPassive;
		//timerAttributes.SynchronizationScope = WdfSynchronizationScopeDevice;
		// set the device as parent object for the timer object so
		// we can get access to the device and its context data
		// in the timer routine
		timerAttributes.ParentObject = device;

		status = WdfTimerCreate(&timerConfig, &timerAttributes, &devCtx->readTimer);
		if (!NT_SUCCESS(status))
		{

			KdBreakPoint();
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfTimerCreate failed %!STATUS!", status);
			return status;
		}

		KeInitializeEvent(&devCtx->RequestWaitEvent, NotificationEvent, FALSE);
		KeResetEvent(&devCtx->RequestWaitEvent);
	}

	return status;
}

NTSTATUS ButtonBoxEvtHandleQueryIrp(
	_In_    WDFDEVICE Device,
	_Inout_ PIRP      Irp
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IrpSp;
	size_t len = 0;
	DECLARE_CONST_UNICODE_STRING(DeviceId, DEVICE_ID);
	PDEVICE_CONTEXT devCtx = DeviceGetContext(Device);

	// get current IRP stack location
	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	if (IrpSp != NULL)
	{

		if (IrpSp->MajorFunction == IRP_MJ_PNP &&
			IrpSp->MinorFunction == IRP_MN_QUERY_ID)
		{

			switch (IrpSp->Parameters.QueryId.IdType)
			{

				case BusQueryDeviceID:
				case BusQueryHardwareIDs:

					if (devCtx != NULL)
					{

						len = DEVICE_ID_LEN;
						devCtx->DeviceId = (PWCHAR)ExAllocatePoolWithTag(NonPagedPool, len, 'beef');
						//					devCtx->DeviceId = (PWCHAR)ExAllocatePool2(POOL_FLAG_NON_PAGED|POOL_FLAG_NON_PAGED_EXECUTE, len, 'beef');
						if (devCtx->DeviceId != NULL)
						{

							RtlZeroMemory(devCtx->DeviceId, len);
							RtlCopyMemory(devCtx->DeviceId, DeviceId.Buffer, DeviceId.Length);

							Irp->IoStatus.Information = (ULONG_PTR)devCtx->DeviceId;
							status = Irp->IoStatus.Status = STATUS_SUCCESS;
						}
					}
					break;

				default:
					status = Irp->IoStatus.Status;
					Irp->IoStatus.Information = (ULONG_PTR)NULL;
					break;
			}

		}
	}

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

// A driver's EvtIoDeviceControl event callback function processes a specified device I/O control request.
// for the ButonBox bus device
VOID ButtonBoxBusEvtIoDeviceControl(
	_In_ WDFQUEUE   Queue,
	_In_ WDFREQUEST Request,
	_In_ size_t     OutputBufferLength,
	_In_ size_t     InputBufferLength,
	_In_ ULONG      IoControlCode
)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);
	UNREFERENCED_PARAMETER(IoControlCode);

	switch (IoControlCode)
	{

		default:
			break;
	}

	WdfRequestComplete(Request, STATUS_SUCCESS);
}

/*++
A driver's EvtDevicePrepareHardware event callback function performs any
operations that are needed to make a device accessible to the driver.

Typically, your driver's EvtDevicePrepareHardware callback function does the following, if necessary:

* Maps physical memory addresses to virtual addresses so the driver can access memory that is assigned to the device
* Determines the device's revision number
* Configures USB devices
* Obtains driver-defined interfaces from other drivers

--*/
NTSTATUS ButtonBoxEvtDevicePrepareHardware(
	_In_ WDFDEVICE    Device,
	_In_ WDFCMRESLIST ResourcesRaw,
	_In_ WDFCMRESLIST ResourcesTranslated
)
{
	UNREFERENCED_PARAMETER(ResourcesRaw);
	UNREFERENCED_PARAMETER(ResourcesTranslated);

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT devCtx;
	WDF_OBJECT_ATTRIBUTES attrib;
	WDFIOTARGET ioTarget;
	WDF_IO_TARGET_OPEN_PARAMS targetParams;
	DECLARE_CONST_UNICODE_STRING(deviceName, L"\\??\\COM4");

	KdPrint(("ButtonBoxEvtDevicePrepareHardware called!\n"));

	devCtx = DeviceGetContext(Device);
	if (devCtx != NULL)
	{

		// open an I/O target to the serial port driver to establish
		// the communication channel with the ButtonBox
		// ...

		WDF_OBJECT_ATTRIBUTES_INIT(&attrib);
		attrib.ParentObject = Device; // set the device as parent

		status = WdfIoTargetCreate(Device, &attrib, &ioTarget);
		if (!NT_SUCCESS(status))
		{

			KdBreakPoint();
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfIoTargetCreate failed! %!STATUS!", status);
			return status;
		}

		WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(&targetParams, &deviceName, GENERIC_READ | GENERIC_WRITE);
		targetParams.ShareAccess = 0;
		targetParams.CreateDisposition = FILE_OPEN;
		targetParams.FileAttributes = FILE_ATTRIBUTE_NORMAL;

		status = WdfIoTargetOpen(ioTarget, &targetParams);
		if (!NT_SUCCESS(status))
		{

			KdBreakPoint();
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfIoTargetOpen failed! %!STATUS!", status);
			return status;
		}

		devCtx->serialIoTarget = ioTarget;

		status = SerialInitPort(Device);
		if (!NT_SUCCESS(status))
		{

			KdBreakPoint();

			return status;
		}

	}

	return status;
}


/*++
A driver's EvtDeviceReleaseHardware event callback function performs operations
that are needed when a device is no longer accessible.

If a driver has registered an EvtDeviceReleaseHardware callback function, the
framework calls it during the following transitions:

* Resource rebalancing
* Orderly removal
* Surprise removal

--*/
NTSTATUS ButtonBoxEvtDeviceReleaseHardware(
	_In_ WDFDEVICE    Device,
	_In_ WDFCMRESLIST ResourcesTranslated
)
{
	UNREFERENCED_PARAMETER(ResourcesTranslated);

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT devCtx;

	KdPrint(("ButtonBoxEvtDeviceReleaseHardware called!\n"));

	devCtx = DeviceGetContext(Device);
	if (devCtx != NULL)
	{
		// close channel to serial port driver
		// ...

		// stop timer
		WdfTimerStop(devCtx->readTimer, TRUE);
	}

	return status;
}


/*++

A driver's EvtDeviceD0Entry event callback function performs operations that
are needed when the driver's device enters the D0 power state.

--*/
NTSTATUS ButtonBoxEvtDeviceD0Entry(
	_In_ WDFDEVICE              Device,
	_In_ WDF_POWER_DEVICE_STATE PreviousState
)
{
	PDEVICE_CONTEXT devCtx = DeviceGetContext(Device);
	NTSTATUS status = STATUS_SUCCESS;
	WDF_OBJECT_ATTRIBUTES oa = { 0 };

	UNREFERENCED_PARAMETER(PreviousState);

	DbgPrint("ButtonBoxEvtDeviceD0Entry called!\n");

	if (devCtx != NULL)
	{

		// since the device is now prepared and now also
		// powered, we can start the actual data reception
		// from the device.

		// first start an asynchronous read request. This read request
		// will be completed by the underlying driver (Serial.sys) when
		// data are available to be received. We will setup and endless 
		// read request loop. This means that we will reuse the request
		// in the completion routine and send it back to the driver
		// every time the completion routine is called.
		// The received data is buffered in a ring-buffer in our driver.

		// pre-allocate a request object
		WDF_OBJECT_ATTRIBUTES_INIT(&oa);
		oa.ParentObject = devCtx->serialIoTarget; // delete request object when IoTarget object is deleted

		status = WdfRequestCreate(&oa, devCtx->serialIoTarget, &devCtx->serialReadRequest);
		if (!NT_SUCCESS(status))
		{

			ASSERT(NT_SUCCESS(status));
			return status;
		}

		// create a memory object for the read request
		WDF_OBJECT_ATTRIBUTES_INIT(&oa);
		oa.ParentObject = devCtx->serialReadRequest;
		status = WdfMemoryCreate(
			&oa,
			NonPagedPool,
			'BoBu',
			16,	// we CANNOT use the full buffer length, b/c the request is completed only when this number of bytes available
			&devCtx->requestMemory,
			NULL);
		if (!NT_SUCCESS(status))
		{

			ASSERT(NT_SUCCESS(status));
			WdfObjectDelete(devCtx->serialReadRequest);

			return status;
		}

		// and a second memory object that describes the RX ring buffer
		status = WdfMemoryCreate(&oa, NonPagedPool, 'BoBu', RX_BUF_LEN, &devCtx->rxRingBufMem, &devCtx->rxBuf.buf);
		if (!NT_SUCCESS(status))
		{

			ASSERT(NT_SUCCESS(status));
			WdfObjectDelete(devCtx->serialReadRequest);
			WdfObjectDelete(devCtx->requestMemory);
			return status;
		}

		// format the request object for asynchronous read
		status = WdfIoTargetFormatRequestForRead(devCtx->serialIoTarget,
												 devCtx->serialReadRequest,
												 devCtx->requestMemory,
												 NULL, // first read from device so no offset for rx buffer
												 NULL);
		if (!NT_SUCCESS(status))
		{

			ASSERT(NT_SUCCESS(status));
			WdfObjectDelete(devCtx->requestMemory);
			WdfObjectDelete(devCtx->rxRingBufMem);
			WdfObjectDelete(devCtx->serialReadRequest);

			return status;
		}

		// set completion routine
		WdfRequestSetCompletionRoutine(devCtx->serialReadRequest, SerialReadComplete, devCtx);

		// send the initial request
		if (!WdfRequestSend(devCtx->serialReadRequest, devCtx->serialIoTarget, NULL))
		{
			status = WdfRequestGetStatus(devCtx->serialReadRequest);
			if (!NT_SUCCESS(status))
			{

				ASSERT(NT_SUCCESS(status));
				WdfObjectDelete(devCtx->requestMemory);
				WdfObjectDelete(devCtx->rxRingBufMem);
				WdfObjectDelete(devCtx->serialReadRequest);

				return status;
			}
		}

		// start the polling timer for read requests
		WdfTimerStart(devCtx->readTimer, WDF_REL_TIMEOUT_IN_MS(16));
		return status;
	}

	return STATUS_INVALID_PARAMETER;
}


/*++

A driver's EvtDeviceD0Exit event callback function performs operations that are
needed when the driver's device leaves the D0 power state.

--*/
NTSTATUS ButtonBoxEvtDeviceD0Exit(
	_In_ WDFDEVICE              Device,
	_In_ WDF_POWER_DEVICE_STATE TargetState
)
{
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(TargetState);

	NTSTATUS status = STATUS_SUCCESS;
	DbgPrint("ButtonBoxEvtDeviceD0Exit called!\n");

	return status;
}

