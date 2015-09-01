#include "System.h"

#ifdef BRIDGE
#include <vector>
#include "mpi.h"
#include <algorithm> //find(), sort()
#endif //BRIDGE

	#ifdef VERBOSE
#include <sstream>
	#endif // VERBOSE


#ifdef BRIDGE
// Create a 2D table of the MPI processes. The table is stored in process_tbl
void System::initialise_process_table(){
	process_tbl.resize(rows); // Each element of this vector represents a row
	for (int i = 0; i < rows; i++) {
		std::vector<int> row(cols); // Each row has cols number of elements
		for (int j = 0; j < cols; j++){

			// For each element in the row: element = TotalRows*CurrentColumn + CurrentRow
			row.at(j) = rows*j + i;
		}

		/* Add the row to the table */
		process_tbl.at(i) = row; 
	}
}

	#ifdef MPI_TOPOLOGY_OPT

// Find the processes that surround the current MPI node in the aforementioned table. They are stored in neighbours.
// This method can be used for a 2D topology ONLY, modifications are required for other dimensions
// WARNING: This method considers the topology a torus. All ranks have the same number of neighbours (8)
void System::find_neighbours(){ 

	/* Number of dimensions */
	int ndims = 2; 

	/* Array indicating the coordinates of the rank in the process group */
	int coords[ndims]; // coord[0]:col, coord[1]: row

	/* Given a communicator, a rank and the number of dimensions of the topology, this routine returns the coordinates of the 
	 * rank in the topology 
	 */
	MPI_Cart_coords(comm,rank, ndims, coords);
	
	/* Indicates the cooordinates of the neighbours */
	int new_coords[2];

	/* The rank of the neighbour */
	int neighbour;


	for (int col = -1; col <= 1; col++){
		for (int row = -1; row <= 1; row++) {
		
			// Don't add the current MPI process in the list of neighbours
			if (col == 0 && rank == 0) {
				continue;
			}
			new_coords[0] = coords[0] + col;
			new_coords[1] = coords[1] + row;

			/* Given a communicator and the cooordinates of the process in the topology, return the rank of that process */
			MPI_Cart_rank(comm,new_coords,&neighbour);

			/* Ensure that each neighbour is added only once. This is useful when we have a small number of processes in a rectangular
			 * topology (e.g. 2x2 or 3x3)
			 */
			if (!(std::find(neighbours.begin(), neighbours.end(), neighbour) != neighbours.end())) {
				this->neighbours.push_back(neighbour);
			}
		}	
	}

}

	#else // ifndef MPI_TOPOLOGY_OPT 

// Find the processes that surround the current MPI node in the aforementioned table. They are stored in neighbours.
// This method can be used for a 2D topology ONLY, modifications are required for other dimensions
// WARNING: This method does not consider the topology a torus. Processes on the edges of the process table will have fewer neighbours
void System::find_neighbours(){

	/* The column on which the rank is located */
	int col = rank/rows; 
	
	/* The column on which the rank is located */
	int row = rank - col*rows;

	/* Neighbour at top left */
	if ((row-1 >= 0) && (col-1 >= 0)) {								
		this->neighbours.push_back(this->process_tbl.at(row-1).at(col-1));
	}

	/* Neighbour at left */
	if (col-1 >= 0) {	
		this->neighbours.push_back(this->process_tbl.at(row).at(col-1)); 
	}

	/* Neighbour at bottom left */
	if ((row+1 < rows) && (col-1 >= 0)) {	
		this->neighbours.push_back(this->process_tbl.at(row+1).at(col-1));
	}

	/* Neighbour at top */
	if (row-1 >= 0) {	
		this->neighbours.push_back(this->process_tbl.at(row-1).at(col));
	}

	/* Neighbour at bottom */
	if (row+1 < rows) { 
		this->neighbours.push_back(this->process_tbl.at(row+1).at(col));
	}

	/* Neighbour at top right */
	if ((row-1 >= 0) && (col+1 < cols )) {	
		this->neighbours.push_back(this->process_tbl.at(row-1).at(col+1));
	}				

	/* Neighbour at right */
	if (col+1 < cols) { 
		this->neighbours.push_back(this->process_tbl.at(row).at(col+1));
	}

	/* Neighbour at bottom right */
	if ((row+1 < rows) && (col+1 < cols)) {	
		this->neighbours.push_back(this->process_tbl.at(row+1).at(col+1));
	}
}
	#endif //MPI_TOPOLOGY_OPT

	#ifdef VERBOSE

