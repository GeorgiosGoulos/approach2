#include <iostream>
#include <cstdio>
#include "Bridge.h"
#include "mpi.h"
#include <vector>
#include <algorithm> //find()
#include <pthread.h>
#include "Packet.h"
#include "Base/System.h"
#include "System.h"

using namespace std;
using namespace SBA;

/**
 * Used for passing parameters to threads 
 */
struct bridge_packets{ 
	/* A pointer to the bridge that created the thread */
	Bridge* bridge;

	/* A list of GPRM packets to be scattered among neighbours of the current MPI node */
	std::vector<Packet_t> packet_list;
};

/**
 * Used for passing parameters to threads 
 */
struct bridge_packet_tag{
	/* A pointer to the bridge that created the thread */
	Bridge* bridge;

	/* A GPRM packet to be sent using a MPI message */
	Packet_t packet;

	/* The tag of the MPI message to be sent */
	int tag;
};

/**
 * Used for creating tags that label MPI messages
 * @param tag_type the type of tag, see enum MPI_Send_Type in Types.h
 * @param receiver the node_id of the receiving tile
 * @param sender the node_id of the sending tile
 * @return the tag
 */
int create_tag(tag_t tag_type, receiver_t receiver=0, sender_t sender=0){
	int tag_field = (tag_type << FS_tag_t) & F_tag_t;
	int receiver_field = (receiver << FS_receiver_t) & F_receiver_t;
	int sender_field = (sender << FS_sender_t) & F_sender_t;
	int final_tag = tag_field + receiver_field + sender_field;
	return final_tag;
}

/**
 * Given a tag return the type of the tag (see enum MPI_Send_Type in Type.h)
 * @param final_tag the tag
 * @return the tag type
 */
int get_tag_type(int final_tag){
	return (final_tag & F_tag_t) >> FS_tag_t;
}

/**
 * Given a tag return the node_id of the receiver 
 * @param final_tag the tag
 * @return the node_id of the receiver
 */
int get_receiver_id(int final_tag){
	return (final_tag & F_receiver_t) >> FS_receiver_t;
}

/**
 * Given a tag return the node_id of the sender
 * @param final_tag the tag
 * @return the node_id of the sender
 */
int get_sender_id(int final_tag){
	return (final_tag & F_sender_t) >> FS_sender_t;
}

/**
 * A function that will be executed by a thread which will initiate a stencil operation
 * @param args a void pointer that points to a dynamically allocated bridge_packets structure
 */
void* stencil_operation_th(void *args); // TODO: Remove?

/**
 * A function that will be executed by a thread which will initiate a send operation
 * @param args a void pointer that points to a dynamically allocated bridge_packet_tag structure
 */
void* send_th_fcn(void *args);

// Used for sending GPRM packets through MPI messages to other MPI processes 
void Bridge::send(Packet_t packet, int tag) {

	/* Reference to the System instance that created the bridge */
	System& sba_system = *((System*)this->sba_system_ptr);

#ifdef EVALUATE
	#ifdef VERBOSE
		#ifndef THREADED_SEND
	/* Used for measuring the time it took to invoke the Bridge::send() method */
	sba_system.send_fcn_start = MPI_Wtime();
		#endif // THREADED_SEND
	#endif// VERBOSE
#endif //EVALUATE

	/* Pointer to the communicator to be used for this operation */
	MPI_Comm *comm_ptr = sba_system.get_comm_ptr();

	/* the header of the GPRM packet to be sent */
	Header_t header = getHeader(packet);

	/* Remainder of division with (Number_of_MPI_Processes * NSERVICES) in case dest is larger */
	ServiceAddress dest = ((getTo(header)-1) % (sba_system.get_size()*NSERVICES))+1;

	/* The MPI rank of the process to receive the packet */
	int to_rank = (int) (dest-1)/ NSERVICES;

	/* A handle to a request object used for quering the status of the send operation */
	MPI_Request req;

	/* Send the MPI message */
	MPI_Isend(&packet.front(), packet.size(), MPI_UINT64_T, to_rank, tag, *comm_ptr, &req);

	/* Flag that indicates the status of the send operation */
	int flag;

	/* Wait until the whole message is sent */
	do { 
		MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
	} while (!flag);

	/* If the GPRM packet is of type P_DRESP send another message containing the float arrat that the packet has a pointer to */
	if (getPacket_type(header) == P_DRESP) {

#ifdef VERBOSE
		printf("Rank %d: Sending float array...\n", sba_system.get_rank());
#endif // VERBOSE

		/* Get size of float array */
		int data_size = getReturn_as(header); 

		/* Get the pointer to the array */
		void* ptr = (void *) packet.back();
		float *arr = (float*)ptr;

		/* Get node_id of the tile that initiated the send operation */
		int sender_id = (int) getReturn_to(header);

		/* Create the tag of the MPI message containing the float array */
		tag = create_tag(tag_dresp_data, dest, sender_id);

		/* Send the float array */
		MPI_Isend(arr, data_size, MPI_FLOAT, to_rank, tag, *comm_ptr, &req);

		/* Wait until the whole message is sent (this does not necessarily mean it was also received) */
		do { 
			MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
		} while (!flag);
		/* Delete the dynamically allocated float array, it is no longer needed on this process */
		delete [] arr;

#ifdef VERBOSE
		printf("Rank %d: Float array sent!\n", sba_system.get_rank());
#endif // VERBOSE

	}

#ifdef VERBOSE
	printf("Rank %d (Sent): Sent a packet to %d\n", get_rank(), to_rank);
#endif // VERBOSE
	
}


