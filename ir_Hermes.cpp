#include "IRremote.h"
#include "IRremoteInt.h"

//#define HERMES
//#define HERMES_BITS 32
#define HERMES_HDR_MARK 4500
#define HERMES_HDR_MARK 4500
#define HERMES_BIT_MARK 590
#define HERMES_ONE_SPACE 1690
#define HERMES_ZERO_SPACE 590

#define HERMES_MAX_BIT_MSG	256
#define HERMES_TRAILER_SPACE 3000

#if SEND_HERMES
void  IRsend::sendHermes (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark(HERMES_HDR_MARK);
	space(HERMES_HDR_MARK);

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark(HERMES_BIT_MARK);
			space(HERMES_ONE_SPACE);
		} else {
			mark(HERMES_BIT_MARK);
			space(HERMES_ZERO_SPACE);
		}
	}

#ifdef DEBUG 
	Serial.println("Sent 32bit data");
#endif

	// Footer
	mark(HERMES_TRAILER_SPACE);
	space(HERMES_TRAILER_SPACE);   
    space(0);  // Always end with the LED off

}
#endif

#ifdef DECODE_HERMES
bool IRrecv::getHermesHDR(decode_results *results)
{ 	
	results->current_pos = 1;  // Skip first space

	if (!MATCH_MARK(results->rawbuf[results->current_pos], HERMES_HDR_MARK))   return false ;
	results->current_pos++;

	if (irparams.rawlen > (2 * HERMES_MAX_BIT_MSG) + 4)
	{
		results->overflow = 1;
		return false ;	
	} 

	// Initial space
	if (!MATCH_SPACE(results->rawbuf[results->current_pos++], HERMES_HDR_MARK))  return false ;

	results->listen_state = STATE_HDR;
	return true;
}

bool IRrecv::getHermesBITS(decode_results *results)
{
	long data = 0;
	int count = 0;
	results->buffer_pos = 0;

	while(results->current_pos < (2 * HERMES_MAX_BIT_MSG) + 4)
	{

		if (!MATCH_MARK(results->rawbuf[results->current_pos++], HERMES_BIT_MARK))  return false ;
		
		if      (MATCH_SPACE(results->rawbuf[results->current_pos], HERMES_ONE_SPACE))
		{
			data = (data << 1) | 1 ;
			count++;
		
		}   
		else if (MATCH_SPACE(results->rawbuf[results->current_pos], HERMES_ZERO_SPACE))
		{
			data = (data << 1) | 0 ;
			count++; 

		}else	return false ;

		results->current_pos++;

		//received one whole packet -> save it to the buffer
		if (count == PACKET_SIZE)
		{	

#ifdef DEBUG
			Serial.print("Received: 32bits: ");
			Serial.println((unsigned long) data);
#endif			
			//TODO:Add overflow protection
			results->rcvd_buffer[results->buffer_pos++] = data;
			count = 0;
			data = 0;
		}
	    
	    if (MATCH_MARK(results->rawbuf[results->current_pos], HERMES_TRAILER_SPACE))
		{	
			break;
		}
		
	}

	results->listen_state = STATE_BIT;
	return true;

}

bool IRrecv::getHermesTRL(decode_results *results)
{
	    
	if (!MATCH_SPACE(results->rawbuf[results->current_pos], HERMES_TRAILER_SPACE))  return false ;


	results->listen_state = STATE_TRAIL;
	return true;

}
#endif