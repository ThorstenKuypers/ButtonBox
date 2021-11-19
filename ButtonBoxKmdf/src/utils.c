
#include "Driver.h"
#include "utils.h"

// common utillity and helper functions


NTSTATUS UtilSendIoctlSync(WDFIOTARGET ioTarget,
						   ULONG ioctl,
						   BOOLEAN Internal,
						   PVOID inputBuffer,
						   ULONG inputBufferLength,
						   PVOID outputBuffer,
						   ULONG outputBufferLength,
						   PULONG bytesReturned)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDF_MEMORY_DESCRIPTOR inputMemory;
	WDF_MEMORY_DESCRIPTOR outputMemory;
	ULONG ret = 0;
	WDF_REQUEST_SEND_OPTIONS opt;
	//	WDFREQUEST Request;


	if (inputBuffer != NULL)
		WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&inputMemory, (PVOID)inputBuffer, inputBufferLength);

	if (outputBuffer != NULL)
		WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&outputMemory, (PVOID)outputBuffer, outputBufferLength);

	WDF_REQUEST_SEND_OPTIONS_INIT(&opt, WDF_REQUEST_SEND_OPTION_TIMEOUT | WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&opt, WDF_REL_TIMEOUT_IN_SEC(1));

	//status = WdfRequestCreate(WDF_NO_OBJECT_ATTRIBUTES, ioTarget, &Request);

	if (Internal)
	{

		status = WdfIoTargetSendInternalIoctlSynchronously(
			ioTarget,
			WDF_NO_HANDLE,
			ioctl,
			&inputMemory,
			&outputMemory,
			&opt,
			(PULONG_PTR)&ret);
	}
	else
	{
		status = WdfIoTargetSendIoctlSynchronously(
			ioTarget,
			WDF_NO_HANDLE,
			ioctl,
			(inputBuffer != NULL) ? (&inputMemory) : (NULL),
			(outputBuffer != NULL) ? (&outputMemory) : (NULL),
			&opt,
			(PULONG_PTR)&ret);
	}
	if (!NT_SUCCESS(status))
	{

		KdBreakPoint();

		return status;
	}

	if (bytesReturned != NULL)
		*bytesReturned = ret;

	return status;
}


NTSTATUS CopyToOutputBuffer(WDFREQUEST Request, PVOID buf, size_t bufLen)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDFMEMORY wdfMem;
	size_t len = 0;

	status = WdfRequestRetrieveOutputMemory(Request, &wdfMem);
	if (!NT_SUCCESS(status))
	{

		KdPrint(("WdfRequestRetrieveOutputMemory failed with status: 0x%08X!\n", status));
		return status;
	}

	PVOID b = WdfMemoryGetBuffer(wdfMem, &len);
	if (b == NULL)
	{

		KdBreakPoint();
	}

	if (len < bufLen)
	{

		KdBreakPoint();
		KdPrint(("Insufficient memory! len: 0x%08X -- bufLen: 0x%08X\n", len, bufLen));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = WdfMemoryCopyFromBuffer(wdfMem, 0, buf, bufLen);
	if (!NT_SUCCESS(status))
	{

		KdPrint(("WdfMemoryCopyFromBuffer failed with status: 0x%08X\n", status));
		return status;
	}

	WdfRequestSetInformation(Request, bufLen);

	return status;
}