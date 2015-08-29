ROWS=2 # Number of processes per row
COLS=2 # Number of processes per column
TOTAL=$(($ROWS*$COLS)) # Total number of processes

# g++ test_bridge.cc bridge.cc

if mpic++ -pthread -std=c++11  -DEVALUATE -DMPI_TOPOLOGY_OPT -DBRIDGE -Wall -Werror main.cc System.cc Tile.cc Transceiver.cc Packet.cc Bridge.cc; then		# COMPILATION
	mpiexec -np $TOTAL ./a.out $ROWS $COLS		# EXECUTION
fi

# if mpic++ -pthread -std=c++11 -Wall -Werror main.cc System.cc Tile.cc Bridge.cc; then		# COMPILATION
#	mpiexec -np $TOTAL ./a.out $ROWS $COLS		# EXECUTION
# fi

