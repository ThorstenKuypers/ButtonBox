// simDash.c - 
//
// implementation of sim racing dashboard and button-box
// (C)2013 - 
//
// using ATmega8
//
////////////////////////////////////////////////////////////////////////

#include "simDash.h"
#include "USART.h"
#include "TWI_Master.h"
#include "SerialProto.h"

uint8_t t[] ="Test String";

// button state array - used to send button state to PC
////////////////////////////////////////////////////////
volatile uint8_t buttons[BUTTON_STATE_LEN] ={0};
volatile uint8_t last_buttons[BUTTON_STATE_LEN] ={0};

// gears for 7-seg display
////////////////////////////////////////////////////////
volatile uint8_t gears[9] ={0x77,0x06,0xD3,0xD6,0xA6,0xF4,0xF5,0x16,0xBF};

void read_encoders();

////////////////////// Timer2 compare interrupt ////////////////////////
ISR(TIMER2_COMP_vect)
{
	bt_poll_cnt++;
	//read_encoders(); // read encoders every 1ms
	gFlags |= ROTENC_POLL;

	if (bt_poll_cnt == BUTTON_POLLING_INTERVAL)
	{
		bt_poll_cnt =0;
		gFlags |=BUTTON_POLL;
	}

#ifdef _WITH_DASHBOARD
	if (limiter_flags & REVLIMITER_STATUS)
	{
		revlimit_intv_cnt++;
		if (revlimit_intv_cnt ==REVLIMIT_BLINK_INTERVAL)
		{
			revlimit_intv_cnt =0;
			limiter_flags |=REVLIMITER_TOGGLE;
		}
	}

	if (warning_flags & LOWFUEL_WARN_STATUS)
	{
		lowFuel_intv_cnt++;
		if (lowFuel_intv_cnt ==LOW_FUEL_BLINK_INTERVAL)
		{
			lowFuel_intv_cnt =0;
			warning_flags |=LOW_FUEL_TOGGLE;
		}
	}

	if (limiter_flags & PITLIMITER_STATUS)
	{
		pitLimit_intv_cnt++;
		if (pitLimit_intv_cnt ==PITLIMITER_BLINK_INTERVAL)
		{
			pitLimit_intv_cnt =0;
			limiter_flags |=PITLIMITER_TOGGLE;
		}
	}
#endif /* _WITH_DASHBOARD */

}

ISR(TIMER1_OVF_vect)
{
	// update pwm_dc
	OCR1A = pwm_dc;
}

