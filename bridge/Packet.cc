#include "Packet.h"
#include "ServiceConfiguration.h"
#include "Types.h"
#include <cstdio>

using namespace SBA;


//Taken from the original GMCF code, returns the header of a packet
Header_t SBA::getHeader(Packet_t packet) {
	Header_t header;
	header.push_back(packet.at(0));
	header.push_back(packet.at(1));
	header.push_back(packet.at(2));
	return  header;
}

// Taken from the original GMCF code, returns the node_id of the receiver of the packet
To_t SBA::getTo(const Header_t& header) {
	Word w1=header[0];
	return (w1 & F_To) >> FS_To;
}


// Taken from the original GMCF code, returns the type of the packet (e.g. DRESP)
Packet_type_t SBA::getPacket_type(const Header_t& header) {
	Word w1=header[0];
	return (w1 & F_Packet_type) >> FS_Packet_type;
}


// Taken from the original GMCF code, returns the node_id of the sender of the packet
Return_to_t SBA::getReturn_to(const Header_t& header) {
	Word w1=header[0];
	return (w1 & F_Return_to) >> FS_Return_to;
}

// Taken from the original GMCF code, returns the size of the data array (for D_RESP packets only)
Word SBA::getReturn_as(const Header_t header) { 
	return header[2];
}

// Taken from the original GMCF code, returns a new header
Header_t SBA::mkHeader(Word packet_type,Word prio,Word redir,Word length,Word to,Word return_to,Word ack_to,Word return_as) {
	Word wpacket_type=(packet_type << FS_Packet_type) & F_Packet_type;
	Word wprio=(prio << FS_Ctrl) & F_Ctrl;
	Word wredir=(redir << FS_Redir) & F_Redir;
	Word wlength=(length << FS_Length) & F_Length;
	Word wto=(to << FS_To) & F_To;
	Word wreturn_to=(return_to << FS_Return_to) & F_Return_to;

	Word w1=wpacket_type+wprio+wredir+wlength+wto+wreturn_to;
	Header_t wl;
	wl.push_back(w1);
	wl.push_back(ack_to);
	wl.push_back(return_as);
	return wl;
}

// Taken from the original GMCF code, creates a packet given a header and a payload
Packet_t SBA::mkPacket(Header_t& header,Payload_t& payload) {
	Packet_t packet;
	for(uint i=0;i<=HEADER_SZ-1 ;i++) {
		packet.push_back(header[i]);
	}
	for(auto iter_=payload.begin();iter_!=payload.end();iter_++) {
		Word w=*iter_;
		packet.push_back(w);
	}
	return packet;
}

// Taken from the original GMCF code, returns a packet with a new header
Packet_t SBA::setHeader(Packet_t& packet,Header_t& header) {
	Packet_t npacket;
	for(auto iter_=header.begin();iter_!=header.end();iter_++) {
		Word w=*iter_;
		npacket.push_back(w);
	}
	Payload_t payload=getPayload(packet);
	for(auto iter_=payload.begin();iter_!=payload.end();iter_++) {
		Word w=*iter_;
		npacket.push_back(w);
	}
	return npacket;
}

// Taken from the original GMCF code, returns the payload of a packet
Payload_t SBA::getPayload(Packet_t packet) {
	Payload_t pl;
	uint i=0;
	for(auto iter_=packet.begin();iter_!=packet.end();iter_++) {
		Word w=*iter_;
		if (i>=3) {pl.push_back(w);}
			i+=1;
	}
	return pl;
}


