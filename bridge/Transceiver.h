#ifndef TRANSCEIVER_H
#define TRANSCEIVER_H

#include "Base/System.h"
#include "Base/Tile.h"
#include "Types.h"

class Transceiver {
	public:

		Base::System* sba_system_ptr;
		Base::Tile* sba_tile_ptr;
		Service service;
		ServiceAddress address;

		RX_Packet_Fifo rx_fifo;
		TX_Packet_Fifo tx_fifo;

		Transceiver(Base::System *sba_s_, Base::Tile *sba_t_, Service s_, ServiceAddress addr_):
			sba_system_ptr(sba_s_), sba_tile_ptr(sba_t_), service(s_), address(addr_) {};

		void transmit_packets();

};

#endif //TRANSCEIVER_H_