// Prints the neighbouring ranks of the current process on the process table
void System::print_neighbours(){
	stringstream sstream;
	sstream.str("");
	sstream << "Neighbours of " << rank << ": ";
	for (std::vector<int>::iterator it = this->neighbours.begin(); it < this->neighbours.end(); it++){
		sstream << *it << " ";
	}
	sstream << "\n";
	std::cout << sstream.str();
}

// Prints a table representing the logical topology
void System::print_process_table(){
	stringstream sstream;
	sstream.str("");
	sstream << "=== PROCESS TABLE ===\n";
	for (auto row1: process_tbl){
		sstream << "\t";
		for (std::vector<int>::iterator it = row1.begin(); it < row1.end(); it++){
			sstream << *it << " ";
		}
		sstream << "\n";
	}
	sstream << "=====================\n";

	std::cout << sstream.str();
}
	#endif // VERBOSE

// Returns the neighbouring processes of the current process on the process table
std::vector<int> System::get_neighbours(){
	return this->neighbours;
}

// Returns the rank of the current MPI process
int System::get_rank(){
	return rank;
}

// Returns the number of MPI processes created (threaded)
int System::get_size() {
	return size;
}

// Selects a bridge and sends a packet to another process
void System::send_th(Packet_t packet, int tag){
	increment_bridge_pos();
	#ifdef VERBOSE	
	stringstream sstream;
	sstream.str("");
	sstream << "Rank " << rank << ": Bridge " << bridge_pos << "(0-" << bridge_list.size()-1 << ") was selected to send a message \n";
	std::cout << sstream.str();
	#endif // VERBOSE
	this->bridge_list.at(bridge_pos)->send_th(packet, tag);
}

// Selects a bridge and sends a packet to another process (not threaded)
void System::send(Packet_t packet, int tag){
	increment_bridge_pos();
	#ifdef VERBOSE	
	stringstream sstream;
	sstream.str("");
	sstream << "Rank " << rank << ": Bridge " << bridge_pos << "(0-" << bridge_list.size()-1 << ") was selected to send a message \n";
	std::cout << sstream.str();
	#endif // VERBOSE
	this->bridge_list.at(bridge_pos)->send(packet, tag);
}

// Increments the member variable that indicates the index of the next bridge to be used 
void System::increment_bridge_pos(){ 
	/* Lock */
	pthread_spin_lock(&bridge_selector_lock);

	bridge_pos = (bridge_pos+1) % bridge_list.size();

	/* Unlock */
	pthread_spin_unlock(&bridge_selector_lock);
}

	#ifdef MPI_TOPOLOGY_OPT

// Creates a new (possibly optimised) communicator that handles the logical topology as a torus architecture
MPI_Comm System::create_communicator(int rows, int cols){

	/* The new communicator */
	MPI_Comm comm;

	/* Number of dimensions in the logical topology */
	int ndims = 2;

	/* An array indicating the size of each dimension (each element represents the size of a dimension) */
	int dim[2];

	/* An array each element of which indicates whether the topology cycles in a dimension (used to create tori) */
	int period[2]; 

	/* States whether reordering of ranks is accepted, required for potential optimised logical topology */
	int reorder; 
	
	dim[0] = rows;
	dim[1] = cols;
	period[0] = true;
	period[1] = true;
	reorder = true;

	/* Create a communicator for the new logical topology */
	MPI_Cart_create(MPI_COMM_WORLD, ndims, dim, period, reorder, &comm);
	return comm;
}

	#endif // MPI_TOPOLOGY_OPT

// Returns the status of the System, which is true until the destructor of the System instance is called
bool System::is_active(){
	return active;
}

// Thread-safe way of incrementing killed_threads, called when a receiving thread is about to exit 
void System::kill_thread(){
	/* Lock */ 
	pthread_spin_lock(&killed_threads_lock);

	killed_threads++;

	/* Unlock */
	pthread_spin_unlock(&killed_threads_lock);
}

// Returns a pointer to the communicator
MPI_Comm* System::get_comm_ptr() {
	return &comm;
}

#endif //BRIDGE
