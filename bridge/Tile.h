#ifndef TILE_H
#define TILE_H

#include "Base/System.h"
#include "Base/Tile.h"
#include "Bridge.h"
#include "Types.h"
#include "Transceiver.h"

class Tile: public Base::Tile {
	public:
		
		Base::System *sba_system_ptr;
		Service service;
		ServiceAddress service_address;
		int rank;
		Transceiver *transceiver;

		Tile(Base::System *sba_s_, int n_, int s_, int r_): sba_system_ptr(sba_s_), service(n_), service_address(s_), rank(r_){

			transceiver = new Transceiver(sba_system_ptr, this, service, service_address);
		};

		~Tile() {
			delete transceiver;
		}

};

#endif // TILE_H
