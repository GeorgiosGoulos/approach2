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

	/**
	 * Taken from the original GMCF code, returns a new header
	 * @param words that are used for constructing the packet
	 * @return the header of the packet
	 */
	Header_t mkHeader(Word,Word,Word,Word,Word,Word,Word,Word);

	/**
	 * Taken from the original GMCF code, creates a packet given a header and a payload
	 * @param header the header
	 * @param payload the payload
	 * @return the newly created packet
	 */
	Packet_t mkPacket(Header_t& header, Payload_t& packet);

	/**
	 * Taken from the original GMCF code, returns the node_id of the receiver of the packet
	 * @param header the packet header
	 * @return the node_id of the receiver of the packet
	 */
	To_t getTo(const Header_t&);

	/**
	 * Taken from the original GMCF code, returns the node_id of the sender of the packet
	 * @param header the packet header
	 * @return the node_id of the sender of the packet
	 */
	Return_to_t getReturn_to(const Header_t&);

	/**
	 * Taken from the original GMCF code, returns the header of a packet
	 * @param packet the packet
	 * @return the header of the packet
	 */
	Header_t getHeader(Packet_t packet);

	/**
	 * Taken from the original GMCF code, returns the size of the data array (for D_RESP packets only)
	 * @param header the packet header
	 * @return the size of the data array
	 */
	Word getReturn_as(Header_t header); 


	/**
	 * Taken from the original GMCF code, returns the type of the packet (e.g. DRESP)
	 * @param header the packet header
	 * @return the type of the packet
	 */
	Packet_type_t getPacket_type(const Header_t& header); // Existing

	/**
	 * Taken from the original GMCF code, returns a packet with a new header
	 * @param packet the packet
	 * @param header the new header
	 * @return the new packet
	 */
	Packet_t setHeader(Packet_t& packet,Header_t& header); // Existing

	/**
	 * Taken from the original GMCF code, returns the payload of a packet
	 * @param packet the packet
	 * @return the payload of the packet
	 */
	Payload_t getPayload(Packet_t packet); 


} // namespace SBA


#endif // #ifndef PACKET_H
