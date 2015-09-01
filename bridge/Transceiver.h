#ifndef TRANSCEIVER_H
#define TRANSCEIVER_H

#include "Base/System.h"
#include "Base/Tile.h"
#include "Types.h"

/* Taken from the original GMCF code */

class Transceiver {
	public:

		Base::System* sba_system_ptr;
		Base::Tile* sba_tile_ptr;
		Service service;
		ServiceAddress address;

		/* RX FIFO, stores received messages */
		RX_Packet_Fifo rx_fifo;

		/* TX FIFO, stores messages to be transmitted */
		TX_Packet_Fifo tx_fifo;

		Transceiver(Base::System *sba_s_, Base::Tile *sba_t_, Service s_, ServiceAddress addr_):
			sba_system_ptr(sba_s_), sba_tile_ptr(sba_t_), service(s_), address(addr_) {};

		/** Transmits all the packets in the TX FIFO
		 */
		void transmit_packets();

};

#endif //TRANSCEIVER_H_
