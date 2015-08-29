#ifndef SYSTEM_BASE_H_
#define SYSTEM_BASE_H_

#include <vector>

namespace Base {
	class System {
		public:
			virtual std::vector<int> get_neighbours()=0;		
	};
}
#endif /*TILE_BASE_H_*/
