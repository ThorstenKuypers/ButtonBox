// SerialProto.h - serial protocol definitions
#ifndef _SERIAL_PROTO_H_
#define _SERIAL_PROTO_H_

// COMMAND BYTES
/////////////////

// STATUS CODES
/////////////////


// HARDWARE handshake lines definitions
/////////////////////////////////////////////////////////////////////
// This line (CTS) is handled by the INT0 ISR
// falling edge means PC ready (sync: OK)
// rising edge means hold data transmission
#define DTR PD2 // PC ready to receive data (sync OK)

// acknowledge line
#define CTS PORTD4 //PD4


// GENERAL PROTOCOL HEADER
///////////////////////////
/*******************************************************************/
/////////////////////////////////////////////
// LAYOUT
// 7 | 6 | 5 | 4 | 3:0
// F | C | S | R | Len (15 byte max)
// F - Fragmentation: continuation data from last packet (was more than 15 bytes)
// C - Configuration: configuration data for uC
// S - Send: upload data to PC
// R - Receive: download data from PC to uC
// Len - length of payload (data) of packet
#define PROTO_FRAG		0x80	// fragmentation flag
#define PROTO_CONFIG	0x40	// config flag
#define PROTO_SEND		0x20	// send data to PC flag
#define PROTO_RECV		0x10	// receive data from PC flag
#define PROTO_LEN_MASK	0x0F	// bitmask to get pyload length

uint8_t proto_cc;	// serial protocol command&control byte

//typedef struct _proto_header
//{
//  uint8_t cmd;
//  uint8_t len;
//}PROTO_HEADER;
//
//#define PROTO_HEADER_LEN 2
#define MAX_PAYLOAD_LEN 15 // max 15 bytes of payload
///////////////////////////////////////////////////////////////////////

#define RPM_UPDATE		0x1
#define FUEL_UPDATE		0x2
#define GEAR_UPDATE		0x4
#define PIT_LIMITER		0x8
#define REV_LIMIT_WARN	0x10
#define YELLOW_FLAG		0x20
#define BLUE_FLAG		0x40
#define LOW_FUEL_WARN	0x80


#define REVLIMIT_LEDS_OFF 	0x7F
#define REVLIMIT_LEDS_ON	~REVLIMIT_LEDS_OFF

// Fuel warning LEDS connected to Pins 1 & 2 of Port Expander
#define FUEL_WARN_OFF	0x3
#define FUEL_WARN_ON	0

// CONFIGURATION DEFINITION
#define DUMMY 0x1


#define FLAG_REVLIMITER 0x80

/**** BUTTON PROTOCOL DATA LAYOUT ****/

// Byte-Bits			Data
// 1-0:7		Buttons 1-8
// 2-0:7		Buttons 9-16
// 3-0:7		Rot.Switch 1 Pos 1-8
// 4-0:3		Rot Switch 1 Pos 9-12 (Bits: 0-3)
// 4-4			RotEnc1 PB (Bit: 4)
// 4-5			RotEnc2 PB (Bit: 5)
// 4-6			Shifter Up + (Bit: 6)
// 4-7			Shifter Down - (Bit: 7)
// 5-0:5		Rot.Switch 2 Pos 1-6
// 5-6			BrakeBias + (Bit: 6)
// 5-7			BrakeBias - (Bit: 7)
// 6-0			RotEnc1 +
// 6-1			RotEnc1 -
// 6-2			RotEnc2 +
// 6-3			RotEnc2 -
/*************************************/


/***
typedef struct _proto_recv_data
{
    byte updateMask;
	byte rpmBar;
	byte fuelBar;
	byte gear;
    byte flags;
	
}PROTO_RECV, *PPROTO_RECV;
***/

/////////////////////////////////////////////////////////////////////



#endif /* _SERIAL_PROTO_H_ */
