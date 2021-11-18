/*++ This module implements all functions specific to communicate with
	 the serial port driver to which the physical device ButtonBox is
	 connected

** WDK Remote I7O Target
*/

#include "Driver.h"
#include "utils.h"
#include "Serial.h"
//#include "Serial.tmh"


/*
++ This function sends a synchronous IOCTL code to the serial driver
*/


/*
++ This function initializes and configures the serial port.
*/
NTSTATUS SerialInitPort(WDFDEVICE Device)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT devCtx = NULL;
	SERIAL_BAUD_RATE baudCfg = { 0 };
	SERIAL_LINE_CONTROL lineControl = { 0 };
	SERIAL_HANDFLOW handshake = { 0 };
	SERIAL_COMMPROP serialProps = { 0 };
	//SERIAL_BASIC_SETTINGS defaultSet = { 0 };

	devCtx = DeviceGetContext(Device);
	if (devCtx != NULL) {

		// Clear DTR
		status = UtilSendIoctlSync(devCtx->serialIoTarget, IOCTL_SERIAL_CLR_DTR, FALSE, NULL, 0, NULL, 0, NULL);
		if (!NT_SUCCESS(status)) {

			KdBreakPoint();

			return status;
		}

		status = UtilSendIoctlSync(devCtx->serialIoTarget, IOCTL_SERIAL_GET_PROPERTIES, FALSE, NULL, 0, &serialProps, sizeof(SERIAL_COMMPROP), NULL);
		if (!NT_SUCCESS(status)) {

			KdBreakPoint();

			return status;
		}

		// set baudrate
		baudCfg.BaudRate = 19200;
		
		status = UtilSendIoctlSync(devCtx->serialIoTarget, IOCTL_SERIAL_SET_BAUD_RATE, FALSE, &baudCfg, sizeof(SERIAL_BAUD_RATE), NULL, 0, NULL);
		if (!NT_SUCCESS(status)) {

			KdBreakPoint();

			return status;
		}

		// setup line control
		// 1 stop-bit / no parity / 8bits (8N1)
		lineControl.StopBits = STOP_BIT_1;
		lineControl.Parity = NO_PARITY;
		lineControl.WordLength = 8;

		status = UtilSendIoctlSync(devCtx->serialIoTarget, IOCTL_SERIAL_SET_LINE_CONTROL, FALSE, &lineControl, sizeof(SERIAL_LINE_CONTROL), NULL, 0, NULL);
		if (!NT_SUCCESS(status)) {

			KdBreakPoint();

			return status;
		}

		status = UtilSendIoctlSync(devCtx->serialIoTarget, IOCTL_SERIAL_SET_HANDFLOW, FALSE, &handshake, sizeof(SERIAL_HANDFLOW), NULL, 0, NULL);
		if (!NT_SUCCESS(status)) {

			KdBreakPoint();

			return status;
		}

		// set DTR line HIGH to signal ButtonBox device that PC is ready to receive data
		status = UtilSendIoctlSync(devCtx->serialIoTarget, IOCTL_SERIAL_SET_DTR, FALSE, NULL, 0, NULL, 0, NULL);
		if (!NT_SUCCESS(status)) {

			KdBreakPoint();

			return status;
		}
	}

	return status;
}


// returns number of bytes available, in the RX ring-buffer, for read
// or -1 on error
int SerialCheckBytesAvailable(WDFCONTEXT ctx)
{
	PDEVICE_CONTEXT devCtx = (PDEVICE_CONTEXT)ctx;

	if (devCtx != NULL) {

		return ((devCtx->rxBuf.writeTo - devCtx->rxBuf.readFrom) & 0xFF);
	}

	return -1;
}


int SerialRead(WDFCONTEXT ctx, PUCHAR buf, size_t buflen)
{
	PDEVICE_CONTEXT devCtx = (PDEVICE_CONTEXT)ctx;
	size_t avail = 0, len = 0;

	if (devCtx != NULL) {

		if (buf != NULL) {

			avail = SerialCheckBytesAvailable(ctx);
			if (avail >= 1) {

				if (avail < buflen)
					len = avail;
				else
					len = buflen;

				for (int i = 0; i < len; i++) {

					buf[i] = devCtx->rxBuf.buf[devCtx->rxBuf.readFrom];
					devCtx->rxBuf.readFrom++;
					devCtx->rxBuf.readFrom %= 0xFF;
				}

				return (int)len;
			}

			return (int)avail;
		}
	}

	return -1;
}



void SerialReadComplete(
	_In_ WDFREQUEST                     Request,
	_In_ WDFIOTARGET                    Target,
	_In_ PWDF_REQUEST_COMPLETION_PARAMS Params,
	_In_ WDFCONTEXT                     Context
	)
{
	PDEVICE_CONTEXT devCtx = (PDEVICE_CONTEXT)Context;
	size_t bytesReturned = 0;
	PUCHAR buf = NULL;
	WDF_REQUEST_REUSE_PARAMS reqParams;
	NTSTATUS status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(Request);
	UNREFERENCED_PARAMETER(Target);

	if (devCtx != NULL) {

		if (NT_SUCCESS(Params->IoStatus.Status)) {

			// request successful
			bytesReturned = Params->Parameters.Read.Length;

			buf = (PUCHAR)WdfMemoryGetBuffer(Params->Parameters.Read.Buffer, NULL);
			if (buf != NULL) {

				// copy the received bytes to the ring-buffer
				for (int i = 0; i < bytesReturned; i++) {

					devCtx->rxBuf.buf[devCtx->rxBuf.writeTo] = buf[i];
					devCtx->rxBuf.writeTo++;
					devCtx->rxBuf.writeTo %= 0xFF;
				}
			}
		}

		// check that the device is still in the powered state
		WDF_POWER_DEVICE_STATE ps = WdfDeviceGetDevicePowerState(WdfIoTargetGetDevice(Target));
		if (ps != WdfDevStatePowerD0NP) {
			KdBreakPoint();
			return; // if device is no longer in D0 state skip and do not resend the request
		}

		 //re-init the request to be reused
		WDF_REQUEST_REUSE_PARAMS_INIT(&reqParams, WDF_REQUEST_REUSE_NO_FLAGS, status);
		status = WdfRequestReuse(Request, &reqParams);
		if (!NT_SUCCESS(status)) {

			ASSERT(NT_SUCCESS(status));

			// TODO: handle error properly (with event e.g.)
			// ...
		}

		WdfRequestSetCompletionRoutine(Request, SerialReadComplete, devCtx);

		// re-format the request as read request
		status = WdfIoTargetFormatRequestForRead(devCtx->serialIoTarget,
			Request,
			devCtx->requestMemory,
			NULL,
			NULL);
		if (!NT_SUCCESS(status)) {

			ASSERT(NT_SUCCESS(status));

			// TODO: handle error properly (with event e.g.)
			// ...
		}

		// send request again
		WdfRequestSend(Request, devCtx->serialIoTarget, NULL);
	}
}