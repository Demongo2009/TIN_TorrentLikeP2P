//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_UPLOADWORKER_H
#define TIN_TORRENTLIKEP2P_UPLOADWORKER_H


#include "../../../Thread.h"

class UploadWorker {


public:
	UploadWorker(){}

	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((UploadWorker*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}
};


#endif //TIN_TORRENTLIKEP2P_UPLOADWORKER_H
