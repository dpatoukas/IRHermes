#include "IRremote.h"
#include "IRremoteInt.h"

#ifdef IR_TIMER_USE_ESP32
hw_timer_t *timer;
void IRTimer(); // defined in IRremote.cpp
#endif

//+=============================================================================
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
//
int  IRrecv::decode (decode_results *results)
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

IRrecv::IRrecv (int recvpin)
{
	irparams.recvpin = recvpin;
	irparams.blinkflag = 0;
}

IRrecv::IRrecv (int recvpin, int blinkpin)
{
	irparams.recvpin = recvpin;
	irparams.blinkpin = blinkpin;
	pinMode(blinkpin, OUTPUT);
	irparams.blinkflag = 0;
}

//Fetches the decode results structure which contains
//the most update data from the IR
#ifndef DEBUG
#define DEBUG 1
#endif
int IRrecv::fetch(decode_results *results)
{	

	while (decode(results))
	{	
		if ((results->arrived == EXPECTED_MSG))
		{	
#ifdef DEBUG
			Serial.println("MESSAGE ARRIVED");
			Serial.println("/=============/");
			Serial.print("Buffer expects:");
			Serial.print(EXPECTED_MSG);
			Serial.print(" elements and contains: ");
			Serial.println(results->arrived);
			for (int i = 0; i < results->arrived; i++)
			{	
				Serial.print("In place: ");
				Serial.print(i);
				Serial.print("|value: ");
				Serial.println((unsigned long)results->rcvd_array[i]);
			}
			Serial.println("/=============/");
#endif
			resetHRM(results);
			results->rcvd_pos = 0;
			results->arrived = 0;
			return true;
		}
		resume();

	}
	return false;
}
//+=============================================================================
// initialization
//
void  IRrecv::enableIRIn ( )
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
void  IRrecv::blink13 (int blinkflag)
{
	irparams.blinkflag = blinkflag;
	if (blinkflag)  pinMode(BLINKLED, OUTPUT) ;
}

//+=============================================================================
// Return if receiving new IR signals
//
bool  IRrecv::isIdle ( )
{
 return (irparams.rcvstate == STATE_IDLE || irparams.rcvstate == STATE_STOP) ? true : false;
}
//+=============================================================================
// Restart the ISR state machine
//
void  IRrecv::resume ( )
{
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;
}

#ifndef DEBUG
#define DEBUG 1
#endif

bool IRrecv::rcvedHDR(decode_results *results)
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

bool IRrecv::rcvedBITS( decode_results *results)
{
	#ifdef DEBUG
	Serial.println("Check for bits and pieces");
	#endif
	if (results->listen_state == STATE_HDR)
	{
		return getHermesBITS(results);
	}

	return false;	
}


bool IRrecv::rcvedTRL(decode_results *results)
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

bool IRrecv::deliverHRM(decode_results *results)
{   

#ifdef DEBUG
	Serial.println("Trailer was detected,these data will be safely stored");
#endif
	
	if (results->listen_state == STATE_TRAIL)
	{

#ifdef DEBUG
	  	Serial.print("Global possition: ");
	    Serial.println(results->rcvd_pos);
	  	Serial.print("Arrived: ");
	  	Serial.print(results->buffer_pos);
	  	Serial.print(" full packet(s) ");
	  	Serial.println("containing: ");
#endif	
		for (int i = 0; i <results->buffer_pos; i++)
		{
			results->rcvd_array[results->rcvd_pos++] = results->rcvd_buffer[i];
			results->arrived++;
#ifdef DEBUG
			Serial.print("Global possition: ");
	        Serial.println(results->rcvd_pos);
		  	Serial.print(" ( ");
		  	Serial.print((unsigned long)results->rcvd_buffer[i]);
		  	Serial.print(" , ");
		  	Serial.print(i);
		  	Serial.println(" ) ");
#endif
		}
		
		if (results->rcvd_pos > MAX_BUFFER)
		{
			results->listen_state = STATE_OVER;
			resetHRM(results);
			return false;
		}

		results->listen_state = STATE_WAITING;
		return true;
	}

	return false;	
}

void IRrecv::resetHRM(decode_results *results)
{
	results->listen_state = STATE_WAITING;
	results->buffer_pos = 0;
}
