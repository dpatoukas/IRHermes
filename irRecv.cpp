#include "IRHermes.h"
#include "IRHermesInt.h"

#ifdef IR_TIMER_USE_ESP32
hw_timer_t *timer;
void IRTimer(); // defined in IRremote.cpp
#endif

//+=============================================================================
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
//
int  IRHermes::decode (decode_results *results)
{ 
	results->rawbuf   = irparams.rawbuf;
	results->rawlen   = irparams.rawlen;
	results->overflow = irparams.overflow;

	if (irparams.rcvstate != STATE_STOP)  return false ;

	results->listen_state = STATE_WAITING;
	//Check for HDR
	if(!rcvedHDR(results)) {
		resume();
		return false;
	}

	if(!rcvedSEQ(results)) {
		resume();
		resetHRM(results);
		return false;
	}

	if(!is_valid(results)){
		resume();
		resetHRM(results);
		return false;
	}

	//Check for bits 
	if(!rcvedBITS(results)) {
		resume();
		return false;
	}
	
	//Check for trailer
	if(!rcvedTRL(results)) {
		resume();
		return false;
	}
	
	//Validate result 
	if(!deliverHRM(results)) {
		resume();
		return false;
	}
	
	return true;
}

IRHermes::IRHermes (int recvpin)
{
	irparams.recvpin = recvpin;
	irparams.blinkflag = 0;
}

IRHermes::IRHermes (int recvpin, int blinkpin)
{
	irparams.recvpin = recvpin;
	irparams.blinkpin = blinkpin;
	pinMode(blinkpin, OUTPUT);
	irparams.blinkflag = 0;
}

//Fetches the decode results structure which contains
//the most updated data from the IR
int IRHermes::fetch(decode_results *results)
{	

	while (decode(results))
	{	
#ifdef DEBUG
		Serial.println("MSGs Arrived:");
		Serial.println(results->arrived);
#endif
		if (results->arrived == EXPECTED_DATA)
		{	
#ifdef DEBUG
			Serial.println("MESSAGE ARRIVED");
			Serial.println("/=============/");
			Serial.print("Buffer expects:");
			Serial.print(EXPECTED_MSG);
			Serial.print(" elements and contains: ");
			Serial.println(results->arrived);
			Serial.println("/=============/");
#endif
			resetHRM(results);
			resume();
			return true;
		}
		resume();

	}
	return false;
}
//+=============================================================================
// initialization
//
void  IRHermes::enableIRIn ( )
{	
// Interrupt Service Routine - Fires every 50uS
#ifdef ESP32
	// ESP32 has a proper API to setup timers, no weird chip macros needed
	// simply call the readable API versions :)
	// 3 timers, choose #1, 80 divider nanosecond precision, 1 to count up
	timer = timerBegin(1, 80, 1);
	timerAttachInterrupt(timer, &IRTimer, 1);
	// every 50ns, autoreload = true
	timerAlarmWrite(timer, 50, true);
	timerAlarmEnable(timer);
#else
	cli();
	// Setup pulse clock timer interrupt
	// Prescale /8 (16M/8 = 0.5 microseconds per tick)
	// Therefore, the timer interval can range from 0.5 to 128 microseconds
	// Depending on the reset value (255 to 0)
	TIMER_CONFIG_NORMAL();

	// Timer2 Overflow Interrupt Enable
	TIMER_ENABLE_INTR;

	TIMER_RESET;

	sei();  // enable interrupts
#endif

	// Initialize state machine variables
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;

	// Set pin modes
	pinMode(irparams.recvpin, INPUT);
}

//+=============================================================================
// Enable/disable blinking of pin 13 on IR processing
//
void  IRHermes::blink13 (int blinkflag)
{
	irparams.blinkflag = blinkflag;
	if (blinkflag)  pinMode(BLINKLED, OUTPUT) ;
}

//+=============================================================================
// Return if receiving new IR signals
//
bool  IRHermes::isIdle ( )
{
 return (irparams.rcvstate == STATE_IDLE || irparams.rcvstate == STATE_STOP) ? true : false;
}
//+=============================================================================
// Restart the ISR state machine
//
void  IRHermes::resume ( )
{
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;
}

bool IRHermes::rcvedHDR(decode_results *results)
{	
	#ifdef DEBUG
	Serial.println("Check for header ");
	#endif
	if (results->listen_state == STATE_WAITING)
	{
		return getHermesHDR(results);
	}

	return false;
}

bool IRHermes::rcvedSEQ(decode_results *results)
{	
	#ifdef DEBUG
	Serial.println("Check for sequence ");
	#endif
	if (results->listen_state == STATE_HDR)
	{
		return getHermesSEQ(results);
	}

	return false;
}

bool IRHermes::rcvedBITS( decode_results *results)
{
	#ifdef DEBUG
	Serial.println("Check for bits and pieces");
	#endif
	if (results->listen_state == STATE_SEQ)
	{
		return getHermesBITS(results);
	}

	return false;	
}


bool IRHermes::rcvedTRL(decode_results *results)
{
#ifdef DEBUG
	Serial.println("Check for trailer");
#endif
	if (results->listen_state == STATE_BIT)
	{
		return getHermesTRL(results);
	}

	return false;
}

bool IRHermes::deliverHRM(decode_results *results)
{   

#ifdef DEBUG
	Serial.println("Trailer was detected,these data will be safely stored");
#endif
	
	if (results->listen_state == STATE_TRAIL)
	{
	
	  	results->previous_pos = results->rcvd_pos;
	  	results->previous_arr = results->arrived;

		for (int i = 0; i <results->buffer_pos; i++)
		{
			results->rcvd_array[results->rcvd_pos++] = results->rcvd_buffer[i];
			results->arrived++;
			// Serial.print("(rcvd_pos , arrived) = (");
			// Serial.print(results->rcvd_pos);
			// Serial.print(" , ");
			// Serial.print(results->arrived);
			// Serial.println(" )");
		}

		
		if (results->rcvd_pos > MAX_BUFFER)
		{	

			Serial.println("DLVR OVFL");
			Serial.println(results->rcvd_pos);
			results->listen_state = STATE_OVER;
			resetHRM(results);
			return false;
		}

		results->previous_seq = results->current_seq;
		vld_t.last_valid = results->previous_seq;
		results->listen_state = STATE_WAITING;
		return true;
	}

	return false;	
}

void IRHermes::resetHRM(decode_results *results)
{
	results->listen_state = STATE_WAITING;
	results->buffer_pos = 0;
	results->rcvd_pos = 0;
	results->arrived = 0;

	results->previous_arr = 0;
	results->previous_pos = 0;
	results->previous_seq = 0;

	vld_t.lock = 0;
	vld_t.last_valid = 0;
	vld_t.cand = 0;
}

bool IRHermes::is_valid(decode_results *results)
{
	
	switch (vld_t.lock){

		case 0: 
			if ((results->rcvd_pos == 0) && (results->current_seq == 0))
			{
				//Serial.println("TX");
				vld_t.lock 		= 1;
				vld_t.last_valid 	= 0;
				return true;
			}
		case 1:
			if ((vld_t.cand == vld_t.last_valid + 1) || (vld_t.cand == vld_t.last_valid)) 
			{
				return true;
			}
		
		default:
			// if (vld_t.cand - vld_t.last_valid > 1)
			// {
			// 	//Serial.println("!1");
			// }
			// if (vld_t.cand - vld_t.last_valid < 0)
			// {
			// 	//Serial.print("!0");
			// }
			//Serial.println("lock");
			vld_t.lock = 0;
			resetHRM(results);
			return false;
	}
}
