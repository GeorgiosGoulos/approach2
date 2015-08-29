#ifndef SERVICE_MGR_CONF_H_
#define SERVICE_MGR_CONF_H_

///#include "Base/Types.h" //
//#include "./Base/Types.h" // WHY?

//using namespace SBA;

namespace SBA {

	#define HEADER_SZ 3

	// For compatibility with streaming data, we either add a Mode field (stream or 'datagram') or we use a bit in the Type field
	// We use the MSB to indicate the mode.
	enum Packet_Type { // >7, so needs at least 4 bits. We have 8 bits.
		    P_error=0, // Payload could be error message. In general not Symbols
		    P_subtask=1, // list of Symbols. At least 2 elts: (S ...); Can be stream
		    P_code=2, // list of Symbols; Can be stream
		    P_reference=3, // 1-elt list of Symbols; Can be stream
		    P_request=4, // 1-elt list of Symbols; Can be stream
		    P_data=5, // preferred; Can be stream
		    P_DREQ=6, // list of Symbols. Usually 1-elt
		    P_TREQ=7,
		    P_DRESP=8, // "what is the address of service n?", with n a number => 1-elt list of uint64
		    P_TRESP=9, //
		    P_FIN=10, // "my address is n", with n a number => 1-elt list of uint64
		    P_DACK=11, // data ack, to signal it's OK to de-allocate memory
		    P_RRDY=12 // register ready for access by another thread
		};

// For Packet header
// Header Word1: 8 | 5|3 | 16 | 16 | 16
const Word F_Packet_type=0xFF00000000000000ULL;
const Word F_Ctrl=0x00F8000000000000ULL;
const Word F_Redir=0x0007000000000000ULL;
const Word F_Length=0x0000FFFF00000000ULL;
const Word F_To=0x00000000FFFF0000ULL; 
const Word F_Return_to=0x00000000000FFFFULL; 
const Word F_Send_to[4]={0x000000000000FFFFULL,0x00000000FFFF0000ULL,0x0000FFFF00000000ULL,0xFFFF000000000000ULL};

// Reverse masks
const Word FN_Packet_type=0x00FFFFFFFFFFFFFFULL;
const Word FN_Ctrl=0xFF07FFFFFFFFFFFFULL;
const Word FN_Redir=0xFFF8FFFFFFFFFFFFULL;
const Word FN_Length=0xFFFF0000FFFFFFFFULL;
const Word FN_To=0xFFFFFFFF0000FFFFULL; 
const Word FN_Return_to=0xFFFFFFFFFFFF0000ULL; 
const Word FN_Send_to[4]={0xFFFFFFFFFFFF0000ULL,0xFFFFFFFF0000FFFFULL,0xFFFF0000FFFFFFFFULL,0x0000FFFFFFFFFFFFULL};

// Field width masks
const Word FW_Packet_type=0xFFULL;
const Word FW_Ctrl=0xF8ULL;
const Word FW_Redir=0x07ULL;
const Word FW_Length=0xFFFFULL;
const Word FW_To=0xFFFFULL; 
const Word FW_Return_to=0xFFFFULL;

// Field width as number of bits
const Word FB_Packet_type=8;
const Word FB_Ctrl=5;
const Word FB_Redir=3;
const Word FB_Length=16;
const Word FB_To=16;
const Word FB_Return_to=16;

// Shifts
const Word FS_Packet_type=56;
const Word FS_Ctrl=51;
const Word FS_Redir=48;
const Word FS_Length=32;
const Word FS_To=16;
const Word FS_Return_to=0;
const Word FS_Send_to[4]={0ULL,16ULL,32ULL,48ULL};

}

#endif // SERVICE_MGR_CONF_H_
