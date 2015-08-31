#ifndef SYSTEM_H
#define SYSTEM_H

#include "Base/System.h"
#include "Tile.h"
#include <vector>
#include <unordered_map>
#include "Types.h"

#ifdef BRIDGE
#include "mpi.h"
#include "pthread.h"
#include "Bridge.h"
#endif // BRIDGE

#ifdef VERBOSE
#include <sstream>
#include <iostream>
#endif //VERBOSE

class System: public Base::System {
	public:

		/* A mapping of the Tile instances and their service number (or node_id). Taken from the original GMCF code */
		std::unordered_map<Service,Tile*> nodes;

#ifdef BRIDGE

		/* The list of Bridge instances that can be used for communication between processes */
		std::vector<Bridge*> bridge_list; 

		/* A spinlock used for safely selecting a bridge from bridge_list */
		pthread_spinlock_t bridge_selector_lock;

		/* A spinlock used for updating the number of receiving threads that were killed. It is used in the destructor */ 
		pthread_spinlock_t killed_threads_lock; 

	#ifdef EVALUATE
		
			/* indicates whether a test is underway */
			bool testing_status;
	#endif // EVALUATE

	#ifdef EVALUATE
		/* double indicating the time elapsed since an arbitrary point in the past. Used for calculating the time it takes for a 
		 * send() operation */
		double start_time; //TODO: change name?

		double total_time;

		#ifdef THREADED_SEND
		/* Used for measuring the time it took to start the sending thread */
		double send_thread_start;
		#else
		/* Used for measuring the time it took to invoke the Bridge::send() method */
		double send_fcn_start;
		#endif // THREADED_SEND

	#endif // EVALUATE

	#ifdef VERBOSE
		/* used for printing messages */
		stringstream ss; 
	#endif // VERBOSE






#endif //BRIDGE
		
#ifdef BRIDGE
		
