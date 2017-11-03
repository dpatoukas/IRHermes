
//  __   __  _______  ______    __   __  _______  _______ 
// |  | |  ||       ||    _ |  |  |_|  ||       ||       |
// |  |_|  ||    ___||   | ||  |       ||    ___||  _____|
// |       ||   |___ |   |_||_ |       ||   |___ | |_____ 
// |       ||    ___||    __  ||       ||    ___||_____  |
// |   _   ||   |___ |   |  | || ||_|| ||   |___  _____| |
// |__| |__||_______||___|  |_||_|   |_||_______||_______|
//IRHermes Power-Intermittent communication protocol
//Version alpha 0.0.1 
//D:3/11/17
//
//Created for improving humanity
//
//By dpatoukas@gmail.com ,	carlo.delle.donne@gmail.com
//
//Heavily based on the following work
//******************************************************************************
// IRremote
// Version 2.0.1 June, 2015
// Copyright 2009 Ken Shirriff
// For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
// Edited by Mitra to add new controller SANYO
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
//******************************************************************************

#ifndef IRHermes_h
#define IRHermes_h

//------------------------------------------------------------------------------
// The ISR header contains several useful macros the user may wish to use
//
#include "IRHermesInt.h"

//------------------------------------------------------------------------------
// Supported IR protocols
// Each protocol you include costs memory and, during decode, costs time
// Disable (set to 0) all the protocols you do not need/want!
//

//------------------------------------------------------------------------------
// An enumerated list of all supported formats
// You do NOT need to remove entries from this list when disabling protocols!
//
typedef
	enum {
		UNKNOWN      = -1,
		UNUSED       =  0,
		HERMES,
	}
decode_type_t;

//------------------------------------------------------------------------------
// Debug directives
//
#if DEBUG
#	define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
#	define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#	define DBG_PRINT(...)
#	define DBG_PRINTLN(...)
#endif

//------------------------------------------------------------------------------
// Mark & Space matching functions
//
int  MATCH       (int measured, int desired) ;
int  MATCH_MARK  (int measured_ticks, int desired_us) ;
int  MATCH_SPACE (int measured_ticks, int desired_us) ;

//------------------------------------------------------------------------------
// Results returned from the decoder
//
class decode_results
{
	//TODO:check data types
	public:
		decode_type_t          decode_type;  // UNKNOWN, NEC, SONY, RC5, ...
		unsigned long          value;        // Decoded value [max 32-bits]
		int32_t				   rcvd_array[MAX_BUFFER];
		int16_t 			   rcvd_pos = 0;
		int16_t				   arrived;      //successfully arrived messages 
		int                    bits;         // Number of bits in decoded value
		volatile unsigned int  *rawbuf;      // Raw intervals in 50uS ticks
		int                    rawlen;       // Number of records in rawbuf
		int                    overflow;     // true if IR raw code too long
		int 				   listen_state; // state of listener 
		int16_t				   current_pos;  //current position in buffer to be checked
		int32_t 			   rcvd_buffer[MAX_BUFFER];
		int16_t				   buffer_pos;
}; 

//------------------------------------------------------------------------------
// Main class for receiving IR
//
class IRHermes
{
	public:
		IRHermes (int recvpin) ;
		IRHermes (int recvpin, int blinkpin);

		void  blink13      (int blinkflag);
		int   fetch		   (decode_results *results);
		int   decode       (decode_results *results);
		void  resume       ( );
		void  enableIRIn   ( );
		bool  isIdle       ( );
	private:
		bool rcvedHDR      (decode_results *results);
		bool rcvedBITS     (decode_results *results);
		bool rcvedTRL 	   (decode_results *results);
		bool getHermesHDR  (decode_results *results);
		bool getHermesBITS (decode_results *results);
		bool getHermesTRL  (decode_results *results);
		bool deliverHRM    (decode_results *results);
		void resetHRM      (decode_results *results);

#		if DECODE_HERMES
		bool  decodeHermes (decode_results *results);
#		endif
} ;

#ifdef SEND_ENABLE
class IRsendHermes
{
	public:
		IRsendHermes () { }

		void  custom_delay_usec (unsigned long uSecs);
		void  enableIROut 		(int khz) ;
		void  mark        		(unsigned int usec) ;
		void  space       		(unsigned int usec) ;
		void  sendRaw     		(const unsigned int buf[],  unsigned int len,  unsigned int hz) ;

		//......................................................................
#		if SEND_HERMES
			void  sendHermes    (unsigned long data,  int nbits) ;
#		endif
} ;
#endif
#endif
