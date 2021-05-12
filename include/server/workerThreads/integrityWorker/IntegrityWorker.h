//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_INTEGRITYWORKER_H
#define TIN_TORRENTLIKEP2P_INTEGRITYWORKER_H


#include "../../../Thread.h"

class IntegrityWorker {

public:
	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((IntegrityWorker*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}
};


#endif //TIN_TORRENTLIKEP2P_INTEGRITYWORKER_H
