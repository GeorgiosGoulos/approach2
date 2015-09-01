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

                // if (dest==sba_system.gw_address ){ // gw_address was not used since its existence is not necessary to bridges
				// Therefore, the code should never enter the block below
				if (false) {
#ifdef VERBOSE
					cout << "Packet for gateway (MOCK)\n";
#endif // VERBOSE
                	// sba_system.gw_instance.transceiver.rx_fifo.push_back(packet); gw_address was not used since its existence is not necessary to bridges
#ifdef VERBOSE
					cout << "Packet delivered to gateway (MOCK)\n";
#endif // VERBOSE

                } else {

#ifndef BRIDGE
	/* Part of the original code */
	#ifdef VERBOSE
                	cout <<"WARNING: ad-hoc dest computation: "<<((service_id-1) % NSERVICES)+1<< "\n";
	#endif
                    ServiceAddress dest = ((service_id-1) % NSERVICES)+1;
                         //sba_system.nodes[dest]->transceiver.rx_fifo.push_back(packet);
                	// dest is generally not the same as the index in the nodes array.!
                    sba_system.nodes[dest]->transceiver->rx_fifo.push_back(packet);
#else // ifdef BRIDGE
	#ifdef VERBOSE
					/* There still is a chance that service_id will be larger than the node with the largest node_id*/
					/* Solution: use modulo, like the original GMCF code */
					cout <<"WARNING: ad-hoc dest computation (MPI): "<<((service_id-1) % (sba_system.get_size()*NSERVICES))+1<< " (original: "<<service_id<<")\n";
	#endif // VERBOSE
					ServiceAddress dest = ((service_id-1) % (sba_system.get_size()*NSERVICES))+1;
					int dest_rank = (int) (dest -1) / NSERVICES;
					if (dest_rank == sba_system.get_rank()){
	#ifdef VERBOSE
					cout << "Rank " << sba_system.get_rank() << " (transceiver): sending packet to self ("<<service_id<<")\n";						
	#endif // VERBOSE
					/* Receiver in same node -> no use of bridge required */
					sba_system.nodes[dest]->transceiver->rx_fifo.push_back(packet);
					}
					else {
	#ifdef VERBOSE
					cout << "Rank " << sba_system.get_rank() << " (transceiver): sending packet to " << dest_rank << " (" << dest << ","<<service_id<<")\n";
	#endif // VERBOSE

	/* Send the packet using a bridge */
	
	#ifdef THREADED_SEND
						/* Create a new thread that will send the message */
						sba_system.send_th(packet);
	#else // THREADED_SEND
						/* Invoke the bridge method responsible for sending the message */
						sba_system.send(packet);
	#endif // THREADED_SEND
					}
#endif // BRIDGE
                }
		}
	}
} // of transmit_packets
