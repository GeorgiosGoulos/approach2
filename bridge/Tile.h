#ifndef TILE_H
#define TILE_H

#include "Base/System.h"
#include "Base/Tile.h"
#include "Bridge.h"
#include "Types.h"
#include "Transceiver.h"

//TODO: Remove
#include <pthread.h>

class Tile: public Base::Tile {
	public:
		
		Base::System *sba_system_ptr;
		Service service;
		ServiceAddress service_address;
		int rank;
		Transceiver *transceiver;

		//TODO:Remove
		pthread_spinlock_t _lock;

		Tile(Base::System *sba_s_, int n_, int s_, int r_): sba_system_ptr(sba_s_), service(n_), service_address(s_), rank(r_){

			transceiver = new Transceiver(sba_system_ptr, this, service, service_address);

			//TODO: Remove
			pthread_spin_init(&_lock, PTHREAD_PROCESS_SHARED);
		};

		~Tile() {
			//TODO: Remove
			pthread_spin_destroy(&_lock);

			//TODO: Required?
			//delete transceiver;
		}


		void add_to_tx_fifo(Packet_t packet); // GPRM already uses spinlock for this
		void add_to_rx_fifo(Packet_t packet); // No thread safety(probably because it's one thread that reads/modifies it)
											  //TODO: Should thread safety be added now?
};

#endif // TILE_H
