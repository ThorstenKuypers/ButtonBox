#ifndef _SIMDASH_H_
#define _SIMDASH_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

/////////////////////////////////////////////////////////////////////
// uncomment to enable debugging
#define _DEBUG
//#define _ENABLE_SLEEP

//#define ENABLE_PWM (TCCR1B |= (1<<CS10)|(1<<CS11))
//					TCCR1A |= (_BV(COM1A0) | _BV(COM1A1)))

//#define DISABLE_PWM (TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12)))
//					TCCR1A &= ~(_BV(COM1A0) | _BV(COM1A1))

static inline void enable_pwm()
{
	TCCR1B |= (1<<CS10)|(1<<CS11);
	TCCR1A |= (_BV(COM1A0) | _BV(COM1A1));
}
#define ENABLE_PWM enable_pwm()

static inline void disable_pwm()
{
	TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
	TCCR1A &= ~(_BV(COM1A0) | _BV(COM1A1));
}
#define DISABLE_PWM disable_pwm()


#define BUTTON_POLLING_INTERVAL 4	// button polling every 4ms
#define REVLIMIT_BLINK_INTERVAL 500
#define LOW_FUEL_BLINK_INTERVAL	900
#define PITLIMITER_BLINK_INTERVAL 1000

#define BUTTON_STATE_LEN 6 // size of buttons array

volatile uint8_t bt_poll_cnt; // button polling interval counter
volatile uint16_t pitLimit_intv_cnt; // pit-Limiter (blink) interval counter
volatile uint16_t lowFuel_intv_cnt; // low Fuel warning (blink) interval counter
volatile uint16_t revlimit_intv_cnt; // revlimiter (blink) interval

volatile uint8_t pwm_dc; // blue LED duty cycle (PWM dimming)

// limiter flags definition (rev- and pit limiter)
//////////////////////////////////////////////////
volatile uint8_t limiter_flags;
#define PITLIMITER_STATUS 	0x1
#define PITLIMITER_LEDS		0x2
#define PITLIMITER_TOGGLE	0x4

#define REVLIMITER_STATUS	0x8
#define REVLIMITER_LEDS		0x10
#define REVLIMITER_TOGGLE	0x20


// warning flags
/////////////////////////////////////////////////
volatile uint8_t warning_flags;
#define LOWFUEL_WARN_STATUS	0x1
#define LOWFUEL_WARN_LEDS	0x2
#define LOW_FUEL_TOGGLE		0x4

#define YFLAG_STATUS	0x8
#define YFLAG_LEDS		0x10

#define BFLAG_STATUS	0x20
#define BFLAG_LEDS		0x40


volatile uint8_t gFlags;  // global flags register
// GLOBAL FLAGS definition
/////////////////////////////////////////////////
#define BUTTON_POLL		0x1		// buttons polling interval expired
#define ROTENC_POLL		0x2		// rotary encoder read
#define BUTTON_UPDATE 	0x4		// send updated button mask to PC
#define SEL_BT			0x8		// last status of RotEnc selector PB
#define PWM_LED			0x10	// PWM enable flag
// ...
#define GF_DTR	 	0x80	// flags signaling successful sync sequence


// PIN definitions
/////////////////////////////////////////////////
#define LOWFUEL_LED_OFF (1<<2)|(1<<3)
#define YFLAG_LED_OFF	 (1<<4)
#define BFLAG_LED_OFF	 (1<<5)

/**** Rotary Encoder for Brake Bias definitions ****/
#define RENC_BB_PINA	(PINB & _BV(PB2))
#define RENC_BB_PINB	(PIND & _BV(PD7))
volatile uint8_t renc_bb_status; // status flags for Rotary Encoder

/**** Rotary Encoder Selector 1 ****/
#define RENC_SEL1_PINA	(PIND & _BV(PD6))
#define RENC_SEL1_PINB	(PIND & _BV(PD5))
#define RENC_SEL1_BUT	_BV(4)
volatile uint8_t renc_sel1_status;	// including button

