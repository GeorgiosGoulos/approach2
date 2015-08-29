#ifndef PACKET_H
#define PACKET_H

#include "Types.h"
#include "ServiceConfiguration.h"

using namespace std;
using namespace SBA;

namespace SBA {

	typedef unsigned int uint;

	typedef u_int64_t Uint64;
	typedef uint8 Packet_type_t; //3
	typedef uint16 Length_t; //16
	typedef uint8 Ctrl_t; //2
	typedef uint8 Redir_t; //3
	typedef uint16 To_t; //16 //ORIGINAL
	typedef uint16 Return_to_t; //16 //ORIGINAL

	Packet_t packet_pointer_int(Packet_t packet);

	Header_t mkHeader(Word,Word,Word); // Actually it's mkHeader(Word,Word,Word,Word,Word,Word,Word,Word)
	Header_t mkHeader(Word,Word,Word,Word,Word,Word,Word,Word);
	Packet_t mkPacket(Header_t&,Payload_t&);
	Packet_t mkPacket_new(Header_t&,Word);

	To_t getTo(const Header_t&);
	Return_to_t getReturn_to(const Header_t&);
	Packet_type_t getType(Header_t header);
	Header_t getHeader(Packet_t packet);
	Length_t getLength(Header_t header);
	Word getReturn_as(Header_t header); 
	Ctrl_t getCtrl(Header_t&); // Existing
	Redir_t getRedir(Header_t&); // Existing
	Packet_type_t getPacket_type(const Header_t&); // Existing
	Packet_t setHeader(Packet_t&,Header_t&); // Existing
	Payload_t getPayload(Packet_t); // Existing


} // namespace SBA


#endif // #ifndef PACKET_H