ISR(INT0_vect)
{
	// ensure that interrupt flag is cleared
	GIFR = (1<<INTF0);
	GICR &= ~(1<<INT0);

	PORTD &= ~_BV(CTS);

	OCR1A=pwm_dc;
	ENABLE_PWM;
	gFlags |= PWM_LED | GF_DTR;

	bt_poll_cnt =0;

//#ifdef _DEBUG
//
//	uint8_t t ='W';
//	usart_send((uint8_t*)&t,1);
//	usart_send((uint8_t*)&gFlags,1);
//
//#endif
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void init()
{
	gFlags =0;
	bt_poll_cnt =0;
	renc_bb_status =0;
	renc_sel1_status =0;
	renc_sel2_status =0;
	proto_cc =0;

	pwm_dc =128;

	lastRotSw_1 =0;
	lastRotSw_2 =0;
	RotSw_1 =0;
	RotSw_2 =0;

	// set in/outputs of uC
	// set all ports as output
	DDRC =0xFF;
	DDRB =0xFF;
	DDRD =0xFF;

	// all ports low
	PORTC =0;
	PORTB =0;
	PORTD =0;

	DDRC &= ~((1<<PC2)|(1<<PC3)|(1<<PC0));
	DDRD &= ~((1<<PD2)|(1<<PD5)|(1<<PD6)|(1<<PD7));
	DDRB &= ~((1<<PB2));

	// enable pull-ups for rotary encoder inputs
	PORTC |= (1<<PC2)|(1<<PC3);
	PORTD |= (1<<PD5)|(1<<PD6)|(1<<PD7);
	PORTB |= (1<<PB2);

	// set CTS line HIGH during initialization
	// (! the MAX232 inverts! the signal !)
	PORTD |=_BV(CTS);

	// disable blue LEDs
	// ( needed because of PNP transistor -> active LOW)
	PORTB |= _BV(PB1);

	// enable ADC0 for temp sensor
	// ...

	init_usart();
	TWI_Master_Initialise();

	// init Timer2 (8-bit)
	TCCR2 =(1<<WGM21);	// (CTC-mode)/)
	OCR2 =250;	//interrupt fires every 1ms
	TIMSK =(1<<OCIE2);	// compare interrupt

	// configure PB1 (OC1A) for PWM (-> LEDs)
	// init TIMER1 for 8-bit Fast-PWM
	TCCR1A = (1<<WGM10);
	TCCR1B = (1<<WGM12);
	TIMSK |= (1<<TOIE1);

	sei();

	// enable TIMER2 after interrupts are enabled (setting CS22
	// immediately enables the timer!)
	(TCCR2 |= (1<<CS22)); // enable timer with prescaler of 64

	ENABLE_PWM;
	gFlags |= PWM_LED;

	// now after TWI is initialized and interrupts are enabled,
	// initialize PCF8574 ICs via TWI
	// set all pins of PCF8574 HIGH
	for (uint8_t i =0; i < 5; i++) {
		uint8_t sla =((0x20+i)<<1)|0x0;
		uint8_t buf[2] ={sla,0xFF};

		TWI_Start_Transceiver_With_Data((unsigned char*)&buf[0], 2);
	}
}

void read_encoders()
{
	// clear old encoder values
	//buttons[RE_BB_12] =0;

	/*** BRAKE BIAS encoder ***/
	if (!RENC_BB_PINA)
		renc_bb_status |=2;
	if (!RENC_BB_PINB)
		renc_bb_status |=1;

	if ((renc_bb_status & 0xC) ==0xC) {
		renc_bb_status &= 3;

		if (renc_bb_status == 1) {
			buttons[RE_BB_12] =0x40;
			//			gFlags |= BUTTON_UPDATE;
		}
		else if (renc_bb_status ==2) {
			buttons[RE_BB_12] =0x80;
			//			gFlags |= BUTTON_UPDATE;
		}
	}
	renc_bb_status =(renc_bb_status << 2) & 0xF;
	/***	***/

	/*** selection encoder 1 ***/
	if (!RENC_SEL1_PINA)
		renc_sel1_status |=2;
	if (!RENC_SEL1_PINB)
		renc_sel1_status |=1;

	if ((renc_sel1_status & 0xC) ==0xC) {
		renc_sel1_status &= 3;

		if (renc_sel1_status ==1) {
			if (lastRotSw_2 & SEL_LED_ON) {
				buttons[RE_BB_12] =0x10;
				//				gFlags |= BUTTON_UPDATE;
			}
			else {
				buttons[RE_BB_12] =0x1;
				//				gFlags |= BUTTON_UPDATE;
			}
		}
		else if (renc_sel1_status ==2) {
			if (lastRotSw_2 & SEL_LED_ON) {
				buttons[RE_BB_12] =0x20;
				//				gFlags |= BUTTON_UPDATE;
			}
			else {
				buttons[RE_BB_12] =0x2;
				//				gFlags |= BUTTON_UPDATE;
			}
		}
	}
	renc_sel1_status = (renc_sel1_status << 2) & 0xF;
	/***	***/

	/*** selection encoder 2 ***/
	if (!RENC_SEL2_PINA)
		renc_sel2_status |=2;
	if (!RENC_SEL2_PINB)
		renc_sel2_status |=1;

	if ((renc_sel2_status & 0xC) ==0xC) {
		renc_sel2_status &= 3;

		if (renc_sel2_status ==1) {
			buttons[RE_BB_12] =0x4;
			//			gFlags |= BUTTON_UPDATE;
		}
		else if (renc_sel2_status ==2) {
			buttons[RE_BB_12] =0x8;
			//			gFlags |= BUTTON_UPDATE;
		}
	}
	renc_sel2_status = (renc_sel2_status << 2) & 0xF;
	/***	***/
}

// update_config();
void update_config(uint8_t cc, uint8_t* buf, uint8_t len)
{
	if (buf !=NULL) {

		if (buf[0] =='L') { // update LED brightness sub-command

			// set PWM duty cycle here to dim blue LEDs
			pwm_dc =buf[1];

		}
	}
}



////////////////////////// MAIN ////////////////////////////
int main(void)
{
	//	pitLimit_intv_cnt =0;
	//	lowFuel_intv_cnt =0;
	//	limiter_flags =0;
	//	warning_flags =0;

	/*** will be changed to uint16_t absolute rpm value in the future ***/
	/*	uint8_t rpm_mask =0;
	uint8_t fuel_mask =0;
	uint8_t rpm =0;
	uint8_t fuel =0;
	uint8_t gear_idx =0; // index into gears array fo 7-seg display
	uint8_t misc_mask =0; // misc bitmask (for indicator lights eg low-fuel)
	 */

	uint8_t proto_buf[MAX_PAYLOAD_LEN] ={0};
	uint8_t len =0;

	uint8_t update_mask =0;
	uint8_t update =0;

	uint8_t twi_buf[2] ={0};

	init();

	// this delay is needed to ensure that the USART is fully initialized,
	// before the uC is put to sleep! (in case the uC is enabled before
	// the PC software becomes available!!!)
	_delay_ms(1);

	// set CTS line LOW to signal PC that uC init completed
	// and uC is ready to receive data
	// MAX232 inverts the signal
	PORTD &= ~_BV(CTS);

	for(;;)
	{

		if ((PIND & _BV(DTR)))
		{

			// if PC is not available, reset all last button states
			// to send new states when PC re-connects
			lastRotSw_2 &= ~0x3F;
			lastRotSw_2 &= ~_BV(BT14);
			lastRotSw_1 =0;
			for (uint8_t i =0; i < BUTTON_STATE_LEN; i++)
				last_buttons[i] =0;

			// set sleep mode

			//usart_send((uint8_t*)&gFlags,1);

			while (!usart_tx_empty());

			// switch off blue LEDs
			DISABLE_PWM;
			PORTB |= _BV(PB1);

			//gFlags &= ~(GF_DTR | PWM_LED);
			gFlags =0;
			PORTD |=_BV(CTS);

			uint8_t sr =SREG;
			cli();

			// configure PD2 as external interrupt ( on LOW level )
			// to wake MCU from sleep mode
			MCUCR &= ~((1<<ISC01)|(1<<ISC00));
			GICR |= (1<<INT0);

			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_enable();

			SREG =sr;
			sleep_cpu();

			sleep_disable();

			//init_usart();

			TWCR &= ~((1<<TWSTO) | (1<<TWEN));
			TWCR |= (1<<TWEN);


		}

		if (proto_cc ==0) {
			if (usart_avail() >= 1) {
				proto_cc =usart_getbyte();
			}

			// get length of protocol data
			len =(proto_cc & PROTO_LEN_MASK);

			// get protocol data
			if (len > 0 && len <=MAX_PAYLOAD_LEN) {
				// wait until len bytes arrived
				while (usart_avail() < len);
				for (int i =0; i < len; i++)
					proto_buf[i] =usart_getbyte();
			}

			// parse serial protocol cc byte
			if (proto_cc & PROTO_CONFIG) {

				// received data contains configuration
				// ...
				if (proto_cc & PROTO_FRAG) {

					// fragmentation flag set; so the received data is a
					// continuation of the last command
					// ...
				}

				update_config(proto_cc, proto_buf, len);
			}
			if (proto_cc & PROTO_RECV) {

				// data received from PC; so update outputs
				// with this data
				// ...

				update_mask =proto_buf[0];
#ifdef _WITH_DASHBOARD
				if (update_mask & RPM_UPDATE)
				{
					rpm_mask =proto_buf[1];
				}
				if (update_mask & FUEL_UPDATE)
				{
					//						if (update_mask & RPM_UPDATE)
					//						{
					fuel_mask =proto_buf[2];
					//						}
					//						else
					//						{
					//							fuel_mask =pload_buf[1];
					//						}
				}
				if (update_mask & GEAR_UPDATE)
				{
					gear_idx =proto_buf[3];
				}
#endif // _WITH_DASHBOARD
				update =1;
			}
		}

		if (update)
		{
#ifdef _WITH_DASHBOARD
			if (update_mask & RPM_UPDATE)
			{
				rpm = ~rpm_mask;

				if (update_mask & REV_LIMIT_WARN)
				{
					if (limiter_flags & REVLIMITER_STATUS)
					{
						limiter_flags &= ~REVLIMITER_LEDS;
						limiter_flags &= ~REVLIMITER_TOGGLE;
						limiter_flags &= ~REVLIMITER_STATUS;
						revlimit_intv_cnt =0;

						twi_buf[0] =0x38;	// TWI address 
						twi_buf[1] =rpm;
						TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
					}
					else
					{
						limiter_flags |= REVLIMITER_STATUS;
						limiter_flags |= REVLIMITER_LEDS;

						twi_buf[0] =0x38;	// TWI address 
						twi_buf[1] =REVLIMIT_LEDS_ON;
						TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
					}
				}
				else
				{
					twi_buf[0] =0x38;	// TWI address 
					twi_buf[1] =rpm;
					TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
				}
			}

			if (update_mask & FUEL_UPDATE)
			{
				fuel =~fuel_mask;

				if (update_mask & LOW_FUEL_WARN)
				{
					if (warning_flags & LOWFUEL_WARN_STATUS)
					{
						warning_flags &= ~LOWFUEL_WARN_STATUS;
						warning_flags &= ~LOWFUEL_WARN_LEDS;
						warning_flags &= ~LOW_FUEL_TOGGLE;
						lowFuel_intv_cnt =0;
					}
					else
					{
						warning_flags |= LOWFUEL_WARN_STATUS;
						warning_flags |=LOWFUEL_WARN_LEDS;
					}
				}
				else
				{
					twi_buf[0] =0x39;
					twi_buf[1] =fuel;
					TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
				}
			}

			if (update_mask & PIT_LIMITER)
			{
				if (limiter_flags & PITLIMITER_STATUS)
				{
					twi_buf[0] =0x38;	// TWI address 
					twi_buf[1] =0xFF;
					TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);

					limiter_flags &= ~PITLIMITER_STATUS;
					limiter_flags &= ~PITLIMITER_LEDS;
					limiter_flags &= ~PITLIMITER_TOGGLE;
					pitLimit_intv_cnt =0;
				}
				else
				{
					twi_buf[0] =0x38;	// TWI address 
					twi_buf[1] =0;
					TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);

					limiter_flags |= PITLIMITER_STATUS;
					limiter_flags |= PITLIMITER_LEDS;
				}
			}

			// not useable with PCF8574 and TWI
			// using Shift-register (74HC164) instead
			if (update_mask & GEAR_UPDATE)
			{
				uint8_t g =gears[gear_idx];

				//...
				//
			}

			/*** Yellow and Blue flag warning indicator lights are NOT
				 flashing (v1.0)!
			 ***/
			if (update_mask & YELLOW_FLAG)
			{
				if (warning_flags & YFLAG_STATUS)
				{
					warning_flags &= ~YFLAG_LEDS;	
					warning_flags &= ~YFLAG_STATUS;
					misc_mask |= YFLAG_LED_OFF;
					twi_buf[0] =0x3A;
					twi_buf[1] =misc_mask;
					TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
				}
				else
				{
					warning_flags |= YFLAG_LEDS;	
					warning_flags |= YFLAG_STATUS;
					misc_mask &= ~YFLAG_LED_OFF;
					twi_buf[0] =0x3A;
					twi_buf[1] =misc_mask;
					TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
				}
			}

			if (update_mask & BLUE_FLAG)
			{
				if (warning_flags & BFLAG_STATUS)
				{
					warning_flags &= ~BFLAG_LEDS;	
					warning_flags &= ~BFLAG_STATUS;
					misc_mask |= BFLAG_LED_OFF;
					twi_buf[0] =0x3A;
					twi_buf[1] =misc_mask;
					TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
				}
				else
				{
					warning_flags |= BFLAG_LEDS;	
					warning_flags |= BFLAG_STATUS;
					misc_mask &= ~BFLAG_LED_OFF;
					twi_buf[0] =0x3A;
					twi_buf[1] =misc_mask;
					TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
				}
			}
#endif /* _WITH_DASHBOARD */

			update =0;
		}

#ifdef _WITH_DASHBOARD
		if (warning_flags & LOW_FUEL_TOGGLE)
		{
			if (warning_flags & LOWFUEL_WARN_LEDS)
			{
				warning_flags &= ~LOWFUEL_WARN_LEDS;
				warning_flags &= ~LOW_FUEL_TOGGLE;
				misc_mask |= LOWFUEL_LED_OFF;
				twi_buf[0] =0x3A;
				twi_buf[1] =misc_mask;
				TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
			}
			else
			{
				warning_flags |= LOWFUEL_WARN_LEDS;
				warning_flags &= ~LOW_FUEL_TOGGLE;
				misc_mask &= ~LOWFUEL_LED_OFF;
				twi_buf[0] =0x3A;
				twi_buf[1] =misc_mask;
				TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
			}
		}


		if (limiter_flags & REVLIMITER_TOGGLE)
		{
			if (limiter_flags & REVLIMITER_LEDS)
			{
				twi_buf[0] =0x38;	// TWI address 
				twi_buf[1] =REVLIMIT_LEDS_OFF;
				TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);

				limiter_flags &= ~REVLIMITER_LEDS;
				limiter_flags &= ~REVLIMITER_TOGGLE;
			}
			else 
			{
				twi_buf[0] =0x38;	// TWI address 
				twi_buf[1] =REVLIMIT_LEDS_ON;
				TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);

				limiter_flags |= REVLIMITER_LEDS;
				limiter_flags &= ~REVLIMITER_TOGGLE;
			}
		}

		if (limiter_flags & PITLIMITER_TOGGLE)
		{
			if (limiter_flags & PITLIMITER_LEDS)
			{				
				twi_buf[0] =0x38;	// TWI address 
				twi_buf[1] =0xFF;
				TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);

				limiter_flags &= ~PITLIMITER_LEDS;
				limiter_flags &= ~PITLIMITER_TOGGLE;
			}
			else 
			{
				twi_buf[0] =0x38;	// TWI address 
				twi_buf[1] =0;
				TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);

				limiter_flags |= PITLIMITER_LEDS;
				limiter_flags &= ~PITLIMITER_TOGGLE;
			}
		}
