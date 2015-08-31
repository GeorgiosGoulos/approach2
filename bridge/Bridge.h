#ifndef BRIDGE_H
#define BRIDGE_H

#include "Base/System.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <pthread.h>
#include "Types.h"
#include "Packet.h"

//#ifdef VERBOSE
#include <sstream>
//#endif // VERBOSE

/** 
 * The function executed by a different thread, used for listening for incoming MPI messages
 * The thread is created in the constructor of Bridge
 * @param args a void pointer that points to the Bridge instance which created the thread
 */
void *wait_recv_any_th(void* args);

class Bridge {
	public:

		/* A pointer to the System instance that created this Bridge */
		Base::System* sba_system_ptr;

		/* A vector containing all the logical neighbours of this MPI node */
		std::vector<int> neighbours;

		/* The MPI rank of this process */
		int rank;

		/* Handle to the thread that will listen for incoming MPI messages */
		pthread_t recv_thread;

#if  MPI_VERSION<3
		/* A lock used to ensure a receiving threads that probes a packet also receives it*/
		pthread_spinlock_t recv_lock;
#endif // MPI_VERSION<3

		/* Stream used for printing messages */
		stringstream ss;




		/** constructor */
		Bridge(Base::System *sba_s_, int r_): sba_system_ptr(sba_s_), rank(r_){	
			neighbours = sba_system_ptr->get_neighbours();		

#if MPI_VERSION<3
			/* Initialise the spinlock */
			pthread_spin_init(&recv_lock, PTHREAD_PROCESS_SHARED); 
#endif // MPI_VERSION<3

			/* create receiving thread */
			int rc = pthread_create(&recv_thread, NULL, wait_recv_any_th, (void *) this);
			if (rc) {
				ss.str("");
				ss << "Rank " << rank << ": Error during creation of receiving thread: Code " << rc << "\n";
				cout << ss.str();
				exit(1);
			}

#ifdef VERBOSE
			ss.str("");
			ss << "Rank " << rank << ": Creation of receiving thread was completed successfully.\n";
			cout << ss.str();
#endif // VERBOSE
		};

		~Bridge(){
#if MPI_VERSION<3
			pthread_spin_destroy(&recv_lock);		
#endif // MPI_VERSION<3
		}
		

		/**
		 * Returns the MPI neighbours of the current MPI process in  System::process_tbl
		 */
		std::vector<int> get_neighbours();

		/**
		 * Used for sending GPRM packets through MPI messages to other MPI processes (threaded)
		 * @param packet A GPRM packet
		 * @param tag The tag of the MPI message to be sent
		 */
		void send_th(Packet_t packet, int tag=tag_default);

		/**
		 * Used for sending GPRM packets through MPI messages to other MPI processes (not threaded)
		 * @param packet A GPRM packet
		 * @param tag The tag of the MPI message to be sent
		 */
		void send(Packet_t packet, int tag=tag_default);
		
		/**
		 * Initiates a stencil operation
		 * @param packet_list A vector containing GPRM packets to be scattered among the neighbours
		 */
		void stencil(std::vector<Packet_t> packet_list);


};

#endif // BRIDGE_H
