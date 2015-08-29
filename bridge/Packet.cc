#include "Packet.h"
#include "ServiceConfiguration.h"
#include "Types.h"
#include <cstdio>

using namespace SBA;

//TODO: Delete or Modify (already exists)
Packet_type_t SBA::getType(Header_t header){
	return (Packet_type_t) header.at(0);
}

//TODO: Delete (already exists)
Header_t SBA::getHeader(Packet_t packet) {
	Header_t header;
	header.push_back(packet.at(0));
	header.push_back(packet.at(1));
	header.push_back(packet.at(2));
	return  header;
}

//TODO: Modify (gets the Service number from the 1st word in the header)
To_t SBA::getTo(const Header_t& header) {
	Word w1=header[0];
	return (w1 & F_To) >> FS_To;
	//printf("Default service selected \n");
}

Packet_type_t SBA::getPacket_type(const Header_t& header) {
	Word w1=header[0];
	return (w1 & F_Packet_type) >> FS_Packet_type;
}



Return_to_t SBA::getReturn_to(const Header_t& header) {
	Word w1=header[0];
	return (w1 & F_Return_to) >> FS_Return_to;
}


//TODO: Delete or Modify (already exists)
Length_t SBA::getLength(Header_t header){
	//return (Length_t) header.at(1); // 2nd word in Header contains the length
	Word w1=header[0];
	return (w1 & F_Length) >> FS_Length;
}

Ctrl_t SBA::getCtrl(Header_t& header) { //wprio in mkHeader()
	Word w1=header[0];
	return (w1 & F_Ctrl) >> FS_Ctrl;
}

Redir_t SBA::getRedir(Header_t& header) {
	Word w1=header[0];
	return (w1 & F_Redir) >> FS_Redir;
}

//TODO: Delete (already exists)
Word SBA::getReturn_as(const Header_t header) { // Returns the size of the data array (for D_RESP only)
	return header[2];
}

//TODO: Delete or Modify (already exists)
Header_t SBA::mkHeader(Word packet_type, Word length, Word return_as) { //return_as: size of data array
	Header_t wl;
	wl.push_back(packet_type);
	wl.push_back(length);
	wl.push_back(return_as);
	return wl;
}

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

Packet_t SBA::mkPacket_new(Header_t& header,Word payload) {
	Packet_t packet;
	for(uint i=0;i<=HEADER_SZ-1 ;i++) {
		packet.push_back(header[i]);
	}
	packet.push_back(payload);
	return packet;
}


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