		/* Constructor used when BRIDGE is defined */
		System(int r_, int c_): rows(r_), cols(c_), bridge_pos(-1), active(true), killed_threads(0){

	#ifdef EVALUATE
			
			/* No test is underway */
			testing_status = false;
			start_time = 0;
			total_time = 0;
	#endif // EVALUATE

			/* Thread support level */
			int tsl;

			/* Initialise MPI for multithreaded use*/
			MPI_Init_thread(nullptr, nullptr, MPI_THREAD_MULTIPLE, &tsl);	

			/* Create a new communicator if MPI_TOPOLOGY_OPT was defined, or MPI_COMM_WORLD otherwise */
#ifdef MPI_TOPOLOGY_OPT
		
			/* Create a new (possibly optimised) communicator */
			comm = create_communicator(rows, cols); 
#else // MPI_TOPOLOGY_OPT

			/* Use the predefined communicator */
			comm = MPI_COMM_WORLD; 
#endif // MPI_TOPOLOGY_OPT

			/* Detect the MPI rank of the process */
			MPI_Comm_rank(comm, &rank);
			
			/* Detect the number of running processes */
			MPI_Comm_size(comm, &size);

#ifdef VERBOSE
	#ifdef MPI_TOPOLOGY_OPT
			if (rank == 0 ) {
				cout << "Using optimised communicator (Cartesian topology)\n";
			}	
	#else // MPI_TOPOLOGY_OPT
			if (rank == 0 ) {
				cout << "Using standard communicator\n";
			}
	#endif // MPI_TOPOLOGY_OPT
#endif // VERBOSE


			/* Check the MPI version used */
#ifdef VERBOSE
			if (rank==0) {
				ss.str("");
				ss << "MPI version: ";
	#ifdef MPI_VERSION // It covers versions before 1.3 (1.2, 1.1, 1.0)
				ss << MPI_VERSION << "." << MPI_SUBVERSION << ". ";
		#if MPI_VERSION>=3
				ss << "MPI_Improbe() and MPI_Imrecv() will be used\n";
		#else // MPI_VERSION<3
				ss << "MPI_Iprobe() and MPI_Irecv() will be used\n";
		#endif // MPI_VERSION==0
	#else // MPI_VERSION
				ss << "Could not be detected (MPI version must be <1.3). sychronised MPI_Iprobe() and MPI_Irecv() will be used\n";
	#endif // MPI_VERSION
				cout << ss.str();
			}
#endif // VERBOSE


/* Print the thread suppport level for each function (MPI does not ensure it will be the same for all processes) */
#ifdef VERBOSE
			// Detect thread support level 
			ss.str("");
			ss << "Rank " << rank << ": Thread support level is ";
			switch (tsl){
				case MPI_THREAD_SINGLE: // A single thread will execute
					ss << "MPI_THREAD_SINGLE\n";
					break;
				case MPI_THREAD_FUNNELED: // If multiple threads exist, only the one that called MPI_Init() will be able to make MPI calls
					ss << "MPI_THREAD_FUNNELED\n";
					break;
				case MPI_THREAD_SERIALIZED: // If multiple threads exist, only one will be able to make MPI library calls at a time
					ss << "MPI_THREAD_SERIALIZED\n";
					break;
				case MPI_THREAD_MULTIPLE: // No restrictions on threads and MPI library calls (WARNING: "lightly tested")
					ss << "MPI_THREAD_MULTIPLE\n";
					break;
				default:
					ss << "???\n"; // Unidentified thread support level, this should never be printed
					break;
			}
			cout << ss.str();
#endif //VERBOSE

			/*
			 * Create a 2D table of the MPI processes. The table is stored in process_tbl
			 */
			initialise_process_table(); 

			/*
			 * Find the processes that surround the current MPI node in the aforementioned table. They are stored in neighbours.
			 * This method can be used for a 2D topology ONLY, modifications are required for other dimensions
			 */
			find_neighbours(); 

			/* Create the Tile instances */
			for (Service node_id_ = rank*NSERVICES + 1; node_id_ <= (Service) (rank+1)*NSERVICES; node_id_++) {
				ServiceAddress service_address=node_id_;
				Service node_id = node_id_;
				if  (service_address != 0) {
					
					if (nodes.count(node_id) == 0) {
	#ifdef VERBOSE
						ss.str("");
						ss << "Rank " << rank << ": Adding tile with node_id " << (int) node_id << "\n";
						cout << ss.str();
	#endif // VERBOSE
						nodes[node_id]=new Tile(this, node_id, service_address, rank);
					}
				}
			}

			/* Create the bridges */
			for (int i = 0; i < NBRIDGES; i++){
				bridge_list.push_back(new Bridge(this, rank));
			}

			/* Initialise the spinlocks */
			pthread_spin_init(&bridge_selector_lock, PTHREAD_PROCESS_SHARED); 
			pthread_spin_init(&killed_threads_lock, PTHREAD_PROCESS_SHARED); 

		};
#endif // BRIDGE // should add a #else here when integrated in the GMCF code so that the original constructors are used #ifndef BRIDGE

		/** 
		 * Destructor for the System class
		 */
		~System(){
		
#ifdef BRIDGE		
			/* System instance is terminating. Receiving threads check the active should now terminate as well */
			active = false;
	#ifdef VERBOSE
			ss.str("");
			ss << "Rank " << rank << ": Receiving thread(s) will now terminate...\n";
			cout << ss.str();
	#endif // VERBOSE
			/* wait for threads to terminate */
			while ((unsigned int) killed_threads < bridge_list.size()) {} 
	#ifdef VERBOSE
			ss.str("");
			ss << "Rank " << rank << ": killed all " << bridge_list.size() << " receiving thread(s)...\n";
			cout << ss.str();
	#endif // VERBOSE
			pthread_spin_destroy(&killed_threads_lock);
			pthread_spin_destroy(&bridge_selector_lock); 
			for (Bridge *bridge_ptr: bridge_list){
				delete bridge_ptr;
			}
	
	/* Finalise MPI. No more MPI routines should be executed after this point */
	MPI_Finalize();
	#ifdef VERBOSE
			ss.str("");
			ss << "Rank " << rank << ": MPI_Finalize() was executed successfully...\n";
			cout << ss.str();
	#endif // VERBOSE
#endif // BRIDGE
		}

#ifdef BRIDGE

