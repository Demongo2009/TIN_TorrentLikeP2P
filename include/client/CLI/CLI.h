//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_CLI_H
#define TIN_TORRENTLIKEP2P_CLI_H


#include "../../Thread.h"

class CLI {


public:
	CLI(){}

	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((CLI*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}
};


#endif //TIN_TORRENTLIKEP2P_CLI_H