#endif /* _WITH_DASHBOARD */

		if (gFlags & ROTENC_POLL)
		{
			read_encoders();

#ifdef _DEBUG_LED
			//			if ((buttons[RE_BB_12] & 0x40) ==0x40) {
			//				PORTB &= ~_BV(PB1);
			//			}
			//			else if ((buttons[RE_BB_12] & 0x1) ==0x1) {
			//				PORTB &= ~_BV(PB1);
			//			}
			//			else if ((buttons[RE_BB_12] & 0x4) ==0x4) {
			//				PORTB &= ~_BV(PB1);
			//			}
			//			else
			//				PORTB |= _BV(PB1);

			if (buttons[RE_BB_12] & 0x10) { // CW
				if (!(gFlags & PWM_LED)) {
					// enable PWM timer
					TCCR1B |= (1<<CS10)|(1<<CS11);

					// set PWM flag
					gFlags |= PWM_LED;
				}
				pwm_dc +=3;
				if (pwm_dc >= (0xFF - 4))
					pwm_dc =0xFF;
			}
			else if (buttons[RE_BB_12] & 0x20) { // CCW
				if (pwm_dc > 3)
					pwm_dc -=3;
				else {
					pwm_dc =0;

					if (gFlags & PWM_LED) {
						// disable PWM timer
						TCCR1B &= ~((1<<CS10)|(1<<CS11));

						// set LED PORT HIGH (off)
						PORTB |= (1<<PB1);

						// clear PWM enable flag
						gFlags &= ~PWM_LED;
					}
				}
			}
#endif

			if (last_buttons[RE_BB_12] !=buttons[RE_BB_12])
				gFlags |=BUTTON_UPDATE;
			last_buttons[RE_BB_12] =buttons[RE_BB_12];

			gFlags &= ~ROTENC_POLL;
		}

		if (gFlags & BUTTON_POLL)
		{

			// read buttons via TWI
			for (int i =0; i < 5; i++)
			{
				// read button state byte from I/O expander
				// address byte - Bit7:1 = slave address
				//				  Bit0	 = R/W bit
				twi_buf[0] = ((0x20 + i)<<1)|0x1;
				twi_buf[1] =0;

				TWI_Start_Transceiver_With_Data(&twi_buf[0], 2);
				TWI_Get_Data_From_Transceiver(&twi_buf[0], 2);

				// parse button states received from port expanders
				uint8_t b = ~twi_buf[1];

				// mask out LED bit
				if (i ==SEL_LED && (b & SEL_LED_ON))
					b &= ~SEL_LED_ON;

				buttons[i] =b;

			}

			if (buttons[PB9_16] & _BV(5)) { // flip switch is on
				if (!(lastRotSw_2 & _BV(BT14))) { // and was off
					lastRotSw_2 |= _BV(BT14); // flip switch stays on (to send pulse to PC
					// and last state of switch is on
				}
				else { // and was on
					buttons[PB9_16] &= ~_BV(5); // clear button flag to prvent sending another pulse to PC
				}
			}
			else { // flip switch is off
				if (lastRotSw_2 & _BV(BT14)) { // and was on
					// send new pulse
					buttons[PB9_16] |= _BV(5);
					lastRotSw_2 &= ~_BV(BT14);
				}
			}

			// Rotary Encoder1 push button pressed...
			if ( (buttons[RE_PB] & 0x80) &&
					(!(gFlags & SEL_BT)) ) {
				// and LED status OFF...
				if (!(lastRotSw_2 & SEL_LED_ON)) {
					// ...enable selector LED (yellow)
					uint8_t led[2] ={(0x24<<1) | 0x0, ~0x40};
					TWI_Start_Transceiver_With_Data(led, 2);

					// set LED status to ON
					lastRotSw_2 |= SEL_LED_ON;
				}
				else {
					// disable LED
					uint8_t led[2] ={(0x24<<1) | 0x0, 0xFF};
					TWI_Start_Transceiver_With_Data(led, 2);

					// set LED status to OFF
					lastRotSw_2 &= ~SEL_LED_ON;
				}
			}

			RotSw_1 =(buttons[3] & 0xF);
			RotSw_1 <<=8;
			RotSw_1 |= buttons[2] & 0xFF;

			// bit 7 of lastRotSw_1 is used for button 14
			// status; so clear low 6 bits first before
			// assigning new value
			RotSw_2 &= ~0x3F;
			RotSw_2 |= (buttons[RS2_1_6] & 0x3F);


			if (lastRotSw_1 ==RotSw_1) {
				if (RotSw_1 & RS1_1)
					buttons[RS1_1_8] &= ~_BV(0);
				else if (RotSw_1 & RS1_2)
					buttons[RS1_1_8] &= ~_BV(1);
				else if (RotSw_1 & RS1_3)
					buttons[RS1_1_8] &= ~_BV(2);
				else if (RotSw_1 & RS1_4)
					buttons[RS1_1_8] &= ~_BV(3);
				else if (RotSw_1 & RS1_5)
					buttons[RS1_1_8] &= ~_BV(4);
				else if (RotSw_1 & RS1_6)
					buttons[RS1_1_8] &= ~_BV(5);
				else if (RotSw_1 & RS1_7)
					buttons[RS1_1_8] &= ~_BV(6);
				else if (RotSw_1 & RS1_8)
					buttons[RS1_1_8] &= ~_BV(7);
				else if (RotSw_1 & RS1_9)
					buttons[RS1_9_12] &= ~_BV(0);
				else if (RotSw_1 & RS1_10)
					buttons[RS1_9_12] &= ~_BV(1);
				else if (RotSw_1 & RS1_11)
					buttons[RS1_9_12] &= ~_BV(2);
				else if (RotSw_1 & RS1_12)
					buttons[RS1_9_12] &= ~_BV(3);
			}
			else {
				lastRotSw_1 =RotSw_1;
			}

			if ( (lastRotSw_2 & 0x3F) ==RotSw_2) {
				if (RotSw_2 & RS2_1)
					buttons[RS2_1_6] &= ~_BV(0);
				else if (RotSw_2 & RS2_2)
					buttons[RS2_1_6] &= ~_BV(1);
				else if (RotSw_2 & RS2_3)
					buttons[RS2_1_6] &= ~_BV(2);
				else if (RotSw_2 & RS2_4)
					buttons[RS2_1_6] &= ~_BV(3);
				else if (RotSw_2 & RS2_5)
					buttons[RS2_1_6] &= ~_BV(4);
				else if (RotSw_2 & RS2_6)
					buttons[RS2_1_6] &= ~_BV(5);
			}
			else {
				lastRotSw_2 &= ~0x3F;
				lastRotSw_2 |= RotSw_2;
			}


			// mask bits that should not arrive PC
			if (buttons[RE_PB] & 0x80) {
				buttons[RE_PB] &= ~0x80;
				gFlags |= SEL_BT;
			}
			else
				gFlags &= ~SEL_BT;

			// check if buttons have changed since last poll
			for (uint8_t i =0; i < BUTTON_STATE_LEN; i++) {

				if (buttons[i] !=last_buttons[i])
					gFlags |=BUTTON_UPDATE;
				last_buttons[i] =buttons[i];
			}

		}

		// hold transmission of data to PC when DTR flag is clear
		//if ( ((gFlags & GF_DTR) ==GF_DTR) ){

		if (!(PIND & _BV(DTR))) {

			if ((gFlags & BUTTON_UPDATE) ==BUTTON_UPDATE) {
				// send buttons state array to PC
				proto_cc =PROTO_SEND | (BUTTON_STATE_LEN & PROTO_LEN_MASK);
				usart_send((uint8_t*)&proto_cc, 1);
				usart_send((uint8_t*)&buttons[0], BUTTON_STATE_LEN);

				proto_cc =0;
				for (int i=0; i<BUTTON_STATE_LEN; i++)
					buttons[i] =0;

				gFlags &= ~BUTTON_UPDATE;
			}
		}

		proto_cc =0;
		gFlags &= ~BUTTON_POLL;
	}
}
