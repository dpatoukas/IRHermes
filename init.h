

//Number of messages for complete transmission 
#define EXPECTED_MSG 5

//Size of minimum useful information packet
#define PACKET_SIZE 32

//Buffer to hold and collect the received information
#define MAX_BUFFER EXPECTED_MSG   //CAUTION:memory consumption

//Buffer on the receiving ISR state machine
#define RAWBUF  128  // Maximum length of raw duration buffer


//Sending and receiving behavior

#define DECODE_HERMES		 1
#define SEND_ENABLE 		 1
#define SEND_HERMES 		 1