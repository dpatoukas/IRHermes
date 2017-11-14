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

//Number of messages for complete transmission 
#define EXPECTED_DATA 128

#define EXPECTED_MSG 1
#define EXPECTED_RESULTS 8 //In each message

//Size of minimum useful information packet
#define PACKET_SIZE 32

//Buffer to hold and collect the received information
#define MAX_BUFFER EXPECTED_DATA //EXPECTED_RESULTS*EXPECTED_MSG   //CAUTION:memory consumption
#define MAX_CHUNK_BUFFER 8
//Buffer on the receiving ISR state machine
//#define RAWBUF  250  // Maximum length of raw duration buffer
#define RAWBUF 520

//Sending and receiving behavior

#define DECODE_HERMES		 1
#define SEND_ENABLE 		 1
#define SEND_HERMES 		 1

//------------------------------------------------------------------------------
// Set DEBUG to 1 for lots of lovely debug output
//
#define DEBUG 0
#undef DEBUG