/**** Rotary Encoder Selector 2 ****/
#define RENC_SEL2_PINA	(PINC & _BV(PC2))
#define RENC_SEL2_PINB	(PINC & _BV(PC3))
#define RENC_SEL2_BUT	_BV(5)
volatile uint8_t renc_sel2_status;	// including button

/**** Ports for 74HC164 for Gear-Display ****/
//#define GEAR_CLK_PIN	(1<<PB1)
//#define GEAR_DATA_PIN	(1<<PB0)


/////////////////////////////////////////////////
// BUTTONS
/////////////////////////////////////////////////
//extern uint8_t buttons1_8; 	// bitmask for buttons 1-8
//extern uint8_t buttons9_16;	// bitmask for buttons 9-16
volatile uint16_t RotSw_1;	// bitmask for rotary switch 1 (1-12)
volatile uint8_t RotSw_2; 	// bitmask for rotary switch 2 (1-6)

// These variables are used to keep track of the last status
// for static switches (i.e. rotary switches)
volatile uint8_t lastRotSw_2; // Bit 7 is used for button 14 ->
							// on/off flip switch !!!
volatile uint16_t lastRotSw_1;

// buttons mask array definitions
#define PB1_8		0
#define PB9_16		1
#define RS1_1_8 	2
#define RS1_9_12	3
#define SHIFT		3
#define RE_PB		3
#define RS2_1_6		4
#define SEL_LED		4
#define RE_BB_12	5

// Button state definitions ( for non pushbuttons )
#define BT14 7

#define RS1_1	0x1
#define RS1_2 	0x2
#define RS1_3	0x4
#define RS1_4	0x8
#define RS1_5	0x10
#define RS1_6	0x20
#define RS1_7	0x40
#define RS1_8	0x80
#define RS1_9	0x100
#define RS1_10	0x200
#define RS1_11	0x400
#define RS1_12	0x800

#define RS2_1	0x1
#define RS2_2	0x2
#define RS2_3	0x4
#define RS2_4	0x8
#define RS2_5	0x10
#define RS2_6	0x20

#define SEL_LED_ON 0x40

///* This structure defines all buttons as bit flags
// * as seen and maintained by the controller. These
// * flags are parsed and sent to PC as a button
// * byte array.
// */
//struct _buttons {
//
//	// buttons[0]
//	uint8_t pb1:1;
//	uint8_t pb2:1;
//	uint8_t pb3:1;
//	uint8_t pb4:1;
//	uint8_t pb5:1;
//	uint8_t pb6:1;
//	uint8_t pb7:1;
//	uint8_t pb8:1;
//
//	// buttons[1]
//	uint8_t pb9:1;
//	uint8_t pb10:1;
//	uint8_t pb11:1;
//	uint8_t pb12:1;
//	uint8_t pb13:1;
//	uint8_t pb14:1;
//	uint8_t pb15:1;
//	uint8_t pb16:1;
//
//	// buttons[2] - rotary switch 1 (1-8)
//	uint8_t rs1_1:1;
//	uint8_t rs1_2:1;
//	uint8_t rs1_3:1;
//	uint8_t rs1_4:1;
//	uint8_t rs1_5:1;
//	uint8_t rs1_6:1;
//	uint8_t rs1_7:1;
//	uint8_t rs1_8:1;
//
//	// buttons[3] - rotary switch 1 (9-12)
//	uint8_t rs1_9:1;
//	uint8_t rs1_10:1;
//	uint8_t rs1_11:1;
//	uint8_t rs1_12:1;
//	// rotary encoders 1 & 2 push buttons
//	uint8_t re1_pb:1;
//	uint8_t re2_pb:1;
//	// shifter up/down
//	uint8_t shift_up:1;
//	uint8_t shift_down:1;
//
//	// buttons[4] - rotary switch 2 (1-6)
//	uint8_t rs2_1:1;
//	uint8_t rs2_2:1;
//	uint8_t rs2_3:1;
//	uint8_t rs2_4:1;
//	uint8_t rs2_5:1;
//	uint8_t rs2_6:1;
//	// yellow indicator LED (rotary encoder X status)
//	uint8_t led_y:1;
//	uint8_t pad:1;	// padding bit
//}; /* struct _buttons */


#endif /* _SIMDASH_H_ */