// Used for sending GPRM packets through MPI messages to other MPI processes 
void Bridge::send_th(Packet_t packet, int tag) {

	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* Create a bridge_packet_tag element which will be passed to a thread as an argument */
	struct bridge_packet_tag parameters_ = {
		this,
		packet,
		tag
	};

	/* Since pthreads take only args of type void* store the struct on the free store and pass a pointer to the thread */
	/* The free store is used so that the thread can have access to the struct */
	struct bridge_packet_tag *parameters = new struct bridge_packet_tag(parameters_);

	int rc = pthread_create(&thread, &attr, send_th_fcn, (void *) parameters);
	if (rc) {
		printf("Rank %d: send thread could not be created. Exiting program...\n", rank);
		exit(1);
	}
	
	/* Destroy the attr element. This does not affect the thread */
	pthread_attr_destroy(&attr);	
}

// Returns the MPI neighbours of the current MPI process in  System::process_tbl
std::vector<int> Bridge::get_neighbours(){
	return sba_system_ptr->get_neighbours();
}

//Returns the rank of this MPI node
int Bridge::get_rank(){
	return rank;
}

// Returns a pointer to the System instance
Base::System* Bridge::get_system_ptr(){
	return sba_system_ptr;
}

// The function to be executed in a different thread, used for listening for incoming MPI messages
// The thread is created in the constructor of Bridge
void* wait_recv_any_th(void *arg){
	Bridge *bridge = (Bridge*) arg;
	MPI_Status status;

	/* Reference to the System instance */
	System& sba_system = *((System*)bridge->get_system_ptr());

	/* Pointer to the communicator */
	MPI_Comm *comm_ptr = sba_system.get_comm_ptr();

	/* Used for checking the output of MPI_Iprobe() and MPI_Test() */
	int flag;

	while(sba_system.is_active()){

		/* Check whether the destructor of System has been invoked */
		if (!sba_system.is_active()){
			/* If the destructor of System has been invoked  exit the loop, which will lead to the thread being terminated */
			break;
		}

#if MPI_VERSION<3

		/* Only one receiving thread at a time should execute the code below, so that the thread that gets a true flag also receives the
		 * incoming message (there is a chance another thread will receive the message before this one can) */

		/* Lock */
		pthread_spin_lock(&(bridge->recv_lock));

		/* Test for a message, but don't receive */ 
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, *comm_ptr, &flag, &status);
		if (!flag){
			
			/* No incoming message, unlock and continue*/
			pthread_spin_unlock(&(bridge->recv_lock));
			continue;
		}

		Packet_t packet;

		/* Find the packet size */
		int recv_size;
		MPI_Get_count(&status, MPI_UINT64_T, &recv_size);
		packet.resize(recv_size);

		/* Handle to request object. Used for querying the status of the reciving operation */
		MPI_Request req;

		/* Receive the probed message */
		MPI_Irecv(&packet.front(), recv_size, MPI_UINT64_T, status.MPI_SOURCE, status.MPI_TAG,
					*comm_ptr, &req);
	
		/* Receiving has started, unlock */
		pthread_spin_unlock(&(bridge->recv_lock));

#else // if MPI_VERSION >= 3

		/* Used for matching the detected incoming message when MPI_Imrecv() is invoked */
		MPI_Message msg;

		/* Test for a tag_default message, but don't receive */ 
		MPI_Improbe(MPI_ANY_SOURCE, tag_default, *comm_ptr, &flag, &msg, &status);

		if (!flag){
			/* No incoming message, continue (i.e. listen again for incoming messages) */
			continue;
		}

		/* Used for retrieving the packet that the incoming MPI message contains */
		Packet_t packet;

		/* Finds the packet size */
		int recv_size;
		/* MPI routine that returns the number of elements of type unsigned int64 in the incoming message */
		MPI_Get_count(&status, MPI_UINT64_T, &recv_size);

		/* Resize the packet variable so that it can hold exactly the incoming packet */
		packet.resize(recv_size);

		/* Handle to request object. Used for querying the status of the receiving operation */
		MPI_Request req;

		/* Receive the probed message */
		MPI_Imrecv(&packet.front(), recv_size, MPI_UINT64_T, &msg, &req);


#endif // MPI_VERSION<3


		/* Wait until the whole message is received */
		do {
			MPI_Test(&req, &flag, &status);
		} while (!flag);


#ifdef VERBOSE
		int from_rank = (int) (getReturn_to(getHeader(packet))-1) / NSERVICES;
		printf("Rank %d: Received msg from rank %d\n", sba_system.get_rank(), from_rank);
#endif // VERBOSE


		Header_t header = getHeader(packet);

#ifdef EVALUATE
		/* Perform operations that should ONLY take place when the EVALUATE macro is defined. This macro should never be used during
		 * actualy deployment. It is only for testing purposes */

		/* Check if the packet received is of type DACK. This is used as an ack packet for evaluation purposes */
		if (getPacket_type(header) == P_DACK) { 

			/* Get time from arbitrary point in the past */
			double end = MPI_Wtime();
	
			/* Get the starting time */
			double start = sba_system.start_time;

	/* These times have meaning only for the test_time_dresp() test and should be ignored otherwise */
	#ifdef VERBOSE 
		#ifdef THREADED_SEND
			printf("until creating the sending thread: %f \n", sba_system.send_thread_start - start);
		#else
			printf("until invoking the Bridge::send() method: %f \n", sba_system.send_fcn_start - start);
		#endif // THREADED_SEND

			/* Print the total time required for this message to be sent, received and an ack to be sent to the original sender */
			printf("RANK %d: TIME: %fsecs\n", sba_system.get_rank(), end-start); 
	#endif // VERBOSE

			sba_system.end_time += end - start;

			/* The test is completed */
			sba_system.testing_status=false; 
			continue;
		}
#endif //EVALUATE

		/* Get node_id of receiver (stored on the packet header) */
		int receiver_node_id = (int) getTo(getHeader(packet));

		/* Remainder of division with (Number_of_MPI_Processes * NSERVICES) in case dest is larger */
		receiver_node_id = ((getTo(header)-1) % (sba_system.get_size()*NSERVICES))+1;

		/* Get node_id of sender (stored on the packet header) */
		int sender_node_id = (int) getReturn_to(getHeader(packet));

		/* if the GMCF packet received is of type DRESP there must be another MPI message coming that holds a float array.
		 * Wait for that packet 
		 */
		if (getPacket_type(header) == P_DRESP){

			/* Create the tag that the expected MPI message will have. It is a combination of the sender ID, receiver ID and the
			 * type of the tag (tag_dresp_data) */
			int tag = create_tag(tag_dresp_data, receiver_node_id, sender_node_id);

#ifdef VERBOSE
			printf("Rank %d: Waiting for a float array...\n", sba_system.get_rank());
#endif // VERBOSE
			
#if MPI_VERSION<3

			/* No locking is required, we do not need to synchronise MPI_Iprobe() and MPI_Irecv() because only the expected 
			 * message should have the aforementioned tag
			 */

			/* Test for a message, but don't receive */ 			
			do{
				MPI_Iprobe(MPI_ANY_SOURCE, tag, *comm_ptr, &flag, &status);
			}while (!flag);

			/* Packet detected. Find the size of the float array, receive the message and store the array */
			MPI_Get_count(&status, MPI_FLOAT, &recv_size);
			float *arr = new float[recv_size];

			/* Receive the probed message */
			MPI_Irecv(arr, recv_size, MPI_FLOAT, status.MPI_SOURCE, tag,
						*comm_ptr, &req);
			do {
				MPI_Test(&req, &flag, &status);
			} while (!flag);

#else // if MPI_VERSION>=3

			/* Wait for the message */
			flag = false;
			while (!flag){
				MPI_Improbe(MPI_ANY_SOURCE, tag, *comm_ptr, &flag, &msg, &status);
			}

			/* Packet detected. Find the size of the float array, receive the message and store the array */
			MPI_Get_count(&status, MPI_FLOAT, &recv_size);
			float *arr = new float[recv_size];
			MPI_Imrecv(arr, recv_size, MPI_FLOAT, &msg, &req);
			do {
				MPI_Test(&req, &flag, &status);
			} while (!flag);

#endif // MPI_VERSION<3

			/* At this point the float array has been received */
#ifdef VERBOSE
			printf("Rank %d: Float array received!\n", sba_system.get_rank());
#endif // VERBOSE

			/* store the arr pointer in the back of the packet, remove the Word the contained the previous pointer (on the sending node) */
			packet.at(packet.size()-1) = (Word) arr;
		}

#ifdef EVALUATE	
		/* If the DRESP packet detected was used for evaluation purposes, don't store it. Instead, send a DACK packet back and keep 
		 * listening for messages. When EVALUATE is defined the packets will NOT go to the RX FIFO of the receiving tile */

		Payload_t payload;
		payload.push_back((Word)999);

		/* Create the header of the GMCF packet. This function is part of the original GMCF code */
		header = mkHeader(P_DACK, 2, 3, payload.size(), sender_node_id, receiver_node_id, 7 , 0);

		/* Create the GMCF packet. This function is part of the original GMCF code */
		packet = mkPacket(header, payload);

		/* Send the packet */
	#ifdef THREADED_SEND
		sba_system.send_th(packet, tag_default);
	#else
		sba_system.send(packet, tag_default);
	#endif // THREADED_SEND
		continue;

#endif // EVALUATE

		/* Calculate the service_id of the Tile instance to receive the GMCF packet */
		Service service_id = ((getTo(getHeader(packet)) - 1) % (sba_system.get_size()*NSERVICES))+1;
		ServiceAddress dest = service_id;
		sba_system.nodes[dest]->transceiver->rx_fifo.push_back(packet);
#ifdef VERBOSE
		printf("Rank %d (Recv): Sent packet to dest %d\n", sba_system.get_rank(), dest);
#endif // VERBOSE
	}

	/* System destructor has been called. Notify the System that this thread will be killed */
	sba_system.kill_thread();
