ROWS=2 # Number of processes per row
COLS=2 # Number of processes per column
TOTAL=$(($ROWS*$COLS)) # Total number of processes

# BRIDGE: Essential for executing this program successfully, required if bridges are to be used in GMCF
# NBRIDGES: Used for defining the number of bridges per MPI node. If not defined it will be assigned the value 3 (in Types.h)
# THREADED_SEND: Used when the sending threads should be used instead of the Bridhe::send() method
# EVALUATE: Used for performing evaluation testing. Should not be used during actual deployment
# MPI_TOPOLOGY_OPT: Used when the potentially optimised cartesian topology should be utilised
# VERBOSE: Print messages that indicate the operations taking place

if mpic++ -pthread -std=c++11  -DBRIDGE -DNBRIDGES=1 -DEVALUATE -DMPI_TOPOLOGY_OPT -Wall -Werror main.cc System.cc Tile.cc Transceiver.cc Packet.cc Bridge.cc; then		# COMPILATION
	mpiexec -np $TOTAL ./a.out $ROWS $COLS		# EXECUTION
	#mpiexec -np $TOTAL valgrind --suppressions=$PREFIX/openmpi-valgrind.supp --gen-suppressions=yes ./a.out $ROWS $COLS	

fi

# if mpic++ -pthread -std=c++11 -Wall -Werror main.cc System.cc Tile.cc Bridge.cc; then		# COMPILATION
#	mpiexec -np $TOTAL ./a.out $ROWS $COLS		# EXECUTION
# fi

