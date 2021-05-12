//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_SUPERVISORWORKER_H
#define TIN_TORRENTLIKEP2P_SUPERVISORWORKER_H


#include "../../../Thread.h"

class SupervisorWorker {
	// idea of this class is to babysit download and upload, check if anything goes wrong
	// pure download part goes to download worker
	// may be unnecessary

public:
	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((SupervisorWorker*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}
};


#endif //TIN_TORRENTLIKEP2P_SUPERVISORWORKER_H
