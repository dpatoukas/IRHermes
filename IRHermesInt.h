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
#include "init.h"
#ifndef IRHermesint_h
#define IRHermesint_h


//------------------------------------------------------------------------------
// Include the right Arduino header
//
#if defined(ARDUINO) && (ARDUINO >= 100)
#	include <Arduino.h>
#else
#	if !defined(IRPRONTO)
#		include <WProgram.h>
#	endif
#endif

//------------------------------------------------------------------------------
// This handles definition and access to global variables
//
#ifdef IR_GLOBAL
#	define EXTERN
#else
#	define EXTERN extern
#endif

//------------------------------------------------------------------------------
// Information for the Interrupt Service Routine
//

typedef
	struct {
		// The fields are ordered to reduce memory over caused by struct-padding
		uint8_t       rcvstate;        // State Machine state
		uint8_t       recvpin;         // Pin connected to IR data from detector
		uint8_t       blinkpin;
		uint8_t       blinkflag;       // true -> enable blinking of pin on IR processing
		uint8_t       rawlen;          // counter of entries in rawbuf
		unsigned int  timer;           // State timer, counts 50uS ticks.
		unsigned int  rawbuf[RAWBUF];  // raw data
		uint8_t       overflow;        // Raw buffer overflow occurred
	}
irparams_t;

// ISR State-Machine : Receiver States
#define STATE_IDLE      2
#define STATE_MARK      3
#define STATE_SPACE     4
#define STATE_STOP      5
#define STATE_OVERFLOW  6

// Parsing State-Machine : Receiver States
#define STATE_WAITING   2
#define STATE_HDR      	3
#define STATE_BIT     	4
#define STATE_TRAIL     5
#define STATE_SAVE	  	6
#define STATE_OVER      7

// Allow all parts of the code access to the ISR data
// NB. The data can be changed by the ISR at any time, even mid-function
// Therefore we declare it as "volatile" to stop the compiler/CPU caching it
EXTERN  volatile irparams_t  irparams;

//------------------------------------------------------------------------------
// Defines for setting and clearing register bits
//
#ifndef cbi
#	define cbi(sfr, bit)  (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#	define sbi(sfr, bit)  (_SFR_BYTE(sfr) |= _BV(bit))
#endif

//------------------------------------------------------------------------------
// Pulse parms are ((X*50)-100) for the Mark and ((X*50)+100) for the Space.
// First MARK is the one after the long gap
// Pulse parameters in uSec
//

// Due to sensor lag, when received, Marks  tend to be 100us too long and
//                                   Spaces tend to be 100us too short
#define MARK_EXCESS    100

// Upper and Lower percentage tolerances in measurements
#define TOLERANCE       25
#define LTOL            (1.0 - (TOLERANCE/100.))
#define UTOL            (1.0 + (TOLERANCE/100.))

// Minimum gap between IR transmissions
#define _GAP            5000
#define GAP_TICKS       (_GAP/USECPERTICK)

#define TICKS_LOW(us)   ((int)(((us)*LTOL/USECPERTICK)))
#define TICKS_HIGH(us)  ((int)(((us)*UTOL/USECPERTICK + 1)))

//------------------------------------------------------------------------------
// IR detector output is active low

#define MARK   0
#define SPACE  1

// All board specific stuff has been moved to its own file, included here.
#include "boarddefs.h"

#endif