#ifdef VERBOSE
	printf(" %d: Recv thread is exiting...\n", sba_system.get_rank());
#endif // VERBOSE
	pthread_exit(NULL);
}

// A function that will be executed by a thread which will initiate a send operation
void* send_th_fcn(void *args) {
	
	/* Convert the void pointer *args to a bridge_packet_tag structure */
	struct bridge_packet_tag *parameters;
	parameters = (struct bridge_packet_tag *) args;

	/* Pointer to the bridge that created this thread */
	Bridge* bridge = parameters->bridge;

	/* The GPRM packet to be sent */
	Packet_t packet = parameters->packet;

	/* The tag of the MPI message to be sent */
	int tag = parameters->tag;

	/* Pointer to the System instance that created the Bridge */
	System& sba_system = *((System*)bridge->get_system_ptr());

#ifdef EVALUATE
	#ifdef VERBOSE
	/* Used for measuring the time it took to start the sending thread */
	sba_system.send_thread_start = MPI_Wtime();
	#endif // VERBOSE
#endif // EVALUATE

	/* Pointer to the communicator to be used for this operation */
	MPI_Comm *comm_ptr = sba_system.get_comm_ptr();

	/* the header of the GPRM packet to be sent */
	Header_t header = getHeader(packet);

	/* Remainder of division with (Number_of_MPI_Processes * NSERVICES) in case dest is larger */
	ServiceAddress dest = ((getTo(header)-1) % (sba_system.get_size()*NSERVICES))+1;

	/* The MPI rank of the process to receive the packet */
	int to_rank = (int) (dest-1)/ NSERVICES;

	/* A handle to a request object used for quering the status of the send operation */
	MPI_Request req;

	/* Send the MPI message */
	MPI_Isend(&packet.front(), packet.size(), MPI_UINT64_T, to_rank, tag, *comm_ptr, &req);

	/* Flag that indicates the status of the send operation */
	int flag;

	/* Wait until the whole message is sent */
	do { 
		MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
	} while (!flag);

	/* If the GPRM packet is of type P_DRESP send another message containing the float arrat that the packet has a pointer to */
	if (getPacket_type(header) == P_DRESP) {

#ifdef VERBOSE
		printf("Rank %d: Sending float array...\n", sba_system.get_rank());
#endif // VERBOSE

		/* Get size of float array */
		int data_size = getReturn_as(header); 

		/* Get the pointer to the array */
		void* ptr = (void *) packet.back();
		float *arr = (float*)ptr;

		/* Get node_id of the tile that initiated the send operation */
		int sender_id = (int) getReturn_to(header);

		/* Create the tag of the MPI message containing the float array */
		tag = create_tag(tag_dresp_data, dest, sender_id);

		/* Send the float array */
		MPI_Isend(arr, data_size, MPI_FLOAT, to_rank, tag, *comm_ptr, &req);

		/* Wait until the whole message is sent (this does not necessarily mean it was also received) */
		do { 
			MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
		} while (!flag);
		/* Delete the dynamically allocated float array, it is no longer needed on this process */
		delete [] arr;

#ifdef VERBOSE
		printf("Rank %d: Float array sent!\n", sba_system.get_rank());
#endif // VERBOSE
	}

#ifdef VERBOSE
	printf("Rank %d (Threaded Sent): Sent a packet to %d\n", sba_system.get_rank(), to_rank);
#endif // VERBOSE


	/* Remove the dynamically allocated bridge_packet_tag structure that was passed as an argument to this function */
	delete parameters;

	pthread_exit(NULL);
}
