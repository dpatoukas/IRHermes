#include "IRHermes.h"
#include "IRHermesInt.h"


/*Signal duration 
*-----------------------------------------------------------------------------------------------------------
*/
/*HEAD*/
#define HERMES_HDR_MARK 4500
/*BODY*/
#define HERMES_BIT_MARK 579
#define HERMES_ONE_SPACE 1684
#define HERMES_ZERO_SPACE 579
/*TAIL*/
#define HERMES_TRAILER_SPACE 3000
/*
*-----------------------------------------------------------------------------------------------------------
*/
#define HERMES_MAX_BIT_MSG	64
#define HERMES_SEQUENCE_SIZE 8


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
	// Serial.print("( ");
	// for (int i = 0; i < RAWBUF; i++)
	// {
	// 	Serial.print(results->rawbuf[i]);
	// 	Serial.print(" ,");
	// }
	// Serial.println("END)");

	results->current_pos = 1;  // Skip first space
	if (!MATCH_MARK(results->rawbuf[results->current_pos], HERMES_HDR_MARK))   return false ;
	results->current_pos++;

	//TODO:Add overflow protection


	// Initial space
	if (!MATCH_SPACE(results->rawbuf[results->current_pos++], HERMES_HDR_MARK))  return false ;
	
	results->listen_state = STATE_HDR;
	return true;
}

bool IRHermes::getHermesSEQ(decode_results *results)
{
	uint8_t data = 0;
	uint8_t pointer = 1;
	int count = 0;


	while(results->current_pos < ((2 * HERMES_SEQUENCE_SIZE) + 2) ) 
	{

		if (!MATCH_MARK(results->rawbuf[results->current_pos++], HERMES_BIT_MARK))  return false ;
		
		if      (MATCH_SPACE(results->rawbuf[results->current_pos], HERMES_ONE_SPACE))
		{
			data |= pointer;
			pointer <<= 1; 
			count++;
		}   
		else if (MATCH_SPACE(results->rawbuf[results->current_pos], HERMES_ZERO_SPACE))
		{
			pointer <<= 1;
			count++; 
		
		}else	return false ;

		results->current_pos++;

		if (count == HERMES_SEQUENCE_SIZE)
		{		
				//Serial.print("SEQ: ");
				//Serial.println(data);
				// Serial.println(results->previous_seq);

			if (data < results->previous_seq)
			{   
				Serial.println("DROP!");
				//Serial.println(data);
				//getHermesBITS(results);
				return false;
			}

			

			results->current_seq = data;
			vld_t.cand = data;
			if 	(data == results->previous_seq) 
			{
				check_seq(results);
				break;
			}

			if (data == results->previous_seq+1)
			{
				break;
			}

			break;

			
		}   
	}
	results->listen_state = STATE_SEQ;
	return true;

}

//Read packets of 32bits and store them in the buffer until either power the transmitter
//stops or the trailer has reached
bool IRHermes::getHermesBITS(decode_results *results)
{
	//Serial.println("BITS");
	uint32_t data = 0;
	int count = 0;
	results->buffer_pos = 0;
	int16_t *pntr ;
	uint32_t pointer = 1;

	pntr = ((int16_t*) (results->rcvd_buffer));

	while(results->current_pos < ((2 * HERMES_MAX_BIT_MSG) + 4)*EXPECTED_RESULTS ) 
	{
		if (!MATCH_MARK(results->rawbuf[results->current_pos++], HERMES_BIT_MARK))  return false ;
		
		if      (MATCH_SPACE(results->rawbuf[results->current_pos], HERMES_ONE_SPACE))
		{
			data |= pointer;
			pointer <<= 1; 
			count++;
		}   
		else if (MATCH_SPACE(results->rawbuf[results->current_pos], HERMES_ZERO_SPACE))
		{
			pointer <<= 1;
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
			pointer = 1;

			//Serial.println(*pntr); pntr++;
			//Serial.println(*pntr); pntr++;
		
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

void IRHermes::check_seq(decode_results *results)
{
	// Serial.print("Backtrack: ");
	// Serial.print(results->rcvd_pos);
	// Serial.print(" -> ");
	// Serial.println(results->previous_pos);

	results->arrived = results->previous_arr;
	results->rcvd_pos = results->previous_pos;
}

