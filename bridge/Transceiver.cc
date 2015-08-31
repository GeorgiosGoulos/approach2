#include "Transceiver.h"
#include "Packet.h"
#include "System.h"

void Transceiver::transmit_packets()  {
    System& sba_system=*((System*)sba_system_ptr);
#ifdef VERBOSE
         cout << service <<" TRX:transmit_packets(): FIFO length: "<< tx_fifo.length() << endl;
#endif // VERBOSE
		if (tx_fifo.status()==1 ){
            while (tx_fifo.status()==1){
                Packet_t packet= tx_fifo.front();tx_fifo.pop_front();
                Service service_id= getTo(getHeader(packet));
				//ServiceAddress dest=service_id; // gw_address: not in this implementation
#ifdef VERBOSE
                // cout << "PACKET:\n" << ppPacket(packet) << "\n"; //TODO: Uncomment for final version
                //cout << "service id: " <<service_id<< "; dest addr: " <<dest<< ""<<endl;// Ashkan_debug //TODO: Uncomment for final version
#endif // VERBOSE

                // if (dest==sba_system.gw_address ){ // gw_address: not in this implementation
				if (false) {
#ifdef VERBOSE
					cout << "Packet for gateway (MOCK)\n";
#endif // VERBOSE
                	// sba_system.gw_instance.transceiver.rx_fifo.push_back(packet); // gw_address: not in this implementation
#ifdef VERBOSE
					cout << "Packet delivered to gateway (MOCK)\n";
#endif // VERBOSE

                } else {
/*
    It is possible that the service_id is higher than the highest node id.
    In that case we remap using modulo. This requires that node ids are contiguous!    
*/

#ifndef BRIDGE
	#ifdef VERBOSE
                	cout <<"WARNING: ad-hoc dest computation: "<<((service_id-1) % NSERVICES)+1<< "\n";
	#endif
                    ServiceAddress dest = ((service_id-1) % NSERVICES)+1;
                         //sba_system.nodes[dest]->transceiver.rx_fifo.push_back(packet);
                	// dest is generally not the same as the index in the nodes array.!
                    sba_system.nodes[dest]->transceiver->rx_fifo.push_back(packet);
#else // ifdef BRIDGE
					cout <<"WARNING: ad-hoc dest computation (MPI): "<<((service_id-1) % (sba_system.get_size()*NSERVICES))+1<< " (from "<<service_id<<")\n";
					ServiceAddress dest = ((service_id-1) % (sba_system.get_size()*NSERVICES))+1;
					int dest_rank = (int) (dest -1) / NSERVICES;
					if (dest_rank == sba_system.get_rank()){
	#ifdef VERBOSE
					cout << "Rank " << sba_system.get_rank() << " (transc): sending packet to self ("<<service_id<<")\n";						
	#endif // VERBOSE
					sba_system.nodes[dest]->transceiver->rx_fifo.push_back(packet);
					}
					else {
	#ifdef VERBOSE
					cout << "Rank " << sba_system.get_rank() << " (transc): sending packet to " << dest_rank << " (" << dest << ","<<service_id<<")\n";
	#endif // VERBOSE

						//sba_system.send_th(packet);
						sba_system.send(packet);
					}
#endif // BRIDGE
                }
		}
	}
} // of transmit_packets
