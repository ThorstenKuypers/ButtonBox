#ifndef _UTILS_H_
#define _UTILS_H_

NTSTATUS UtilSendIoctlSync(WDFIOTARGET ioTarget,
	ULONG ioctl,
	BOOLEAN Internal,
	PVOID inputBuffer,
	ULONG inputBufferLength,
	PVOID outputBuffer,
	ULONG outputBufferLength,
	PULONG bytesReturned);

NTSTATUS CopyToOutputBuffer(WDFREQUEST Request, PVOID buf, size_t bufLen);


#endif // _UTILS_H_