		/**
		 * Find the processes that surround the current MPI node in the process table 
		 */
		void find_neighbours(); 

	#ifdef VERBOSE

		/**
		 * Prints a table representing the logical topology
		 */
		void print_process_table();

		/**
		 * Prints the neighbouring ranks of the current process on the process table
		 */
		void print_neighbours();
	#endif //VERBOSE

		/**
		 * Returns the neighbouring processes of the current process on the process table
		 * @return the contents of neighbours
		 */
		std::vector<int> get_neighbours();

		/** 
		 * Returns the rank of the current MPI process
		 * @return the value of rank
		 */
		int get_rank();
		
		/**
		 * Returns the number of MPI processes created
		 * @return the value of size
		 */
		int get_size();

		/**
		 * Selects a bridge and sends a packet to another process (threaded)
		 * @param packet the GMCF packet to be sent
		 * @param tag The tag of the MPI message to be sent
		 */
		void send_th(Packet_t packet, int tag=tag_default);

		/**
		 * Selects a bridge and sends a packet to another process (not threaded)
		 * @param packet the GMCF packet to be sent
		 * @param tag The tag of the MPI message to be sent
		 */
		void send(Packet_t packet, int tag=tag_default);


		//void stencil_operation(std::vector<Packet_t> packet_list); // TODO: Remove?
		//void neighboursreduce_operation(std::vector<Packet_t> packet_list); //TODO: Remove?

		/**
		 * Returns the status of the System, which is true until the destructor of the System instance is called
		 * @return the value of active
		 */
		bool is_active();

		/**
		 * Thread-safe way of incrementing killed_threads, called when a receiving thread is about to exit 
		 */
		void kill_thread(); 

	#ifdef MPI_TOPOLOGY_OPT
		/**
		 * Creates a new (possibly optimised) communicator that handles the logical topology as a torus architecture
		 * @param rows the number of rows in the logical topology to be created
		 * @param cols the number of columns in the logical topology to be created
		 * @return the communicator
		 */
		MPI_Comm create_communicator(int rows, int cols);
	#endif // MPI_TOPOLOGY_OPT

		/**
		 * Returns a pointer to the communicator
		 * @return the communicator pointer
		 */
		MPI_Comm* get_comm_ptr();

	private:

		/* The communicator to be used for inter-process communication */
		MPI_Comm comm; 

		/* Dimensions of process_tbl */
		int rows, cols; 

		/* Indicates the index of the next bridge to be used, cycles from 0 to bridge_list.size()-1 */
		int bridge_pos; 

		/* Indicates whether the System instance is active or not, used for killing receiving threads created by Bridge */
		bool active; 

		/* Indicates the number of threads killed during the destruction process, used to ensure all receivers have been killed */
		int killed_threads; 

		/* A vector of vectors representing a table of MPI processes TODO: move to find_neighbours after deployment */
		std::vector< std::vector<int> > process_tbl; 

		/* The neighbouring MPI processes of the current process */
		std::vector<int> neighbours; 

		/* MPI rank of current process */
		int rank;

		/* number of active MPI processes */
		int size;
		
		/** 
		 * Creates a 2D table of the MPI processes, which is stored in process_tbl
		 */
		void initialise_process_table(); 

		/**
		 * Increments the member variable that indicates the index of the next bridge to be used 
		 */
		void increment_bridge_pos();


#endif // BRIDGE

};

#endif // SYSTEM_H
