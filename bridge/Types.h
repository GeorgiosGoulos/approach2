#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <deque> 
#include <pthread.h>
#include <iostream>

using namespace std;


//TODO: Remove?
#include <cstdint>
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint64_t u_int64_t;

typedef u_int64_t Uint64;
typedef Uint64 Word;
typedef std::vector<Word> Packet_t;
typedef std::vector<Word> Header_t;
typedef std::vector<Word> Payload_t;
typedef unsigned int Service; // 4 bytes
typedef unsigned int MemAddress; // typically fits into Subtask
typedef MemAddress ServiceAddress;

//typedef std::vector<Packet_t> Packet_Fifo; //Actually it's typedef Fifo<Packet_t> Packet_Fifo;


//typedef Packet_Fifo RX_Packet_Fifo; // Actually RX_Packet_Fifo is a class containing a deque (Types.h)


enum MPI_Send_Type {
	tag_other = 0,
	tag_default = 1,
	tag_stencil_scatter = 2,
	tag_stencil_reduce = 3,
	tag_neighboursreduce_scatter = 4,
	tag_neighboursreduce_reduce = 5,
	tag_neighboursreduce_bcast = 6,
	tag_test = 7 // TODO: Remove?
	
	,tag_dresp_data = 10
#ifdef EVALUATE
	, tag_time_send = 8,
	tag_time_ack = 9
#endif // EVALUATE
};

#define TMP_RANK 0 // debugging, used in System.h/System.cc
#define NSERVICES 9 // tmp, used in System.h/System.cc
#define NBRIDGES 1 // tmp, used in System.h/System.cc





//template <typename Packet_t, Word depth>
class RX_Packet_Fifo {
	private:
    	deque<Packet_t> packets;
    	pthread_spinlock_t _RXlock;
 	bool _status;
	public:
 		RX_Packet_Fifo () :  _status(0) {
 			pthread_spin_init(&_RXlock, PTHREAD_PROCESS_SHARED);
 		};
 		~RX_Packet_Fifo() {
 			pthread_spin_destroy(&_RXlock);
 		}
 	bool status() {
 		//FIXME: make this blocking? Ashkan added lock & unlock
 		pthread_spin_lock(&_RXlock);
 		bool stat = _status;
 		pthread_spin_unlock(&_RXlock);
     	return stat;
 	}

    bool has_packets() {
// we want to block until the status is true
// so status() will block until it can return true
 	  pthread_spin_lock(&_RXlock);
 	 bool has = (_status==1);
 	  pthread_spin_unlock(&_RXlock);
 	  return(has);
    }

    void wait_for_packets() {
    // we want to block until the status is true
      // so status() will block until it can return true
        while(packets.empty())
        {
            __asm__ __volatile__ ("" ::: "memory");
        }
        _status=1;
#ifdef VERBOSE
    cout << "RX_Packet_Fifo: UNBLOCK on wait_for_packets()\n";
#endif
	}
    void push_back(Packet_t const& data) {
        pthread_spin_lock(&_RXlock);
        packets.push_back(data);
        _status=1;
        pthread_spin_unlock(&_RXlock);
      }

    Packet_t pop_front() {
#ifdef VERBOSE
        cout << "RX_Packet_Fifo: pop_front()\n";
#endif
        while(packets.empty()) { // this is for double check, it should not happen
        	cout << "RX_Packet_Fifo: Thread Blocked For Some Strange Reason!" << endl;
        	__asm__ __volatile__ ("" ::: "memory");
        }
        pthread_spin_lock(&_RXlock);
        Packet_t t_elt=packets.front();
        packets.pop_front();
        if (packets.size() == 0) _status=0;
        pthread_spin_unlock(&_RXlock);

#ifdef VERBOSE
        cout << "RX_Packet_Fifo: DONE pop_front() \n";
#endif
        return t_elt;
    }
}; // RX_Packet_Fifo




// depth is ignored for dynamic alloc
//template <typename LType, Word depth> class Fifo  {
template <typename LType> class Fifo  {
    private:
        deque<LType> fifo;
        unsigned int _status;
	public:
        unsigned int status() {
            //std::cout << "Fifo status: "<<_status<<"\n";
            return _status;
        }
        bool has_packets() {
            return (_status==1);
        }
        LType front() {
        return fifo.front();
		}
		LType pop_front() {
			LType elt=fifo.front();
			fifo.pop_front();
			if (fifo.size()==0) _status=0;
			return elt;
		}

		LType shift() {
			LType t_elt=fifo.front();
			fifo.pop_front();
			if (fifo.size()==0) _status=0;
			return t_elt;
		}
		void unshift(LType& elt) {
			fifo.push_front(elt);
			_status=1;
		}
		void push(LType& elt) {
			fifo.push_back(elt);
			_status=1;
		}

		void push_back(LType& elt) {
			fifo.push_back(elt); // this throws an error in malloc
			_status=1;
		}

		LType pop() {
			LType t_elt=fifo.back();
			fifo.pop_back();
			if (fifo.size()==0) _status=0;
			return t_elt;
		}
		unsigned int size() {
			return fifo.size();
		}

		unsigned int length() {
			return fifo.size();
		}
		void clear() {
			fifo.clear();
			_status=0;
		}
		Fifo() : _status(0) {};
}; // of Fifo template



typedef Fifo<Packet_t> Packet_Fifo;

typedef Packet_Fifo TX_Packet_Fifo;

#endif // TYPES_H
