#ifndef _SERIAL_H_
#define _SERIAL_H_

EXTERN_C_START

#include <ntddser.h>


// SERIAL PROTOCOL HEADER LAYOUT
///////////////////////////
/*******************************************************************/
/////////////////////////////////////////////
// LAYOUT
// 7 | 6 | 5 | 4 | 3:0
// F | C | S | R | Len (15 byte max)
// F - Fragmentation: continuation data from last packet (was more than 15 bytes)
// C - Configuration: configuration data for uC
// S - Send: upload data to PC from uC
// R - Receive: download data from PC to uC
// Len - length of payload (data) of packet

#define BITS_FRAG		7
#define BITS_CONFIG		6
#define BITS_SEND		4
#define BITS_RECV		5

#define FLAGS_FRAG		(1<<BITS_FRAG)
#define FLAGS_CONFIG	(1<<BITS_CONFIG)
#define FLAGS_SEND		(1<<BITS_SEND)
#define FLAGS_RECV		(1<<BITS_RECV)

#define MASK_LEN		0xF
#define HDR_LEN(b)		(b & MASK_LEN)

#define CHECK_FRAG(b)		(((b & FLAGS_FRAG) == FLAGS_FRAG) ? 1 : 0)
#define CHECK_CONFIG(b)		(((b & FLAGS_CONFIG) == FLAGS_CONFIG) ? 1 : 0)
#define CHECK_SEND(b)		(((b & FLAGS_SEND) == FLAGS_SEND) ? 1 : 0)
#define CHECK_RECV(b)		(((b & FLAGS_RECV) == FLAGS_RECV) ? 1 : 0)

#define SET_FLAG_FRAG(b)	(b |= FLAGS_FRAG)
#define SET_FLAG_CONFIG(b)	(b |= FLAGS_CONFIG)
#define SET_FLAG_SEND(b)	(b |= FLAGS_SEND)
#define SET_FLAG_RECV(b)	(b |= FLAGS_RECV)



NTSTATUS SerialInitPort(WDFDEVICE Device);
int SerialCheckBytesAvailable(WDFCONTEXT ctx);
int SerialRead(WDFCONTEXT ctx, PUCHAR buf, size_t buflen);

EVT_WDF_REQUEST_COMPLETION_ROUTINE SerialReadComplete;


EXTERN_C_END

#endif // _SERIAL_H_
