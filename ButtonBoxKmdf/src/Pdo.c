/**

	file: Pdo.c

	Implementation details of Bus enumerated device -> raw PDO
*/

#include "Driver.h"
#include "Pdo.h"
//#include "pdo.tmh"

#include "utils.h"


VOID
ButtonBoxHidInternalDeviceControl(
	_In_ WDFQUEUE Queue,
	_In_ WDFREQUEST Request,
	_In_ size_t OutputBufferLength,
	_In_ size_t InputBufferLength,
	_In_ ULONG IoControlCode
	)
	/*++

	Routine Description:

	This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

	Arguments:

	Queue -  Handle to the framework queue object that is associated with the
	I/O request.

	Request - Handle to a framework request object.

	OutputBufferLength - Size of the output buffer in bytes

	InputBufferLength - Size of the input buffer in bytes

	IoControlCode - I/O control code.

	Return Value:

	VOID

	--*/
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT devCtx;
	WDFDEVICE device = WdfIoQueueGetDevice(Queue);
	size_t len = 0;
	PUCHAR buf = NULL;

	//TraceEvents(TRACE_LEVEL_INFORMATION,
	//	TRACE_QUEUE,
	//	"!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d",
	//	Queue, Request, (int)OutputBufferLength, (int)InputBufferLength, IoControlCode);

	UNREFERENCED_PARAMETER(InputBufferLength);

	devCtx = DeviceGetContext(device);
	if (devCtx == NULL) {

		KdBreakPoint();
		status = STATUS_INVALID_PARAMETER;
		WdfRequestComplete(Request, status);

		return;
	}

	DbgPrint("ButtonBoxEvtIoDeviceControl called!\n");
	//KdBreakPoint();

	switch (IoControlCode)
	{
	case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
		DbgPrint("IOCTL_HID_GET_DEVICE_DESCRIPTOR called!");

		// HID class driver asks for HID descriptor
		status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, (PVOID)&buf, &len);
		if (!NT_SUCCESS(status)) {

			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! WdfRequestRetrieveOutputMemory failed! %!STATUS!", status);
			WdfRequestComplete(Request, status);

			return;
		}

		if (len < OutputBufferLength) {

			status = STATUS_BUFFER_TOO_SMALL;
			WdfRequestComplete(Request, status);

			return;
		}

		RtlCopyMemory(buf, &devCtx->hidDescriptor, devCtx->hidDescriptor.bLength);

		WdfRequestCompleteWithInformation(Request, status, devCtx->hidDescriptor.bLength);

		return;

	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
		DbgPrint("IOCTL_HID_GET_DEVICE_ATTRIBUTES called!");

		if (devCtx != NULL) {

			status = CopyToOutputBuffer(Request, &devCtx->deviceAttributes, sizeof(HID_DEVICE_ATTRIBUTES));
			if (!NT_SUCCESS(status)) {

				break;
			}
		}

		WdfRequestCompleteWithInformation(Request, status, sizeof(HID_DEVICE_ATTRIBUTES));

		return;

	case IOCTL_HID_GET_REPORT_DESCRIPTOR:
		DbgPrint("IOCTL_HID_GET_REPORT_DESCRIPTOR called!");

		status = CopyToOutputBuffer(Request, devCtx->reportDescriptor, OutputBufferLength);

		WdfRequestCompleteWithInformation(Request, status, OutputBufferLength);

		return;

	case IOCTL_HID_READ_REPORT:
		DbgPrint("IOCTL_HID_READ_REPORT called!");

		status = WdfRequestForwardToIoQueue(Request, devCtx->readReqQueue);
		if (!NT_SUCCESS(status)) {

			KdBreakPoint();
			return;
		}
		
		return;



	case IOCTL_GET_PHYSICAL_DESCRIPTOR:
		DbgPrint("IOCTL_GET_PHYSICAL_DESCRIPTOR called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_ACTIVATE_DEVICE:
		DbgPrint("IOCTL_HID_ACTIVATE_DEVICE called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_DEACTIVATE_DEVICE:
		DbgPrint("IOCTL_HID_DEACTIVATE_DEVICE called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_GET_FEATURE:
		DbgPrint("IOCTL_HID_GET_FEATURE called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_GET_INDEXED_STRING:
		DbgPrint("IOCTL_HID_GET_INDEXED_STRING called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_GET_INPUT_REPORT:
		DbgPrint("IOCTL_HID_GET_INPUT_REPORT called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_GET_STRING:
		DbgPrint("IOCTL_HID_GET_STRING called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:
		DbgPrint("IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_SET_FEATURE:
		DbgPrint("IOCTL_HID_SET_FEATURE called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_SET_OUTPUT_REPORT:
		DbgPrint("IOCTL_HID_SET_OUTPUT_REPORT called!");
		KdBreakPoint();

		break;

	case IOCTL_HID_WRITE_REPORT:
		DbgPrint("IOCTL_HID_WRITE_REPORT called!");
		KdBreakPoint();

		break;

	default:
		KdBreakPoint();

		break;
	}
	WdfRequestComplete(Request, status);

	return;
}
