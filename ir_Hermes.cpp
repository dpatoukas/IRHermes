#include "IRHermes.h"
#include "IRHermesInt.h"


/*Signal duration 
*-----------------------------------------------------------------------------------------------------------
*/
/*HEAD*/
#define HERMES_HDR_MARK 4500
#define HERMES_HDR_MARK 4500
/*BODY*/
#define HERMES_BIT_MARK 590
#define HERMES_ONE_SPACE 1690
#define HERMES_ZERO_SPACE 590
/*TAIL*/
#define HERMES_TRAILER_SPACE 3000
/*
*-----------------------------------------------------------------------------------------------------------
*/
#define HERMES_MAX_BIT_MSG	32



/*Sending 
*-----------------------------------------------------------------------------------------------------------
*/

#if SEND_HERMES
void  IRsendHermes::sendHermes (unsigned long data,  int nbits)
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

	// Footer
	mark(HERMES_TRAILER_SPACE);
	space(HERMES_TRAILER_SPACE);   
    space(0);  // Always end with the LED off

}
#endif

/*Receiving 
*-----------------------------------------------------------------------------------------------------------
*/

#ifdef DECODE_HERMES
//Look for the Header of the message
bool IRHermes::getHermesHDR(decode_results *results)
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

//Read packets of 32bits and store them in the buffer until either power the transmitter
//stops or the trailer has reached
bool IRHermes::getHermesBITS(decode_results *results)
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

		//if received one whole packet -> save it to the buffer
		if (count == PACKET_SIZE)
		{	
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

bool IRHermes::getHermesTRL(decode_results *results)
{
	    
	if (!MATCH_SPACE(results->rawbuf[results->current_pos], HERMES_TRAILER_SPACE))  return false ;


	results->listen_state = STATE_TRAIL;
	return true;

}
#endif